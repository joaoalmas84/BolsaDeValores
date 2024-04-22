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

	TCHAR c;
	
	// Numero maximo de clientes que podem estar ligados em simultaneo
	DWORD nClientes;

	// Semaforo que impede a existencia de mais que 1 processo bolsa em simultaneo
	HANDLE hSem;

	// Buffer para guardar mensagens do developer em caso de erro
	TCHAR errorMsg[BIG_TEXT];

	// Variavel para guardar codigos de erro
	DWORD codigoErro;

	// Buffer para guardar o comando recebido
	TCHAR input[SMALL_TEXT];

	// Estrutura comando
	CMD comando;

	// Variáveis relativas às Threads
	TDATA_BOLSA threadData;
	HANDLE hThread[3+MAX_USERS];
	DWORD threadId;

#ifdef UNICODE
	setmodeReturn = _setmode(_fileno(stdin), _O_WTEXT);
	setmodeReturn = _setmode(_fileno(stdout), _O_WTEXT);
	setmodeReturn = _setmode(_fileno(stderr), _O_WTEXT);
#endif 

	hSem = CreateSemaphore(NULL, 1, 1, SEM_BOLSA);
	if (hSem == NULL) {
		PrintErrorMsg(GetLastError(), _T("Erro em CreateSemaphore"));
		return 1;
	}

	if (WaitForSingleObject(hSem, 100) == WAIT_TIMEOUT) {
		_tprintf_s(_T("\nJá existe um processo Bolsa em execução."));
		return 1;
	}

	nClientes = getNCLIENTES();
	if (nClientes < 0) { 
		_tprintf_s(_T("\nValor da RegKey %s inválido!"), _NCLIENTES);
		return 1; 
	}

	threadData.continua = TRUE;
	threadData.numEmpresas = 0;
	threadData.numUsers = 0;
	
	threadData.empresas = AlocaEmpresas();
	if (threadData.empresas == NULL) {
		_tprintf_s(_T("\nErro ao alocar memória para empresas."));
		return 1;
	}

	threadData.users = AlocaUsers();
	if (threadData.users == NULL) {
		_tprintf_s(_T("\nErro ao alocar memória para users."));
		return 1;
	}

	if (FileExists(FILE_EMPRESAS)) {
		if (!CarregaEmpresas(threadData.empresas, &threadData.numEmpresas, errorMsg, &codigoErro)) {
			PrintErrorMsg(codigoErro, errorMsg);
			return 1;
		}
	}

	_tprintf_s(_T("\nEmpresas do File:"));
	for (DWORD i = 0; i < threadData.numEmpresas; i++) {
		PrintEmpresa(threadData.empresas[i]);
	}

	exit(0);

	/*if (FileExists(FILE_USERS)) {
		if (!CarregaUsers(threadData.users, &threadData.numUsers, errorMsg, &codigoErro)) {
			PrintErrorMsg(codigoErro, errorMsg);
			return 1;
		}
	}*/

	threadData.hEvent_Board = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (threadData.hEvent_Board == NULL) {
		PrintErrorMsg(GetLastError(), _T("Erro ao lançar CreateEvent"));
		return 1;
	}

	threadData.hMutex = CreateMutex(NULL, FALSE, NULL);
	if (threadData.hEvent_Board == NULL) {
		PrintErrorMsg(GetLastError(), _T("Erro ao lançar CreateMutex"));
		return 1;
	}

	// Lançamento da ThreadBoard
	hThread[0] = CreateThread(NULL, 0, ThreadBoard, (LPVOID)&threadData, 0, &threadId);
	if (hThread[0] == NULL) {
		PrintErrorMsg(GetLastError(), _T("Erro ao lançar ThreadBoard"));
		return 1;
	}

	system("cls");

	while (1) {
		GetCmd(input);
		c = _gettchar();

		if (!ValidaCmd(input, &comando, errorMsg, TRUE)) {
			_tprintf(_T("\n[ERRO] %s."), errorMsg);
		} else {
			ExecutaComando(comando, &threadData);
			if (comando.Index == 5) { break; }
		}
	}

	//SalvaEmpresas(empresas, numEmpresas);
	//SalvaUsers(users, numUsers);

	free(threadData.users);

	free(threadData.empresas);

	WaitForSingleObject(hThread[0], INFINITE);

	CloseHandle(threadData.hMutex);

	CloseHandle(threadData.hEvent_Board);

	CloseHandle(hSem);

	return 0;
}

