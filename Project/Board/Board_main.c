#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <fcntl.h> 
#include <io.h>
#include <Utils.h>

int _tmain(int argc, TCHAR* argv[]) {
	int setmodeReturn;
	DWORD numDeEmpresas;
  STRUCT_MEMORIA_VIRTUAL* dadosMemoria;
  HANDLE hMapFile, hEvent;



#ifdef UNICODE
	setmodeReturn = _setmode(_fileno(stdin), _O_WTEXT);
	setmodeReturn = _setmode(_fileno(stdout), _O_WTEXT);
	setmodeReturn = _setmode(_fileno(stderr), _O_WTEXT);
#endif 

	if (argc != 2 || (numDeEmpresas = _wtoi(argv[1])) <= 0) {
		_tprintf_s(_T("O programa Board recebe 1 e apenas 1 argumento de entrada\nsendo este o n.º de empresas a listar"));
		return 1;
	}
    numDeEmpresas = numDeEmpresas >= 10 ? 10 : numDeEmpresas;

    hEvent = OpenEvent(SYNCHRONIZE, FALSE, NOME_DO_EVENTO_PARA_AVISAR_BOARD);
    if (hEvent == NULL) {
        _tprintf(_T("Erro ao abrir o evento. Código de erro: %d OU o server ainda nao esta aberto\n"), GetLastError());
        return 1;
    }

    hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(STRUCT_MEMORIA_VIRTUAL), NOME_DO_FILE_MEMORIA_VIRTUAL);
    if (hMapFile == NULL) {
        printf("Erro ao abrir o mapeamento de arquivo. Código de erro: %d\n", GetLastError());
        return 1;
    }

    // Mapear a memória compartilhada para o espaço de endereço do processo
    dadosMemoria = (EMPRESA*)MapViewOfFile(hMapFile, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
    if (dadosMemoria == NULL) {
        printf("Erro ao mapear a memória compartilhada. Código de erro: %d\n", GetLastError());
        CloseHandle(hMapFile);
        return 1;
    }

    while(dadosMemoria->continuar){
        WaitForSingleObject(hEvent, INFINITE);
        for (int i = 0; i < (numDeEmpresas > dadosMemoria->numDeEmpresasExistentes ? dadosMemoria->numDeEmpresasExistentes : numDeEmpresas) ; ++i) {
            _tprintf(_T("Nome: %s, Preço: %.2lf, Número de Ações: %u\n"), dadosMemoria->empresas[i].nome, dadosMemoria->empresas[i].preco, dadosMemoria->empresas[i].numDeAcao);
        }
        _tprintf(_T("\n \n \n \n %s \n \n \n"), dadosMemoria->continuar ? _T("") : _T("\t\tO SERVIDOR FOI FECHADO :("));
    }

    UnmapViewOfFile(dadosMemoria);
    CloseHandle(hMapFile);
    CloseHandle(hEvent);
	return 0;
}
