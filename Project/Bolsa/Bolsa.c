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

	HANDLE hPipe = NULL;


	hPipe = CreateNamedPipe(PIPE_NAME, PIPE_ACCESS_DUPLEX, 
		PIPE_WAIT | PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE, 
		ptd->nClientes, sizeof(PEDIDO_LOGIN), sizeof(RESPOSTA_LOGIN), 1000, NULL);



	ExitThread(6);
}

//|==========================================================================|
//|===============================| Comandos |===============================|
//|==========================================================================|

void ExecutaComando(const CMD comando, TDATA_BOLSA* threadData) {
	TCHAR mensagem[SMALL_TEXT];

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

	switch (comando.Index) {
		case 0:
			if (!ADDC(comando, threadData, mensagem)) { _tprintf_s(_T("\n%s"), mensagem); }
			break;
		case 1:
			LISTC(threadData);
			break;
		case 2:
			if (!STOCK(comando, threadData, mensagem)) { _tprintf_s(_T("\n%s"), mensagem); }
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
	}
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
	EnterCriticalSection(threadData->pCs);
	*threadData->continua = FALSE;
	LeaveCriticalSection(threadData->pCs);
	SetEvent(threadData->hEvent_Board);
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