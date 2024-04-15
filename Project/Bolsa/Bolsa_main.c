#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <fcntl.h> 
#include <io.h>

#include "Commands.h"
#include "Utils.h"

#include "Bolsa.h"

int _tmain(int argc, TCHAR* argv[]) {
	int setmodeReturn;

	DWORD numClintes = -1;
	TCHAR c;

	// Buffer para guardar mensagens de erro
	TCHAR msg[TAM];

	// Buffer para guardar o comando recebido
	TCHAR input[TAM];

	// Estrutura comando
	CMD comando;

	// Dados das empreas 
	EMPRESA empresas[NUMERO_MAX_DE_EMPRESAS];
	DWORD numDeEmpresas = 0; // número de empresas existentes

	// Dados dos users 
	CARTEIRA users[NUMERO_MAX_DE_USERS];
	DWORD numUsers = 0; // número de carteiras existentes 

	// Variáveis relativas às Threads
	TDATA_BOLSA threadData;
	HANDLE hThread[3+NUMERO_MAX_DE_USERS];
	DWORD threadId;

#ifdef UNICODE
	setmodeReturn = _setmode(_fileno(stdin), _O_WTEXT);
	setmodeReturn = _setmode(_fileno(stdout), _O_WTEXT);
	setmodeReturn = _setmode(_fileno(stderr), _O_WTEXT);
#endif 

	if ((numClintes = getNCLIENTES()) == -1) { return -1; }

	threadData.empresas = empresas;
	threadData.numEmpresas = &numDeEmpresas;
	threadData.continua = TRUE;

	LerEmpresasDoArquivo(empresas, &numDeEmpresas);
	LerUsersDoArquivo(users, &numUsers);

	// Lançamento da ThreadBoard
	hThread[0] = CreateThread(NULL, 0, ThreadBoard, (LPVOID)&threadData, 0, &threadId);
	if (hThread[0] == NULL) {
		PrintError(GetLastError(), _T("Erro ao lançar ThreadBoard"));
		return 1;
	}

	while (1) {
		GetCmd(input);
		c = _gettchar();

		if (!ValidaCmd(input, &comando, msg, TRUE)) {
			_tprintf(_T("\n[ERRO] %s."), msg);
		} else {
			ExecutaComando(comando, empresas, &numDeEmpresas, users, &numUsers, &threadData, hThread);
			if (comando.Index == 5) { break; }
		}
	}

	SalvarEmpresasNoArquivo(empresas, numDeEmpresas);
	SalvarUsersNoArquivo(users, numUsers);

	return 0;
}

