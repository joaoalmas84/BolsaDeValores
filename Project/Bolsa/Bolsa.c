#include <Windows.h>
#include <tchar.h>
#include <stdio.h>

#include "Commands.h"
#include "Utils.h"
#include "Structs.h"

#include "Bolsa.h"

//|=========================================================================|
//|===============================| Threads |===============================|
//|=========================================================================|

DWORD WINAPI ThreadBoard(LPVOID data) {
	TDATA_BOLSA* ptd = (TDATA_BOLSA*)data;

	BOOL continua = TRUE; // Flag que ira imitar a flag continua na estrutura "TDATA_BOLSA"

	HANDLE hMap;	// HANDLE do FileMapping
	HANDLE hMutex;	// Mutex para proteger alteracoes a sharedMemory
	HANDLE hEvent;	// Evento que avisa a board de que a sharedMemory foi atualizada

	SHM* sharedMemory; // Ponteiro para a SharedMemory

	hMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(SHM), SHARED_MEMORY);
	if (hMap == NULL) {
		PrintErrorMsg(GetLastError(), _T("Erro em CreateFileMapping"));
		return 0;
	}

	sharedMemory = (SHM*)MapViewOfFile(hMap, FILE_MAP_WRITE, 0, 0, 0);
	if (sharedMemory == NULL) {
		PrintErrorMsg(GetLastError(), _T("Erro em MapViewOfFile"));
		CloseHandle(hMap);
		return 0;
	}

	hMutex = CreateMutex(NULL, FALSE, SHM_MUTEX);
	if (hMutex == NULL) {
		PrintErrorMsg(GetLastError(), _T("Erro em CreateMutex"));
		CloseHandle(hMap);
		return 0;
	}

	hEvent = CreateEvent(NULL, TRUE, FALSE, SHM_EVENT);
	if (hEvent == NULL) {
		PrintErrorMsg(GetLastError(), _T("Erro em CreateEvent"));
		return 1;
	}

	while (1) {
		EnterCriticalSection(ptd->pCs);
		continua = *ptd->continua;
		LeaveCriticalSection(ptd->pCs);

		if (!continua) { break; }

		EnterCriticalSection(ptd->pCs);
		ZeroMemory(sharedMemory, sizeof(sharedMemory));
		CopyMemory(sharedMemory->empresas, ptd->empresas, sizeof(EMPRESA)*MAX_EMPRESAS_TO_SHM);
		LeaveCriticalSection(ptd->pCs);

		SetEvent(hEvent); // Avisa a Board de que pode voltar atualizar o TOP
		ResetEvent(hEvent);

		WaitForSingleObject(ptd->hEvent_Board, INFINITE); // Espera por alteracoes aos dados
	}

	FlushViewOfFile(sharedMemory, 0);

	UnmapViewOfFile(sharedMemory);

	CloseHandle(hEvent);

	CloseHandle(hMutex);

	CloseHandle(hMap);

	ExitThread(6);
}

