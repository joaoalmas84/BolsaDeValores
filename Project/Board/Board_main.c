#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <fcntl.h> 
#include <io.h>

#include "Utils.h"
#include "Board.h"

int _tmain(int argc, TCHAR* argv[]) {
    int setmodeReturn;

    // Numero de empresas a listar
    DWORD N;

    // Variáveis da Thread
    DWORD threadId;
    HANDLE hThread;
    TDATA_BOARD threadData;

    TCHAR c;

#ifdef UNICODE
    setmodeReturn = _setmode(_fileno(stdin), _O_WTEXT);
    setmodeReturn = _setmode(_fileno(stdout), _O_WTEXT);
    setmodeReturn = _setmode(_fileno(stderr), _O_WTEXT);
#endif 

    if (argc != 2 ) {
        _tprintf_s(_T("O programa Board recebe 1 e apenas 1 argumento de entrada\nsendo este o n.º de empresas a listar"));
        return 1;
    } else if (_wtoi(argv[1]) <= 0) {
        _tprintf_s(_T("O n.º de empresas a listar é inválido!"));
        return 1;
    }

    N = _wtoi(argv[1]);

    if (N > 10) { N = 10; }

    threadData.hEvents[0] = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (threadData.hEvents[0] == NULL) {
        PrintErrorMsg(GetLastError(), _T("Erro em CreatEvent"));
        return 1;
    }

    threadData.hEvents[1] = OpenEvent(SYNCHRONIZE, FALSE, SHM_EVENT);
    if (threadData.hEvents[1] == NULL) {
        PrintErrorMsg(GetLastError(), _T("Erro em OpenEvent"));
        return 1;
    }

    threadData.hMutex = CreateMutex(NULL, FALSE, NULL);
    if (threadData.hMutex == NULL) {
        PrintErrorMsg(GetLastError(), _T("Erro em CreateMutex"));
        CloseHandle(threadData.hEvents[1]);
        return 1;
    }

    threadData.continua = TRUE;
    threadData.numTop = N;

    hThread = CreateThread(NULL, 0, ThreadRead, (LPVOID)&threadData, 0, &threadId);
    if (hThread == NULL) {
        PrintErrorMsg(GetLastError(), _T("Erro ao lançar ThreadRead"));
        return 1;
    }

    c = _gettchar();
    
    WaitForSingleObject(threadData.hMutex, INFINITE);
    threadData.continua = FALSE;
    ReleaseMutex(threadData.hMutex);

    SetEvent(threadData.hEvents[0]);

    WaitForSingleObject(hThread, INFINITE);

    CloseHandle(hThread);

    CloseHandle(threadData.hMutex);

    CloseHandle(threadData.hEvents[1]);

    CloseHandle(threadData.hEvents[0]);

    return 0;
}