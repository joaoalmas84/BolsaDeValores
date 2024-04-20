#include <Windows.h>
#include <tchar.h>
#include <stdio.h>

#include "Bolsa.h"
#include "Commands.h"
#include "Utils.h"

//|=========================================================================|
//|===============================| Threads |===============================|
//|=========================================================================|

DWORD WINAPI ThreadBoard(LPVOID data) {
	TDATA_BOLSA* ptd = (TDATA_BOLSA*)data;

	BOOL continua = TRUE;

	HANDLE hMap, hMutex, hEvent; // Evento que avisa a board para ler
	SHM* sharedMemory;

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
		WaitForSingleObject(ptd->hEvent_Board, INFINITE);

		WaitForSingleObject(ptd->hMutex, INFINITE);
		continua = ptd->continua;
		ReleaseMutex(ptd->hMutex);

		if (!continua) { break; }

		WaitForSingleObject(ptd->hMutex, INFINITE);
		ZeroMemory(sharedMemory, sizeof(sharedMemory));
		CopyMemory(sharedMemory->empresas, ptd->empresas, sizeof(EMPRESA)*ptd->numEmpresas);
		ReleaseMutex(ptd->hMutex);

		SetEvent(hEvent);
		ResetEvent(hEvent);
	}

	FlushViewOfFile(sharedMemory, 0);

	UnmapViewOfFile(sharedMemory);

	CloseHandle(hEvent);

	CloseHandle(hMutex);

	CloseHandle(hMap);

	ExitThread(0);
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

	switch (comando.Index) {
		case 0:
			if (!ADDC(comando, threadData, mensagem)) {
				_tprintf_s(_T("%s"), mensagem);
			}
			break;
		case 1:
			LISTC(threadData);
			break;
		case 2:
			STOCK(comando, threadData);
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

	WaitForSingleObject(threadData->hMutex, INFINITE);
	numEmpresas = (threadData->numEmpresas);
	ReleaseMutex(threadData->hMutex);

	if (numEmpresas >= MAX_EMPRESAS) {
		_tcscpy_s(mensagem, SMALL_TEXT, _T("O número máximo de empresas já foi atingido!\n"));
		return FALSE;
	}

	WaitForSingleObject(threadData->hMutex, INFINITE);
	for (DWORD i = 0; i < numEmpresas; i++) {
		if (_tcscmp(ToLowerString(threadData->empresas[i].nome), ToLowerString(comando.Args[1])) == 0) {
			jaExiste = TRUE;
			break;
		}
	}
	ReleaseMutex(threadData->hMutex);

	if (jaExiste) { 
		_tcscpy_s(mensagem, SMALL_TEXT, _T("Já existe uma empresa com esse nome\n"));
		return FALSE; 
	}

	WaitForSingleObject(threadData->hMutex, INFINITE);
	threadData->empresas[numEmpresas].numAcoes = numAcoes;
	threadData->empresas[numEmpresas].preco = preco;
	_tcscpy_s(threadData->empresas[numEmpresas].nome, SMALL_TEXT, comando.Args[1]);
	threadData->numEmpresas++;
	ReleaseMutex(threadData->hMutex);

	SetEvent(threadData->hEvent_Board);
	ResetEvent(threadData->hEvent_Board);

	return TRUE;
}

void LISTC(TDATA_BOLSA* threadData) {
	WaitForSingleObject(threadData->hMutex, INFINITE);
	for (DWORD i = 0; i < threadData->numEmpresas; i++) {
		PrintEmpresa(threadData->empresas[i]);
	}
	ReleaseMutex(threadData->hMutex);
}

void STOCK(const CMD comando, TDATA_BOLSA* threadData) {
	DOUBLE preco = _tcstod(comando.Args[2], NULL);

	for (DWORD i = 0; i < threadData->numEmpresas; i++) {
		if (_tcscmp(threadData->empresas[i].nome, comando.Args[1]) == 0) {

			WaitForSingleObject(threadData->hMutex, INFINITE);
			threadData->empresas[i].preco = preco;
			qsort(threadData->empresas, threadData->numEmpresas, sizeof(EMPRESA), compara_empresas);
			ReleaseMutex(threadData->hMutex);

			return;
		}
	}

	_tprintf(_T("O nome %s nao foi encontrado\n"), comando.Args[1]);
}

void USERS(TDATA_BOLSA* threadData) {
	WaitForSingleObject(threadData->hMutex, INFINITE);
	for (DWORD i = 0; i < threadData->numUsers; i++) {
		PrintUser(threadData->users[i]);
	}
	ReleaseMutex(threadData->hMutex);
}

void PAUSE() {

}

void CLOSE(TDATA_BOLSA* threadData) {
	WaitForSingleObject(threadData->hMutex, INFINITE);
	threadData->continua = FALSE;
	ReleaseMutex(threadData->hMutex);
	SetEvent(threadData->hEvent_Board);
}

//|====================================================================================|
//|===============================| Ficheiros de Dados |===============================|
//|====================================================================================|

BOOL CarregaEmpresas(EMPRESA empresas[], DWORD* numEmpresas, TCHAR* errorMsg, DWORD* codigoErro) {
	HANDLE hFile, hMap;
	TCHAR buff[BIG_TEXT];

	hFile = CreateFile(FILE_EMPRESAS, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		*codigoErro = GetLastError();
		_tcscpy_s(errorMsg, BIG_TEXT, _T("Erro em CreateFile"));
		return FALSE;
	}

	if (!ReadFile(hFile)) {

	}


	exit(0);
}

BOOL ProcessaEmpresasDoFicheiro(const TCHAR* txt, EMPRESA empresas[], DWORD* numEmpresas) {
	TCHAR* txtCopy;
	TCHAR* nextToken = NULL;

	TCHAR buff[SMALL_TEXT];

	txtCopy = _tcsdup(txt);

	_tcscpy_s(buff, SMALL_TEXT, _tcstok_s(txtCopy, _T("\n"), &nextToken));
	
	_tprintf_s(_T("buff: '%s'"), buff);

	free(txtCopy);

	exit(0);
}

BOOL SalvaEmpresas(const EMPRESA empresas[], DWORD numEmpresas, TCHAR* errorMsg, DWORD* codigoErro) {
	return FALSE;
}

BOOL CarregaUsers(USER users[], DWORD* numUsers, TCHAR* errorMsg, DWORD* codigoErro) {
	return FALSE;
}

BOOL SalvaUsers(const USER users[], DWORD numUsers, TCHAR* errorMsg) {
	return FALSE;
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

int compara_empresas(const void* a, const void* b) {
	const EMPRESA* empresa1 = (const EMPRESA*)a;
	const EMPRESA* empresa2 = (const EMPRESA*)b;

	if (empresa1->preco < empresa2->preco) return 1;
	if (empresa1->preco > empresa2->preco) return -1;
	return 0;
}