DWORD WINAPI ThreadGetClients(LPVOID data) {
	TDATA_BOLSA* ptd = (TDATA_BOLSA*)data;

	TD_WRAPPER td_w[NCLIENTES];

	HANDLE hThreads[NCLIENTES];

	HANDLE hSemClientes;
	HANDLE hPipe;
	DWORD nclientes;
	BOOL continua;
	DWORD id;

	EnterCriticalSection(ptd->pCs);
	nclientes = ptd->nclientes;
	LeaveCriticalSection(ptd->pCs);

	_tprintf_s(_T("\n[GETCLIENTS] Semáforo '%s' criado... (CreateSemaphore)"), SEM_CLIENTES);
	hSemClientes = CreateSemaphore(NULL, nclientes, nclientes, SEM_CLIENTES);
	if (hSemClientes == NULL) {
		PrintErrorMsg(GetLastError(), _T("CreateSemaphore"));
		exit(-1);
	}

	EnterCriticalSection(ptd->pCs);
	for (DWORD i = 0; i < NCLIENTES; i++) {
		ptd->hPipes[i] = NULL;
	}
	ptd->hSemClientes = hSemClientes;
	LeaveCriticalSection(ptd->pCs);

	while (1) {

		if (WaitForSingleObject(hSemClientes, 0) == WAIT_TIMEOUT) {
			_tprintf_s(_T("\n[GETCLIENTS] N.º máximo de ligações atingido..."));
			WaitForSingleObject(hSemClientes, INFINITE);
		}

		EnterCriticalSection(ptd->pCs);
		continua = *ptd->continua;
		LeaveCriticalSection(ptd->pCs);

		if (!continua) { break; }

		Sleep(1);

		hPipe = CreateNamedPipe(PIPE_NAME, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
			PIPE_WAIT | PIPE_TYPE_BYTE | PIPE_READMODE_BYTE,
			nclientes, sizeof(PEDIDO_LOGIN), sizeof(RESPOSTA_LOGIN), 1000, NULL);
		if (hPipe == INVALID_HANDLE_VALUE) {
			PrintErrorMsg(GetLastError(), _T("CreateNamedPipe"));
			exit(-1);
		}

		_tprintf_s(_T("\n[GETCLIENTS] A aguardar por um cliente... (ConnectNamedPipe)\n"));
		if (!ConnectNamedPipe(hPipe, NULL)) {
			PrintErrorMsg(GetLastError(), _T("ConnectNamedPipe"));
			exit(-1);
		}

		id = getHandlePipeLivre(ptd);
		if (id == -1) { exit(-1); }

		EnterCriticalSection(ptd->pCs);
		ptd->hPipes[id] = hPipe;
		td_w[id].id = id;
		td_w[id].ptd = ptd;
		LeaveCriticalSection(ptd->pCs);


		hThreads[id] = CreateThread(NULL, 0, ThreadClient, (LPVOID)&td_w[id], 0, NULL);
		if (hThreads[id] == NULL) {
			PrintErrorMsg(GetLastError(), _T("CreateThread"));
			exit(-1);
		}
	}

	for (DWORD i = 0; i < NCLIENTES; i++) {
		if (ptd->hPipes[i] != NULL) {
			WaitForSingleObject(hThreads[i], INFINITE);
			CloseHandle(hThreads[i]);
		}
	}

	CloseHandle(hSemClientes);

	ExitThread(6);
}

DWORD WINAPI ThreadClient(LPVOID data) {
	TD_WRAPPER* ptd_w = (TD_WRAPPER*)data;

	HANDLE hEv_Read;
	OVERLAPPED ov;

	HANDLE hPipes[NCLIENTES];

	DWORD id, codigo, nbytes, err;

	TCHAR errorMsg[SMALL_TEXT];

	BOOL continua = TRUE;

	hEv_Read = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (hEv_Read == NULL) {
		PrintErrorMsg(GetLastError(), _T("CreateEvent"));
		exit(-1);
	}

	ZeroMemory(&ov, sizeof(OVERLAPPED));
	ov.hEvent = hEv_Read;

	EnterCriticalSection(ptd_w->ptd->pCs);
	id = ptd_w->id;
	for (DWORD i = 0; i < NCLIENTES; i++) {
		hPipes[i] = ptd_w->ptd->hPipes[i];
	}
	LeaveCriticalSection(ptd_w->ptd->pCs);

	_tprintf_s(_T("\n\n[THREAD_CLIENTE - N.º%d] Viva! Ligação estabelecida...\n"), id);

	while (continua) {

		if (!ReadFile(hPipes[id], &codigo, sizeof(DWORD), &nbytes, &ov)) {
			err = GetLastError();
			if (err == ERROR_IO_PENDING) {
				WaitForSingleObject(ov.hEvent, INFINITE);
				if (!GetOverlappedResult(hPipes[id], &ov, &nbytes, FALSE)) {
					err = GetLastError();
					if (err == ERROR_OPERATION_ABORTED) {
						break;
					} else {
						_stprintf_s(errorMsg, SMALL_TEXT, _T("[THREAD_CLIENTE - N.º%d] - GetOverlappedResult"), id);
						PrintErrorMsg(err, errorMsg);
						exit(-1);
					}
				}
				ResetEvent(ov.hEvent);
			} else {
				_stprintf_s(errorMsg, SMALL_TEXT, _T("[THREAD_CLIENTE - N.º%d] - ReadFile"), id);
				PrintErrorMsg(GetLastError(), errorMsg);
				exit(-1);
			}
		}

		EnterCriticalSection(ptd_w->ptd->pCs);
		continua = *ptd_w->ptd->continua;
		for (DWORD i = 0; i < NCLIENTES; i++) {
			hPipes[i] = ptd_w->ptd->hPipes[i];
		}
		LeaveCriticalSection(ptd_w->ptd->pCs);

		if (!continua) { break; } 

		if (!GerePedidos(id, codigo, ptd_w->ptd)) {
			_tprintf_s(_T("\n[THREAD_CLIENTE - N.º%d] O cliente desligou-se..."), id);
			break;
		}
	}
	
	ReleaseSemaphore(ptd_w->ptd->hSemClientes, 1, NULL);

	FlushFileBuffers(hPipes[id]);

	_tprintf_s(_T("\n[THREAD_CLIENTE - N.º%d] Ligação terminada... (DisconnectNamedPipe)\n"), id);
	if (!DisconnectNamedPipe(hPipes[id])) {
		_stprintf_s(errorMsg, SMALL_TEXT, _T("[THREAD_CLIENTE - N.º%d] - DisconnectNamedPipe"), id);
		PrintErrorMsg(GetLastError(), errorMsg);
		exit(-1);
	}

	CloseHandle(hPipes[id]);

	ExitThread(6);
}

