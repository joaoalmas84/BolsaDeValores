﻿#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <fcntl.h> 
#include <io.h>

#include "Utils.h"
#include "Structs.h"

#include "Board.h"

int _tmain(int argc, TCHAR* argv[]) {
    int setmodeReturn; // Evitar o warning da funcao setmode();

    DWORD codigoErro;

    // Numero de empresas a listar (tamanho do top)
    DWORD N; 
    
    EMPRESA empresas[MAX_EMPRESAS_TO_SHM];
    DWORD numEmpresas;
    BOOL continua_Board = TRUE;
    BOOL continua_Bolsa = TRUE;

    // Variáveis da Thread
    DWORD threadId;
    HANDLE hThread;
    TDATA_BOARD threadData;
    CRITICAL_SECTION cs;

    // Variaveis da SharedMemory
    SHM* sharedMemory;
    HANDLE hMap;
    HANDLE hMutex;
    HANDLE hEvent_SHM; // Evento SHM (Reset Manual);

    // Array de Eventos para o WaitForMultipleObjects();
    HANDLE hEvents[2]; // [0]: Evento_SHM; [1]: Evento_Exit

    TCHAR ultimaTransacao[SMALL_TEXT] = {_T("\0")};
    
#ifdef UNICODE
    setmodeReturn = _setmode(_fileno(stdin), _O_WTEXT);
    setmodeReturn = _setmode(_fileno(stdout), _O_WTEXT);
    setmodeReturn = _setmode(_fileno(stderr), _O_WTEXT);
#endif 

    if (argc != 2 || !IsInteger(argv[1]) || _wtoi(argv[1]) <= 0 || _wtoi(argv[1]) > 10) {
        _tprintf_s(_T("O programa Board recebe 1 argumento, o tamanho do TOP a ser apresentado.\nEste tamanho é no máximo 10"));
        return 1;
    }

    N = _wtoi(argv[1]);

    if (N > 10) { N = 10; }

    hMutex = OpenMutex(SYNCHRONIZE, FALSE, SHM_MUTEX);
    if (hMutex == NULL) {
        codigoErro = GetLastError();
        if (codigoErro == ERROR_FILE_NOT_FOUND) {
            _tprintf_s(_T("\nO programa Bolsa ainda não se encontra em execução."));
        } else {
            PrintErrorMsg(codigoErro, _T("Erro em OpenMutex."));
        }
        return 1;
    }

    hMap = OpenFileMapping(FILE_MAP_READ, FALSE, SHARED_MEMORY);
    if (hMap == NULL) {
        PrintErrorMsg(GetLastError(), _T("Erro em OpenFileMapping"));
        CloseHandle(hMutex);
        return 1;
    }

    sharedMemory = (SHM*)MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
    if (sharedMemory == NULL) {
        PrintErrorMsg(GetLastError(), _T("Erro em MapViewOfFile"));
        CloseHandle(hMutex);
        CloseHandle(hMap);
        return 1;
    }

    hEvent_SHM = OpenEvent(SYNCHRONIZE, FALSE, SHM_EVENT);
    if (hEvent_SHM == NULL) {
        PrintErrorMsg(GetLastError(), _T("Erro em OpenEvent"));
        CloseHandle(hMutex);
        CloseHandle(hMap);
        return 1;
    }

    if (!InitializeCriticalSectionAndSpinCount(&cs, 0)) {
        PrintErrorMsg(GetLastError(), _T("Erro em InitializeCriticalSectionAndSpinCount"));
        CloseHandle(hMutex);
        CloseHandle(hEvent_SHM);
        CloseHandle(hMap);
        return 1;
    }

    threadData.pCs = &cs;
    threadData.continua = &continua_Board;

    threadData.hEvent_Exit = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (threadData.hEvent_Exit == NULL) {
        PrintErrorMsg(GetLastError(), _T("Erro em CreateEvent"));
        CloseHandle(hMutex);
        CloseHandle(hEvent_SHM);
        CloseHandle(hMap);
        return 1;
    }

    hThread = CreateThread(NULL, 0, ThreadGetChar, (LPVOID)&threadData, 0, &threadId);
    if (hThread == NULL) {
        PrintErrorMsg(GetLastError(), _T("Erro ao lançar ThreadRead"));
        CloseHandle(hMutex);
        CloseHandle(hEvent_SHM);
        CloseHandle(hMap);
        return 1;
    }

    WaitForSingleObject(hMutex, INFINITE);
    numEmpresas = sharedMemory->numEmpresas;
    _tcscpy_s(ultimaTransacao, SMALL_TEXT, sharedMemory->ultimaTransacao);
    CopyMemory(empresas, sharedMemory->empresas, sizeof(EMPRESA) * MAX_EMPRESAS_TO_SHM);
    ReleaseMutex(hMutex);

    if (numEmpresas <= 0) {
        _tprintf_s(_T("\nAinda não existem empresas registadas no sistema"));
        return 1;
    }

    hEvents[0] = hEvent_SHM;
    hEvents[1] = threadData.hEvent_Exit;

    while (continua_Board && continua_Bolsa) {
        system("cls");

        PrintTop(empresas, N, numEmpresas);
        _tprintf_s(_T("\n\tÚltima Transação : '%s'"), ultimaTransacao);

        _tprintf_s(_T("\n\n\nPrima ENTER para terminar..."));

        WaitForMultipleObjects(2, hEvents, FALSE, INFINITE);

        WaitForSingleObject(hMutex, INFINITE);
        EnterCriticalSection(&cs);
        _tcscpy_s(ultimaTransacao, SMALL_TEXT, sharedMemory->ultimaTransacao);
        CopyMemory(empresas, sharedMemory->empresas, sizeof(EMPRESA) * MAX_EMPRESAS_TO_SHM);
        continua_Board = *threadData.continua;
        continua_Bolsa = sharedMemory->continua;
        LeaveCriticalSection(&cs);
        ReleaseMutex(hMutex);

        Sleep(30000);

    }

    if (continua_Board) {
        EnterCriticalSection(&cs);
        continua_Board = FALSE;
        LeaveCriticalSection(&cs);

        if (!CancelSynchronousIo(hThread)) {
            PrintErrorMsg(GetLastError(), _T("CancelSynchronousIo"));
            exit(-1);
        }
    }

    WaitForSingleObject(hThread, INFINITE);

    CloseHandle(hThread);

    DeleteCriticalSection(&cs);

    FlushViewOfFile(sharedMemory, 0);

    CloseHandle(hMap);

    CloseHandle(hMutex);

    return 0;
}