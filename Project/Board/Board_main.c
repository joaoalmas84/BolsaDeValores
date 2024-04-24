#include <windows.h>
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

    DWORD N; // Numero de empresas a listar (tamanho do top)
    
    EMPRESA empresas[MAX_EMPRESAS_TO_SHM];
    BOOL continua = TRUE;

    // Variáveis da Thread
    DWORD threadId;
    HANDLE hThread;
    TDATA_BOARD threadData;
    CRITICAL_SECTION cs;

    // Variaveis da SharedMemory
    SHM* sharedMemory;
    HANDLE hMap;
    HANDLE hMutex_SHM;
    HANDLE hEvent_SHM; // Evento SHM (Reset Manual);

    // Array de Eventos para o WaitForMultipleObjects();
    HANDLE hEvents[2]; // [0]: Evento_SHM; [1]: Evento_Exit
    
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
            PrintErrorMsg(codigoErro, _T("Erro em OpenMutex."));
        }
        return 1;
    }

    hMap = OpenFileMapping(FILE_MAP_READ, FALSE, SHARED_MEMORY);
    if (hMap == NULL) {
        PrintErrorMsg(GetLastError(), _T("Erro em OpenFileMapping"));
        CloseHandle(hMutex_SHM);
        return 1;
    }

    sharedMemory = (SHM*)MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
    if (sharedMemory == NULL) {
        PrintErrorMsg(GetLastError(), _T("Erro em MapViewOfFile"));
        CloseHandle(hMutex_SHM);
        CloseHandle(hMap);
        return 1;
    }

    hEvent_SHM = OpenEvent(SYNCHRONIZE, FALSE, SHM_EVENT);
    if (hEvent_SHM == NULL) {
        PrintErrorMsg(GetLastError(), _T("Erro em OpenEvent"));
        CloseHandle(hMutex_SHM);
        CloseHandle(hMap);
        return 1;
    }

    if (!InitializeCriticalSectionAndSpinCount(&cs, 0)) {
        PrintErrorMsg(GetLastError(), _T("Erro em InitializeCriticalSectionAndSpinCount"));
        CloseHandle(hMutex_SHM);
        CloseHandle(hEvent_SHM);
        CloseHandle(hMap);
        return 1;
    }

    threadData.pCs = &cs;
    threadData.continua = &continua;

    threadData.hEvent_Exit = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (threadData.hEvent_Exit == NULL) {
        PrintErrorMsg(GetLastError(), _T("Erro em CreateEvent"));
        CloseHandle(hMutex_SHM);
        CloseHandle(hEvent_SHM);
        CloseHandle(hMap);
        return 1;
    }

    hThread = CreateThread(NULL, 0, ThreadGetChar, (LPVOID)&threadData, 0, &threadId);
    if (hThread == NULL) {
        PrintErrorMsg(GetLastError(), _T("Erro ao lançar ThreadRead"));
        CloseHandle(hMutex_SHM);
        CloseHandle(hEvent_SHM);
        CloseHandle(hMap);
        return 1;
    }

    WaitForSingleObject(hMutex_SHM, INFINITE);
    CopyMemory(empresas, sharedMemory->empresas, sizeof(EMPRESA) * MAX_EMPRESAS_TO_SHM);
    ReleaseMutex(hMutex_SHM);

    hEvents[0] = hEvent_SHM;
    hEvents[1] = threadData.hEvent_Exit;

    while (continua) {
        system("cls");

        PrintTop(empresas, N);
        _tprintf_s(_T("\n\nPrima ENTER para terminar..."));

        WaitForMultipleObjects(2, hEvents, FALSE, INFINITE);

        EnterCriticalSection(&cs);
        CopyMemory(empresas, sharedMemory->empresas, sizeof(EMPRESA) * MAX_EMPRESAS_TO_SHM);
        continua = *threadData.continua;
        LeaveCriticalSection(&cs);
    }

    WaitForSingleObject(hThread, INFINITE);

    CloseHandle(hThread);

    DeleteCriticalSection(&cs);

    FlushViewOfFile(sharedMemory, 0);

    CloseHandle(hMap);

    CloseHandle(hMutex_SHM);

    return 0;
}