//|==============================================================================================|
//|===============================| Comunicacao Cliente -> Bolsa |===============================|
//|==============================================================================================|

BOOL GerePedidos(const DWORD id, const DWORD codigo, TDATA_BOLSA* threadData) {
	HANDLE hPipe;

	// Recebe
	_LOGIN login;
	COMPRA compra;
	VENDA venda;

	// Envia
	RESPOSTA_LOGIN r_login;
	RESPOSTA_LISTA r_lista;
	RESPOSTA_COMPRA r_compra;
	RESPOSTA_VENDA r_venda;

	EnterCriticalSection(threadData->pCs);
	hPipe = threadData->hPipes[id];
	LeaveCriticalSection(threadData->pCs);

	switch (codigo) {
		case P_LOGIN:
			if (!GetLogin(id, hPipe, &login)) { return FALSE; };

			r_login = ValidaLogin(threadData, login);

			if (!SendRespostaLogin(hPipe, id, r_login)) { return FALSE; }
			break;

		case P_LISTA:

			_tprintf_s(_T("\n[THREAD_CLIENTE - N.º%d] LISTA (%d bytes)..."), id, (DWORD)sizeof(DWORD));
			break;

		case P_COMPRA: 
			_tprintf_s(_T("\n[THREAD_CLIENTE - N.º%d] COMPRA (%d bytes)..."), id, (DWORD)sizeof(DWORD));
			GetCompra(id, hPipe);
			SendRespostaCompra();
			break;

		case P_VENDA:
			_tprintf_s(_T("\n[THREAD_CLIENTE - N.º%d] VENDA (%d bytes)..."), id, (DWORD)sizeof(DWORD));
			GetVenda(id, hPipe);
			SendRespostaVenda();
			break;

		case P_BALANCE:
			_tprintf_s(_T("\n[THREAD_CLIENTE - N.º%d] BALANCE (%d bytes)..."), id, (DWORD)sizeof(DWORD));
			break;

		case P_EXIT:
			_tprintf_s(_T("\n[THREAD_CLIENTE - N.º%d] EXIT (%d bytes)..."), id, (DWORD)sizeof(DWORD));

			return FALSE;
			break;

		default:
			break;
	}

	return TRUE;
}

BOOL GetLogin(const DWORD id, const HANDLE hPipe, _LOGIN* login) {
	DWORD nbytes;
	TCHAR errorMsg[SMALL_TEXT];

	if (!ReadFile(hPipe, login, sizeof(_LOGIN), &nbytes, NULL)) {
		_stprintf_s(errorMsg, SMALL_TEXT, _T("[THREAD_CLIENTE - N.º%d] - ReadFile"), id);
		PrintErrorMsg(GetLastError(), errorMsg);
		return FALSE;
	}

	_tprintf_s(_T("\n[THREAD_CLIENTE - N.º%d] Recebi LOGIN (%d bytes)..."), id, nbytes);
	_tprintf_s(_T("\nlogin.name: '%s'\nlogin.pass: '%s'"), login->nome, login->pass);

	return TRUE;
}

BOOL GetCompra(const DWORD id, const HANDLE hPipe) {
	COMPRA compra;
	DWORD nbytes;
	TCHAR errorMsg[SMALL_TEXT];

	if (!ReadFile(hPipe, &compra, sizeof(COMPRA), &nbytes, NULL)) {
		_stprintf_s(errorMsg, SMALL_TEXT, _T("[THREAD_CLIENTE - N.º%d] - ReadFile"), id);
		PrintErrorMsg(GetLastError(), errorMsg);
		return FALSE;
	}

	_tprintf_s(_T("\n[THREAD_CLIENTE - N.º%d] Recebi COMPRA (%d bytes)..."), id, nbytes);
	_tprintf_s(_T("\ncompra.nomeEmpresa: '%s'\ncompra.numAcoes: %d"), compra.nomeEmpresa, compra.numAcoes);

	return TRUE;
}

