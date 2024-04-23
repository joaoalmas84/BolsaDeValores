#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <fcntl.h> 
#include <io.h>

#include "Commands.h"
#include "Utils.h"

#include "Bolsa.h"

int _tmain(int argc, TCHAR* argv[]) {
	int setmodeReturn;	// Evitar o warning da funcao setmode();

	// Semaforo que impede a existencia de mais que 1 processo bolsa em simultaneo
	HANDLE hSem;

	// Numero maximo de clientes que podem estar ligados em simultaneo (RegKey NCLIENTES)
	DWORD nClientes;

	// Array de empresas alocado dinamicamente para evitar exceder o limite da stack da função main
	EMPRESA* empresas;		// O seu tamanho sera MAX_EMPRESAS
	DWORD numEmpresas = 0;	// Numero de empresas registadas

	// Array de users registados alocado dinamicamente para evitar exceder o limite da stack da função main
	USER* users;		// O seu tamanho sera MAX_USERS
	DWORD numUsers = 0; // Numero de users registados

	// Flag para terminar as threads
	BOOL continua = TRUE;				

	// Variáveis relativas às Threads
	TDATA_BOLSA threadData;
	HANDLE hThread[3 + MAX_USERS];
	DWORD threadId[3 + MAX_USERS];

	// Variaveis para lidar com casos de erro
	TCHAR errorMsg[BIG_TEXT];	// Buffer para guardar mensagens do developer em caso de erro
	DWORD codigoErro;			// Variavel para guardar codigos de erro

	// Variaveis para lidar com os comandos
	CMD comando;				// Estrutura comando
	TCHAR input[SMALL_TEXT];	// Buffer para guardar o comando recebido

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

	empresas = AlocaEmpresas();
	if (empresas == NULL) {
		_tprintf_s(_T("\nErro ao alocar memória para empresas."));
		return 1;
	}

	users = AlocaUsers();
	if (users == NULL) {
		_tprintf_s(_T("\nErro ao alocar memória para users."));
		return 1;
	}

	threadData.continua = &continua;
	threadData.empresas = empresas;
	threadData.numEmpresas = &numEmpresas;
	threadData.users = users;
	threadData.numUsers = &numUsers;

	threadData.hEvent_Board = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (threadData.hEvent_Board == NULL) {
		PrintErrorMsg(GetLastError(), _T("Erro em CreateEvent"));
		return 1;
	}

	threadData.hMutex = CreateMutex(NULL, FALSE, NULL);
	if (threadData.hEvent_Board == NULL) {
		PrintErrorMsg(GetLastError(), _T("Erro em CreateMutex"));
		return 1;
	}

	if (FileExists(FILE_EMPRESAS)) {
		if (!CarregaEmpresas(empresas, &numEmpresas, errorMsg, &codigoErro)) {
			PrintErrorMsg(codigoErro, errorMsg);
			return 1;
		}
	}

	qsort(empresas, numEmpresas, sizeof(EMPRESA), ComparaEmpresas);

	if (FileExists(FILE_USERS)) {
		if (!CarregaUsers(users, &numUsers, errorMsg, &codigoErro)) {
			PrintErrorMsg(codigoErro, errorMsg);
			return 1;
		}
	}

	// Lançamento da ThreadBoard 
	hThread[0] = CreateThread(NULL, 0, ThreadBoard, (LPVOID)&threadData, 0, &threadId[0]);
	if (hThread[0] == NULL) {
		PrintErrorMsg(GetLastError(), _T("Erro ao lançar ThreadBoard"));
		return 1;
	}

	system("cls");

	while (1) {
		if (!GetCmd(input)) { continue; }

		if (!ValidaCmd(input, &comando, errorMsg, TRUE)) {
			_tprintf(_T("\n[ERRO] %s."), errorMsg);
		} else {
			ExecutaComando(comando, &threadData);
			if (comando.Index == 5) { break; }
		}
	}

	//SalvaEmpresas(empresas, numEmpresas);
	//SalvaUsers(users, numUsers);

	WaitForSingleObject(hThread[0], INFINITE);

	CloseHandle(threadData.hMutex);

	CloseHandle(threadData.hEvent_Board);

	free(users);

	free(empresas);

	CloseHandle(hSem);

	return 0;
}

