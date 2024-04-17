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

	DWORD numClients = -1;
	TCHAR c;
	
	// Semaforo que impede a existencia de mais que 1 processo bolsa em simultaneo
	HANDLE hSem;

	// Buffer para guardar mensagens de erro
	TCHAR msg[TAM];

	// Buffer para guardar o comando recebido
	TCHAR input[TAM];

	// Estrutura comando
	CMD comando;

	// Dados das empreas 
	EMPRESA empresas[MAX_EMPRESAS];
	DWORD numDeEmpresas = 0; // número de empresas existentes

	// Dados dos users 
	USER users[MAX_USERS];
	DWORD numUsers = 0; // número de carteiras existentes 

	// Variáveis relativas às Threads
	TDATA_BOLSA threadData;
	HANDLE hThread[3+MAX_USERS];
	DWORD threadId;

#ifdef UNICODE
	setmodeReturn = _setmode(_fileno(stdin), _O_WTEXT);
	setmodeReturn = _setmode(_fileno(stdout), _O_WTEXT);
	setmodeReturn = _setmode(_fileno(stderr), _O_WTEXT);
#endif 

	hSem = CreateSemaphore(NULL, 1, 1, SEMAPHORE);
	if (hSem == NULL) {
		PrintError(GetLastError(), _T("Erro em CreateSemaphore"));
		return 1;
	}

	if (WaitForSingleObject(hSem, 100) == WAIT_TIMEOUT) {
		_tprintf_s(_T("\nJá existe um processo Bolsa em execução."));
		return 1;
	}

	numClients = getNCLIENTES();
	if (numClients < 0) { return -1; }

	InitializeEmpresas(empresas);
	InitializeUsers(users);

	//LerEmpresasDoArquivo(empresas, &numDeEmpresas);
	LerUsersDoArquivo(users, &numUsers);

	threadData.continua = TRUE;
	threadData.empresas = empresas;
	threadData.numEmpresas = &numDeEmpresas;
	threadData.clients = users;
	threadData.numClients = &numClients;

	threadData.hEvent_Board = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (threadData.hEvent_Board == NULL) {
		PrintError(GetLastError(), _T("Erro ao lançar CreateEvent"));
		return 1;
	}

	threadData.hMutex = CreateMutex(NULL, FALSE, NULL);
	if (threadData.hEvent_Board == NULL) {
		PrintError(GetLastError(), _T("Erro ao lançar CreateMutex"));
		return 1;
	}

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
			ExecutaComando(comando, &threadData);
			if (comando.Index == 5) { break; }
		}
	}

	SalvarEmpresasNoArquivo(empresas, numDeEmpresas);
	SalvarUsersNoArquivo(users, numUsers);

	return 0;
}