BOOL GetVenda(const DWORD id, const HANDLE hPipe) {
	VENDA venda;
	DWORD nbytes;
	TCHAR errorMsg[SMALL_TEXT];

	if (!ReadFile(hPipe, &venda, sizeof(VENDA), &nbytes, NULL)) {
		_stprintf_s(errorMsg, SMALL_TEXT, _T("[THREAD_CLIENTE - N.º%d] - ReadFile"), id);
		PrintErrorMsg(GetLastError(), errorMsg);
		return FALSE;
	}

	_tprintf_s(_T("\n[THREAD_CLIENTE - N.º%d] Recebi VENDA (%d bytes)..."), id, nbytes);
	_tprintf_s(_T("\nvenda.nomeEmpresa: '%s'\nvenda.numAcoes: %d"), venda.nomeEmpresa, venda.numAcoes);

	return TRUE;
}

//|========================================================================================|
//|===============================| Validação de operações |===============================|
//|========================================================================================|

RESPOSTA_LOGIN ValidaLogin(TDATA_BOLSA* threadData, const _LOGIN login) {
	RESPOSTA_LOGIN r_login;

	r_login.codigo = R_LOGIN;

	EnterCriticalSection(threadData->pCs);
	for (DWORD i = 0; i < *threadData->numUsers; i++) {
		if (_tcscmp(threadData->users[i].nome, login.nome) == 0 &&
			_tcscmp(threadData->users[i].pass, login.pass) == 0) {
			
			LeaveCriticalSection(threadData->pCs);

			r_login.resultado = TRUE;
			return r_login;
		}
	}
	LeaveCriticalSection(threadData->pCs);

	r_login.resultado = FALSE;
	return r_login;
}

BOOL ValidaCompra(TDATA_BOLSA* threadData, const COMPRA compra) {

	return FALSE;
}

BOOL ValidaVenda(TDATA_BOLSA* threadData, const VENDA venda) {
	return FALSE;
}

//|==============================================================================================|
//|===============================| Comunicacao Bolsa -> Cliente |===============================|
//|==============================================================================================|

BOOL SendRespostaLogin(const HANDLE hPipe, const DWORD id, const RESPOSTA_LOGIN r_login) {
	DWORD nbytes, err;

	if (!WriteFile(hPipe, &r_login, sizeof(RESPOSTA_LOGIN), &nbytes, NULL)) {
		err = GetLastError();
		if (err == ERROR_PIPE_NOT_CONNECTED) {
			return FALSE;
		}
		else {
			PrintErrorMsg(err, _T("WriteFile"));
			exit(-1);
		}
	}
	_tprintf_s(_T("\n[THREAD_CLIENTE - N.º%d] Enviei RESPOSTA_LOGIN (%d bytes)..."), id, nbytes);

	return TRUE;
}

BOOL SendRespostaCompra() {
	return FALSE;

}

BOOL SendRespostaVenda() {
	return FALSE;

}

//|==========================================================================|
//|===============================| Comandos |===============================|
//|==========================================================================|

BOOL ExecutaComando(const CMD comando, TDATA_BOLSA* threadData, TCHAR* errorMsg) {

	switch (comando.Index) {
		case 0:
			if (!ADDC(comando, threadData, errorMsg)) { return FALSE; }
			break;

		case 1:
			LISTC(threadData);
			break;

		case 2:
			if (!STOCK(comando, threadData, errorMsg)) { return FALSE; }
			break;

		case 3:
			USERS(threadData);
			break;

		case 4:
			PAUSE();
			break;

		case 5:
			CLOSE(threadData);
			break;

		default:
			break;
	}

	// Apagar depois
	_tprintf_s(_T("\nComando: "));
	_tprintf_s(_T("\nNome: '%s'"), comando.Nome);
	_tprintf_s(_T("\nNumArgs: %d"), comando.NumArgs);
	_tprintf_s(_T("\nIndex: %d"), comando.Index);
	_tprintf_s(_T("\nArgs: "));
	for (DWORD i = 0; i < comando.NumArgs; i++) {
		_tprintf_s(_T("'%s' "), comando.Args[i]);
	}
	_tprintf_s(_T("\n\n"));
	
	return TRUE;
}

