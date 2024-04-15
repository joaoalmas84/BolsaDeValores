﻿#include "Board.h"

int _tmain(int argc, TCHAR* argv[]) {
    int setmodeReturn;

    DWORD numEmpresas, threadId;
    SHM* dadosMemoria;
    HANDLE hMapFile, hEvent, hThreadOutput;
    BOOL continuar = TRUE;

#ifdef UNICODE
    setmodeReturn = _setmode(_fileno(stdin), _O_WTEXT);
    setmodeReturn = _setmode(_fileno(stdout), _O_WTEXT);
    setmodeReturn = _setmode(_fileno(stderr), _O_WTEXT);
#endif 

    if (argc != 2 || (numEmpresas = _wtoi(argv[1])) <= 0) {
        _tprintf_s(_T("O programa Board recebe 1 e apenas 1 argumento de entrada\nsendo este o n.º de empresas a listar"));
        return 1;
    }
    numEmpresas = numEmpresas >= 10 ? 10 : numEmpresas;

    hThreadOutput = CreateThread(NULL, 0, ThreadOutput, (LPVOID)&continuar, 0, &threadId);
    if (hThreadOutput == NULL) {
        PrintError(GetLastError(), _T("Erro ao lançar ThreadBoard"));
        return 1;
    }

    hEvent = OpenEvent(SYNCHRONIZE, FALSE, NOME_DO_EVENTO_PARA_AVISAR_BOARD);
    if (hEvent == NULL) {
        _tprintf(_T("Erro ao abrir o evento. Codigo de erro: %d OU o server ainda nao esta aberto\n"), GetLastError());
        return 1;
    }

    hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(SHM), NOME_DO_FILE_MEMORIA_VIRTUAL);
    if (hMapFile == NULL) {
        PrintError(GetLastError(), _T("Erro em CreateFileMapping"));
        return 1;
    }

    // Mapear a mem�ria compartilhada para o espa�o de endere�o do processo
    dadosMemoria = (SHM*)MapViewOfFile(hMapFile, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
    if (dadosMemoria == NULL) {
        PrintError(GetLastError(), _T("Erro em MapViewOfFile"));
        CloseHandle(hMapFile);
        return 1;
    }

    while (dadosMemoria->continuar && continuar) {
        WaitForSingleObject(hEvent, INFINITE);
        for (DWORD i = 0; i < (numEmpresas > dadosMemoria->numEmpresas ? dadosMemoria->numEmpresas : numEmpresas); ++i) {
            _tprintf(_T("Nome: %s, Preço: %.2lf, Número de Ações: %u\n"), dadosMemoria->empresas[i].nome, dadosMemoria->empresas[i].preco, dadosMemoria->empresas[i].numDeAcao);
        }
        _tprintf(_T("\n \n \n \n %s \n \n \n"), dadosMemoria->continuar ? _T("") : _T("\t\tO SERVIDOR FOI FECHADO :("));
    }

    CloseHandle(hThreadOutput);
    UnmapViewOfFile(dadosMemoria);
    CloseHandle(hMapFile);
    CloseHandle(hEvent);
    return 0;
}