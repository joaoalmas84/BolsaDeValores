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
		_tprintf_s(_T("O programa Board recebe 1 e apenas 1 argumento de entrada\nsendo este o n.� de empresas a listar"));
		return 1;
	}
    numDeEmpresas = numDeEmpresas >= 10 ? 10 : numDeEmpresas;

    hEvent = OpenEvent(SYNCHRONIZE, FALSE, NOME_DO_EVENTO_PARA_AVISAR_BOARD);
    if (hEvent == NULL) {
        _tprintf(_T("Erro ao abrir o evento. C�digo de erro: %d OU o server ainda nao esta aberto\n"), GetLastError());
        return 1;
    }

    hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(STRUCT_MEMORIA_VIRTUAL), NOME_DO_FILE_MEMORIA_VIRTUAL);
    if (hMapFile == NULL) {
        printf("Erro ao abrir o mapeamento de arquivo. C�digo de erro: %d\n", GetLastError());
        return 1;
    }

    // Mapear a mem�ria compartilhada para o espa�o de endere�o do processo
    dadosMemoria = (EMPRESA*)MapViewOfFile(hMapFile, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
    if (dadosMemoria == NULL) {
        printf("Erro ao mapear a mem�ria compartilhada. C�digo de erro: %d\n", GetLastError());
        CloseHandle(hMapFile);
        return 1;
    }

    while(dadosMemoria->continuar){
        WaitForSingleObject(hEvent, INFINITE);
        for (int i = 0; i < (numDeEmpresas > dadosMemoria->numDeEmpresasExistentes ? dadosMemoria->numDeEmpresasExistentes : numDeEmpresas) ; ++i) {
            _tprintf(_T("Nome: %s, Pre�o: %.2lf, N�mero de A��es: %u\n"), dadosMemoria->empresas[i].nome, dadosMemoria->empresas[i].preco, dadosMemoria->empresas[i].numDeAcao);
        }
        _tprintf(_T("\n \n \n \n %s \n \n \n"), dadosMemoria->continuar ? _T("") : _T("\t\tO SERVIDOR FOI FECHADO :("));
    }

    UnmapViewOfFile(dadosMemoria);
    CloseHandle(hMapFile);
    CloseHandle(hEvent);
	return 0;
}





/*int _tmain(int argc, TCHAR* argv[]) {
	int setmodeReturn;
	DWORD numDeEmpresas;
	EMPRESA *empresas;
    HANDLE hFile, hMapFile;



#ifdef UNICODE
	setmodeReturn = _setmode(_fileno(stdin), _O_WTEXT);
	setmodeReturn = _setmode(_fileno(stdout), _O_WTEXT);
	setmodeReturn = _setmode(_fileno(stderr), _O_WTEXT);
#endif 

	if (argc != 2 || (numDeEmpresas = _wtoi(argv[1])) <= 0) {
		_tprintf_s(_T("O programa Board recebe 1 e apenas 1 argumento de entrada\nsendo este o n.� de empresas a listar"));
		return 1;
	}

    hFile = CreateFile(
        TEXT("filelll.bin"),        // Nome do arquivo bin�rio
        GENERIC_READ,               // Acesso de leitura
        0,                          // Compartilhamento (n�o especificado neste exemplo)
        NULL,                       // Par�metros de seguran�a (n�o especificados neste exemplo)
        OPEN_EXISTING,              // Abre o arquivo se ele existir
        FILE_ATTRIBUTE_NORMAL,      // Atributos do arquivo (normal)
        NULL                        // Handle do arquivo para cria��o (n�o especificado neste exemplo)
    );
    if (hFile == INVALID_HANDLE_VALUE) {
        PrintError(GetLastError(), _T("Erro ao abrir o arquivo. C�digo de erro: %d\n"));
        return 1;
    }

    // Cria um mapeamento do arquivo na mem�ria
    hMapFile = CreateFileMapping(
        hFile,                      // Handle do arquivo
        NULL,                       // Atributos de seguran�a (n�o especificados neste exemplo)
        PAGE_READONLY,              // Prote��o de p�gina (somente leitura)
        0,                          // Tamanho m�ximo alto do mapeamento (n�o especificado neste exemplo)
        0,                          // Tamanho m�ximo baixo do mapeamento (n�o especificado neste exemplo)
        NULL                        // Nome do mapeamento (n�o especificado neste exemplo)
    );
    if (hMapFile == NULL) {
        PrintError(GetLastError(), _T("Erro ao criar o mapeamento do arquivo. C�digo de erro: %d\n"));
        CloseHandle(hFile);
        return 1;
    }

    // Mapeia a visualiza��o do arquivo na mem�ria
    pBuf = MapViewOfFile(
        hMapFile,               // Handle do mapeamento de arquivo
        FILE_MAP_READ,              // Acesso de leitura
        0,                          // Offset alto (n�o especificado neste exemplo)
        0,                          // Offset baixo (n�o especificado neste exemplo)
        10 * sizeof(EMPRESA)        // Tamanho da visualiza��o (10 * tamanho da estrutura EMPRESA)
    );
    if (pBuf == NULL) {
        PrintError(GetLastError(), _T("Erro ao mapear a visualiza��o do arquivo. C�digo de erro: %d\n"));
        CloseHandle(hMapFile);
        CloseHandle(hFile);
        return 1;
    }

    // Converte o buffer mapeado para uma estrutura
    empresas = (EMPRESA*)pBuf;

    for (int i = 0; i < 100; i++) {
        Sleep(1000);
        for (int i = 0; i < 10; ++i) {
            _tprintf(TEXT("Nome: %s, Pre�o: %u, N�mero de A��es: %u\n"), empresas[i].nome, empresas[i].preco, empresas[i].numDeAcao);
        }
    }

	return 0;
}*/