BOOL ADDC(const CMD comando, TDATA_BOLSA* threadData, TCHAR* mensagem) {
	DWORD numAcoes = _tstoi(comando.Args[2]);
	DOUBLE preco = _tcstod(comando.Args[3], NULL);
	
	DWORD numEmpresas;
	BOOL jaExiste = FALSE;

	EnterCriticalSection(threadData->pCs);
	numEmpresas = *threadData->numEmpresas;
	LeaveCriticalSection(threadData->pCs);

	if (numEmpresas >= MAX_EMPRESAS) {
		_tcscpy_s(mensagem, SMALL_TEXT, _T("O número máximo de empresas já foi atingido!"));
		return FALSE;
	}

	EnterCriticalSection(threadData->pCs);
	for (DWORD i = 0; i < numEmpresas; i++) {
		if (_tcscmp(ToLowerString(threadData->empresas[i].nome), ToLowerString(comando.Args[1])) == 0) {
			jaExiste = TRUE;
			break;
		}
	}
	LeaveCriticalSection(threadData->pCs);

	if (jaExiste) { 
		_tcscpy_s(mensagem, SMALL_TEXT, _T("Já existe uma empresa com esse nome"));
		return FALSE; 
	}

	EnterCriticalSection(threadData->pCs);
	threadData->empresas[numEmpresas].numAcoes = numAcoes;
	threadData->empresas[numEmpresas].preco = preco;
	_tcscpy_s(threadData->empresas[numEmpresas].nome, SMALL_TEXT, comando.Args[1]);
	
	(*threadData->numEmpresas)++;

	qsort(threadData->empresas, *threadData->numEmpresas, sizeof(EMPRESA), ComparaEmpresas);
	LeaveCriticalSection(threadData->pCs);

	SetEvent(threadData->hEvent_Board);
	ResetEvent(threadData->hEvent_Board);

	return TRUE;
}

void LISTC(TDATA_BOLSA* threadData) {
	EnterCriticalSection(threadData->pCs);
	PrintEmpresas(threadData->empresas, *threadData->numEmpresas);
	LeaveCriticalSection(threadData->pCs);
}

BOOL STOCK(const CMD comando, TDATA_BOLSA* threadData, TCHAR* mensagem) {
	DOUBLE preco = _tcstod(comando.Args[2], NULL);
	DWORD numEmpresas;
	DWORD i;

	EnterCriticalSection(threadData->pCs);
	numEmpresas = *threadData->numEmpresas;
	LeaveCriticalSection(threadData->pCs);

	if (numEmpresas == 0) {
		_tcscpy_s(mensagem, SMALL_TEXT, _T("Ainda não existem empresas registadas no sistema."));
		return FALSE;
	}

	EnterCriticalSection(threadData->pCs);
	for (i = 0; i < numEmpresas; i++) {
		if (_tcscmp(ToLowerString(threadData->empresas[i].nome), ToLowerString(comando.Args[1])) == 0) {
			threadData->empresas[i].preco = preco;
			qsort(threadData->empresas, numEmpresas, sizeof(EMPRESA), ComparaEmpresas);
			break;
		}
	}
	LeaveCriticalSection(threadData->pCs);

	if (i < numEmpresas) { return TRUE; }
	else {
		_stprintf_s(mensagem, SMALL_TEXT, _T("A empresa '%s' não está registada no sistema\n"), comando.Args[1]);
		return FALSE;
	}
}

void USERS(TDATA_BOLSA* threadData) {
	EnterCriticalSection(threadData->pCs);
	PrintUsers(threadData->users, *threadData->numUsers);
	LeaveCriticalSection(threadData->pCs);
}

void PAUSE() {

}

void CLOSE(TDATA_BOLSA* threadData) {
	HANDLE hPipes[NCLIENTES];

	EnterCriticalSection(threadData->pCs);
	*threadData->continua = FALSE;
	CopyMemory(hPipes, threadData->hPipes, sizeof(hPipes));
	LeaveCriticalSection(threadData->pCs);
	
	SetEvent(threadData->hEvent_Board);

	BroadcastClose(threadData, hPipes);

}

void BroadcastClose(TDATA_BOLSA* threadData, HANDLE hPipes[]) {
	DWORD nbytes, err, codigo = R_CLOSE;

	for (DWORD i = 0; i < threadData->nclientes; i++) {
		if (hPipes[i] != NULL) {

			if (!WriteFile(hPipes[i], &codigo, sizeof(DWORD), &nbytes, NULL)) {
				err = GetLastError();
				if (err == ERROR_PIPE_NOT_CONNECTED) {
					_tprintf_s(_T("\n[BOLSA] Pipe N.º %d já está fechado..."), i);
				} else {
					PrintErrorMsg(err, _T("WriteFile"));
					exit(-1);
				}
			}

		}
	}
}

