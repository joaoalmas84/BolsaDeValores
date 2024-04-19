#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <fcntl.h> 
#include <io.h>

#include "Utils.h"
#include "Board.h"

int _tmain(int argc, TCHAR* argv[]) {
    int setmodeReturn;

    DWORD codigoErro;

    // Numero de empresas a listar
    DWORD N;

    EMPRESA empresas[10];
    BOOL continua = TRUE;
    HANDLE hEvents[2]; // [0] Evento SHM(Reset Manual); [1] Evento Exit(Reset Atomatico)

    // Variáveis da Thread
    DWORD threadId;
    HANDLE hThread;
    TDATA_BOARD threadData;

    // Variaveis da SharedMemory
    SHM* sharedMemory;
    HANDLE hMap;
    HANDLE hMutex_SHM;
    
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

    hMutex_SHM = OpenMutex(SYNCHRONIZE, FALSE, SHM_MUTEX);
    if (hMutex_SHM == NULL) {
        codigoErro = GetLastError();
        if (codigoErro == ERROR_FILE_NOT_FOUND) {
            _tprintf_s(_T("\nO programa Bolsa ainda não se encontra em execução."));
        } else {
            PrintErrorMsg(GetLastError(), _T("Erro em OpenMutex."));
        }
        return 1;
    }

    hEvents[0] = OpenEvent(SYNCHRONIZE, FALSE, SHM_EVENT);
    if (hEvents[0] == NULL) {
        PrintErrorMsg(GetLastError(), _T("Erro em OpenEvent"));
        CloseHandle(hMutex_SHM);
        return 1;
    }

    hEvents[1] = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (hEvents[1] == NULL) {
        PrintErrorMsg(GetLastError(), _T("Erro em CreateEvent"));
        CloseHandle(hMutex_SHM);
        CloseHandle(hEvents[0]);
        return 1;
    }

    hMap = OpenFileMapping(FILE_MAP_READ, FALSE, SHARED_MEMORY);
    if (hMap == NULL) {
        PrintErrorMsg(GetLastError(), _T("Erro em OpenFileMapping"));
        CloseHandle(hMutex_SHM);
        CloseHandle(hEvents[0]);
        return 1;
    }

    sharedMemory = (SHM*)MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
    if (sharedMemory == NULL) {
        PrintErrorMsg(GetLastError(), _T("Erro em MapViewOfFile"));
        CloseHandle(hMutex_SHM);
        CloseHandle(hEvents[0]);
        CloseHandle(hMap);
        return 1;
    }

    threadData.continua = TRUE;
    threadData.hEvent_Exit = &hEvents[1];

    threadData.hMutex = CreateMutex(NULL, FALSE, NULL);
    if (threadData.hMutex == NULL) {
        PrintErrorMsg(GetLastError(), _T("Erro em CreateMutex"));
        CloseHandle(hMutex_SHM);
        CloseHandle(hEvents[0]);
        CloseHandle(hMap);
        return 1;
    }

    hThread = CreateThread(NULL, 0, ThreadGetChar, (LPVOID)&threadData, 0, &threadId);
    if (hThread == NULL) {
        PrintErrorMsg(GetLastError(), _T("Erro ao lançar ThreadRead"));
        CloseHandle(hMutex_SHM);
        CloseHandle(hEvents[0]);
        CloseHandle(hMap);
        CloseHandle(threadData.hMutex);
        return 1;
    }

    WaitForSingleObject(hMutex_SHM, INFINITE);
    CopyMemory(empresas, sharedMemory->empresas, N * sizeof(EMPRESA));
    ReleaseMutex(hMutex_SHM);

    while (continua) {
        PrintTop(empresas, N);
        _tprintf_s(_T("\n\nPrima ENTER para terminar..."));

        WaitForMultipleObjects(2, hEvents, FALSE, INFINITE);

        system("cls");

        WaitForSingleObject(threadData.hMutex, INFINITE);
        CopyMemory(empresas, sharedMemory->empresas, sizeof(EMPRESA) * 10);
        continua = threadData.continua;
        ReleaseMutex(threadData.hMutex);
    }

    CloseHandle(hThread);

    FlushViewOfFile(sharedMemory, 0);

    CloseHandle(threadData.hMutex);

    CloseHandle(hMap);

    CloseHandle(hEvents[0]);

    CloseHandle(hMutex_SHM);

    return 0;
}