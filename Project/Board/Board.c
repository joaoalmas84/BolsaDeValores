#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <fcntl.h> 
#include <io.h>

#include "Utils.h"
#include "Board.h"

DWORD WINAPI ThreadRead(LPVOID data) {
    TDATA_BOARD* ptd = (TDATA_BOARD*)data;

    // Variaveis da SharedMemory
    SHM* sharedMemory;
    HANDLE hMapFile;
    HANDLE hMutex_SHM;

    // Variaveis locais
    BOOL continua;
    DWORD numTop;
    EMPRESA empresas[MAX_EMPRESAS_TO_SHM];

    hMapFile = OpenFileMapping(FILE_MAP_READ, FALSE, SHARED_MEMORY);
    if (hMapFile == NULL) {
        PrintError(GetLastError(), _T("Erro em OpenFileMapping"));
        return 1;
    }

    sharedMemory = (SHM*)MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, 0);
    if (sharedMemory == NULL) {
        PrintError(GetLastError(), _T("Erro em MapViewOfFile"));
        CloseHandle(hMapFile);
        return 1;
    }

    hMutex_SHM = OpenMutex(SYNCHRONIZE, FALSE, SHM_MUTEX);
    if (hMutex_SHM == NULL) {
        PrintError(GetLastError(), _T("Erro em OpenMutex"));
        CloseHandle(hMapFile);
        return 1;
    }

    WaitForSingleObject(ptd->hMutex, INFINITE);
    numTop = ptd->numTop;
    ReleaseMutex(ptd->hMutex);

    WaitForSingleObject(hMutex_SHM, INFINITE);
    for (size_t i = 0; i < numTop; i++) {
        empresas[i] = sharedMemory->empresas[i];
    }
    ReleaseMutex(hMutex_SHM);

    PrintTop(empresas, numTop);
    _tprintf_s(_T("\n\nPrima ENTER para terminar..."));

    while (1) {
        WaitForMultipleObjects(2, ptd->hEvents, FALSE, INFINITE);

        system("cls");

        WaitForSingleObject(ptd->hMutex, INFINITE);
        continua = ptd->continua;
        ReleaseMutex(ptd->hMutex);

        if (!continua) { break; }

        WaitForSingleObject(hMutex_SHM, INFINITE);
        for (size_t i = 0; i < numTop; i++) {
            empresas[i] = sharedMemory->empresas[i];
        }
        ReleaseMutex(hMutex_SHM);

        //qsort_s(empresas, MAX_EMPRESAS_TO_SHM, sizeof(EMPRESA), )

        PrintTop(empresas, numTop);
        _tprintf_s(_T("\n\nPrima ENTER para terminar..."));
    }

    FlushViewOfFile(sharedMemory, 0);

    UnmapViewOfFile(sharedMemory);

    CloseHandle(hMutex_SHM);

    CloseHandle(hMapFile);

    ExitThread(6);
}

void PrintTop(EMPRESA empresas[], DWORD numTop) {
    if (empresas[0].numAcoes == 0) { return; }

    _tprintf_s(_T("\nTOP %d \nEmpresas mais bem cotadas da bolsa"), numTop);
    for (DWORD i = 0; i < numTop; i++) {
        if (empresas[i].numAcoes > 0) {
            _tprintf_s(_T("\n\n%d.º\n--------------------------------------"), i+1);
            PrintEmpresa(empresas[i]);
        }
    }
}