//|===============================================================================================|
//|===============================| Ficheiros de Dados - Empresas |===============================|
//|===============================================================================================|

BOOL CarregaEmpresas(EMPRESA empresas[], DWORD* numEmpresas) {
	HANDLE hFile;
	char buff_c[BIG_TEXT];
	TCHAR buff_t[BIG_TEXT];
	DWORD nbytes;

	hFile = CreateFile(FILE_EMPRESAS, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		PrintErrorMsg(GetLastError(), _T("Erro em CreateFile"));
		return FALSE;
	}

	if (!ReadFile(hFile, buff_c, sizeof(buff_c)-1, &nbytes, NULL)) {
		PrintErrorMsg(GetLastError(), _T("Erro em ReadFile"));
		CloseHandle(hFile);
		return FALSE;
	}
	buff_c[nbytes/sizeof(char)] = '\0';

	// nbytes vai ser usado para outra coisa agora

	nbytes = MultiByteToWideChar(CP_UTF8, MB_COMPOSITE, buff_c, (int)strlen(buff_c), buff_t, _countof(buff_t));
	if (nbytes == 0) {
		PrintErrorMsg(GetLastError(), _T("Erro em MultiByteToWideChar"));
		CloseHandle(hFile);
		return FALSE;
	}
	buff_t[nbytes] = _T('\0');

	if (!ProcessaEmpresasDoFicheiro(buff_t, empresas, numEmpresas)) {
		_tprintf_s(_T("\nErro em _stscanf_s"));
		CloseHandle(hFile);
		return FALSE;
	}

	CloseHandle(hFile);

	return TRUE;
}

BOOL ProcessaEmpresasDoFicheiro(const TCHAR* buff, EMPRESA empresas[], DWORD* numEmpresas) {
	TCHAR *buffCopy, *str, *nextToken = NULL;

	buffCopy = _tcsdup(buff);

	str = _tcstok_s(buffCopy, _T("\n"), &nextToken);
	str[_tcslen(str)] = '\0';

	if (!GetEmpresa(str, &empresas[*numEmpresas], numEmpresas)) { return FALSE; }

	while (1) {
		str = _tcstok_s(NULL, _T("\n"), &nextToken);

		if (str == NULL) { break; }

		str[_tcslen(str)] = '\0';

		if (!GetEmpresa(str, &empresas[*numEmpresas], numEmpresas)) { return FALSE; }
	} 

	free(buffCopy);

	return TRUE;
}

BOOL GetEmpresa(const TCHAR* str, EMPRESA* empresa, DWORD* numEmpresas) {
	TCHAR line[SMALL_TEXT];
	int res;

	_tcscpy_s(line, SMALL_TEXT, str);

	res = _stscanf_s(line, _T("%s %d %lf"),
		empresa->nome,
		(unsigned)_countof(empresa->nome),
		&empresa->numAcoes,
		&empresa->preco);

	if (res != 3) { return FALSE; }

	(*numEmpresas)++;

	return TRUE;
}

BOOL SalvaEmpresas(const EMPRESA empresas[], const DWORD numEmpresas) {
	HANDLE hFile;
	TCHAR buff_t [SMALL_TEXT];
	char buff_c[SMALL_TEXT];
	DWORD nbytes, res;

	DWORD dwCreationDisposition = FileExists(FILE_EMPRESAS) ? OPEN_EXISTING : CREATE_NEW;

	hFile = CreateFile(FILE_EMPRESAS, GENERIC_WRITE, 0, NULL, dwCreationDisposition, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		PrintErrorMsg(GetLastError(), _T("Erro em CreateFile"));
		return FALSE;
	}

	for (DWORD i = 0; i < numEmpresas; i++) {
		res = _stprintf_s(buff_t, SMALL_TEXT, _T("%s %d %.2lf\n"),
			empresas[i].nome, empresas[i].numAcoes, empresas[i].preco);

		if (res == -1) {
			CloseHandle(hFile);
			return FALSE;
		}

		nbytes = WideCharToMultiByte(CP_UTF8, WC_COMPOSITECHECK, buff_t, -1, buff_c, SMALL_TEXT, NULL, NULL) ;
		if (res == 0) {
			PrintErrorMsg(GetLastError(), _T("Erro em WideCharToMultiByte"));
			return FALSE;
		}
		buff_c[nbytes / sizeof(char)] = '\0';

		// nbytes vai ser usado para outra coisa agora

		if (!WriteFile(hFile, buff_c, (DWORD)strlen(buff_c), &nbytes, NULL)) {
			PrintErrorMsg(GetLastError(), _T("Erro em WriteFile"));
			CloseHandle(hFile);
			return FALSE;
		}

		//_tprintf_s(_T("\nEscrevi %d bytes (%d)"), nbytes, i+1);

	}

	CloseHandle(hFile);
	
	return TRUE;
}

