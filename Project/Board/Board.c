#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <fcntl.h> 
#include <io.h>

#include "Utils.h"
#include "Structs.h"

#include "Board.h"

DWORD WINAPI ThreadGetChar(LPVOID data) {
    TDATA_BOARD* ptd = (TDATA_BOARD*)data;
    TCHAR input[SMALL_TEXT];

    _getts_s(input, SMALL_TEXT);

    EnterCriticalSection(ptd->pCs);
    ptd->continua = FALSE;
    LeaveCriticalSection(ptd->pCs);

    SetEvent(ptd->hEvent_Exit);

    ExitThread(6);
}

void PrintTop(const EMPRESA empresas[], DWORD numTop, DWORD numEmpresas) {
    if (numEmpresas <= 0) { return; }

    if (numEmpresas < numTop) {
        _tprintf_s(_T("\nTOP %d \nEmpresas mais bem cotadas da bolsa"), numEmpresas);
        PrintEmpresas(empresas, numEmpresas);
    } else {
        _tprintf_s(_T("\nTOP %d \nEmpresas mais bem cotadas da bolsa"), numTop);
        PrintEmpresas(empresas, numTop);
    }

}