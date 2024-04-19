#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <fcntl.h> 
#include <io.h>

#include "Utils.h"
#include "Board.h"

DWORD WINAPI ThreadGetChar(LPVOID data) {
    TDATA_BOARD* ptd = (TDATA_BOARD*)data;

    TCHAR c =  _gettchar();

    WaitForSingleObject(ptd->hMutex, INFINITE);
    ptd->continua = FALSE;
    ReleaseMutex(ptd->hMutex);

    SetEvent(ptd->hEvent_Exit);

    ExitThread(6);
}

void PrintTop(const EMPRESA empresas[], DWORD numTop) {
    if (empresas[0].numAcoes == 0) { return; }

    _tprintf_s(_T("\nTOP %d \nEmpresas mais bem cotadas da bolsa"), numTop);
    for (DWORD i = 0; i < numTop; i++) {
        if (empresas[i].numAcoes > 0) {
            _tprintf_s(_T("\n\n%d.º\n--------------------------------------"), i+1);
            PrintEmpresa(empresas[i]);
        }
    }
}