//|============================================================================================|
//|===============================| Ficheiros de Dados - Users |===============================|
//|============================================================================================|

BOOL CarregaUsers(USER users[], DWORD* numUsers) {
	HANDLE hFile;
	char buff_c[BIG_TEXT];
	TCHAR buff_t[BIG_TEXT];
	DWORD nbytes;

	hFile = CreateFile(FILE_USERS, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		PrintErrorMsg(GetLastError(), _T("Erro em CreateFile"));
		return FALSE;
	}

	if (!ReadFile(hFile, buff_c, sizeof(buff_c)-1, &nbytes, NULL)) {
		PrintErrorMsg(GetLastError(), _T("Erro em ReadFile"));
		CloseHandle(hFile);
		return FALSE;
	}
	buff_c[nbytes / sizeof(char)] = '\0';

	// nbytes vai ser usado para outra coisa agora

	nbytes = MultiByteToWideChar(CP_UTF8, MB_COMPOSITE, buff_c, (int)strlen(buff_c), buff_t, _countof(buff_t));
	if (nbytes == 0) {
		PrintErrorMsg(GetLastError(), _T("Erro em MultiByteToWideChar"));
		CloseHandle(hFile);
		return FALSE;
	}
	buff_t[nbytes] = _T('\0');

	if (!ProcessaUsersDoFicheiro(buff_t, users, numUsers)) {
		PrintErrorMsg(GetLastError(), _T("Erro em _stscanf_s"));
		CloseHandle(hFile);
		return FALSE;
	}

	CloseHandle(hFile);

	return TRUE;
}

BOOL ProcessaUsersDoFicheiro(const TCHAR* buff, USER users[], DWORD* numUsers) {
	TCHAR *buffCopy, *str, *nextToken = NULL;

	buffCopy = _tcsdup(buff);

	str = _tcstok_s(buffCopy, _T("\n"), &nextToken);
	str[_tcslen(str)] = '\0';

	if (!GetUser(str, &users[*numUsers], numUsers)) { return FALSE; }

	while (1) {
		str = _tcstok_s(NULL, _T("\n"), &nextToken);
		if (str == NULL) { break; }

		str[_tcslen(str)] = '\0';

		if (!GetUser(str, &users[*numUsers], numUsers)) { return FALSE; }
	}

	free(buffCopy);

	return TRUE;
}

BOOL GetUser(const TCHAR* str, USER* user, DWORD* numUsers) {
	TCHAR line[SMALL_TEXT];
	int res;

	_tcscpy_s(line, SMALL_TEXT, str);

	res = _stscanf_s(line, _T("%s %s %lf"),
		user->nome,
		(unsigned)_countof(user->nome),
		&user->pass,
		(unsigned)_countof(user->pass),
		&user->carteira.saldo);

	if (res != 3) { return FALSE; }

	user->ligado = FALSE;
	user->carteira.numEmpresas = 0;

	(*numUsers)++;

	return TRUE;
}

BOOL SalvaUsers(const USER users[], const DWORD numUsers) {
	HANDLE hFile;
	TCHAR buff_t[SMALL_TEXT];
	char buff_c[SMALL_TEXT];
	DWORD nbytes, res;

	DWORD dwCreationDisposition = FileExists(FILE_USERS) ? OPEN_EXISTING : CREATE_NEW;

	hFile = CreateFile(FILE_USERS, GENERIC_WRITE, 0, NULL, dwCreationDisposition, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		PrintErrorMsg(GetLastError(), _T("Erro em CreateFile"));
		return FALSE;
	}

	for (DWORD i = 0; i < numUsers; i++) {
		res = _stprintf_s(buff_t, SMALL_TEXT, _T("%s %s %.2lf\n"),
			users[i].nome, users[i].pass, users[i].carteira.saldo);

		if (res == -1) {
			_tprintf_s(_T("\nErro em _stprintf_s"));
			CloseHandle(hFile);
			return FALSE;
		}

		// nbytes vai ser usado para outra coisa agora

		nbytes = WideCharToMultiByte(CP_UTF8, WC_COMPOSITECHECK, buff_t, -1, buff_c, SMALL_TEXT, NULL, NULL);
		if (res == 0) {
			PrintErrorMsg(GetLastError(), _T("Erro em WideCharToMultiByte"));
			CloseHandle(hFile);
			return FALSE;
		}
		buff_c[nbytes / sizeof(char)] = '\0';

		if (!WriteFile(hFile, buff_c, (DWORD)strlen(buff_c), &nbytes, NULL)) {
			PrintErrorMsg(GetLastError(), _T("Erro em WriteFile"));
			CloseHandle(hFile);
			return FALSE;
		}

		//_tprintf_s(_T("\nEscrevi %d bytes (%d)"), nbytes, i + 1);

	}

	CloseHandle(hFile);

	return TRUE;
}

//|========================================================================|
//|===============================| Outras |===============================|
//|========================================================================|

DWORD getNCLIENTES() {
	TCHAR chave_completa[TAM] = _T("Software\\BolsaValores_SO2");
	HKEY chave;
	DWORD res;

	DWORD valueType;
	DWORD dataSize;
	DWORD value;

	DWORD queryResult;

	DWORD AUX;

	DWORD setResult;

	if (RegCreateKeyEx(HKEY_CURRENT_USER, chave_completa, 0, NULL, 
		REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &chave, &res) != ERROR_SUCCESS) {
		PrintErrorMsg(GetLastError(), _T("Erro no RegCreateKeyEx"));
		return -1;
	}

	queryResult = RegQueryValueEx(chave, _NCLIENTES, NULL, &valueType, NULL, &dataSize);

	if (queryResult == ERROR_SUCCESS) {
		if (valueType == REG_DWORD) {

			queryResult = RegQueryValueEx(chave, _NCLIENTES, NULL, &valueType, (LPBYTE)&value, &dataSize);

			if (queryResult == ERROR_SUCCESS) {

				RegCloseKey(chave);
				return value;

			} else {
				PrintErrorMsg(GetLastError(), _T("Erro no RegQueryValueEx"));
				RegCloseKey(chave);
				return -1;
			}

		} else {
			_tprintf_s(_T("\nTipo de dados invalido o Registry foi alterado: %d "), queryResult);
			queryResult = ERROR_FILE_NOT_FOUND;
		}
	}
	if (queryResult == ERROR_FILE_NOT_FOUND) {
		_tprintf_s(_T("\nO valor não existe. Criando..."));

		AUX = NCLIENTES;
		setResult = RegSetValueEx(chave, _NCLIENTES, 0, REG_DWORD, (LPBYTE)(&AUX), sizeof(AUX));

		if (setResult != ERROR_SUCCESS) {
			PrintErrorMsg(GetLastError(), _T("Erro no RegSetValueEx"));
			RegCloseKey(chave);
			return -1;
		}
	}

	RegCloseKey(chave);
	return NCLIENTES;
}

EMPRESA* AlocaEmpresas() {
	EMPRESA* empresas = malloc(sizeof(EMPRESA) * MAX_EMPRESAS);
	return empresas;
}

void InicializaEmpresas(EMPRESA* empresas) {
	for (DWORD i = 0; i < MAX_EMPRESAS; i++) {
		_tcscpy_s(empresas[i].nome, SMALL_TEXT, _T(""));
		empresas[i].numAcoes = 0;
		empresas[i].preco = 0.0;
	}
}

USER* AlocaUsers() {
	USER* users = malloc(sizeof(USER) * MAX_EMPRESAS);
	return users;
}

void InicializaUsers(USER* users) {
	for (DWORD i = 0; i < MAX_USERS; i++) {
		users[i].ligado = FALSE;
		_tcscpy_s(users[i].nome, SMALL_TEXT, _T(""));
		_tcscpy_s(users[i].pass, SMALL_TEXT, _T(""));
	}
}

int ComparaEmpresas(const void* a, const void* b) {
	const EMPRESA* empresa1 = (const EMPRESA*)a;
	const EMPRESA* empresa2 = (const EMPRESA*)b;

	if (empresa1->preco < empresa2->preco) return 1;
	if (empresa1->preco > empresa2->preco) return -1;
	return 0;
}

DWORD getHandlePipeLivre(TDATA_BOLSA* ptd) {
	HANDLE hPipes[NCLIENTES];

	EnterCriticalSection(ptd->pCs);
	CopyMemory(hPipes, ptd->hPipes, sizeof(hPipes));
	LeaveCriticalSection(ptd->pCs);

	for (DWORD i = 0; i < NCLIENTES; i++) {
		if (hPipes[i] == NULL) { return i; }
	}

	return -1;
}