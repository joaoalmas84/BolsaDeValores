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
		CopyMemory(sharedMemory, ptd->empresas, sizeof(EMPRESA)*(*ptd->numEmpresas));
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

	// Apagar depois
	_tprintf_s(_T("\nComando: "));
	_tprintf_s(_T("\nNome: '%s'"), comando.Nome);
	_tprintf_s(_T("\nNumArgs: %d"), comando.NumArgs);
	_tprintf_s(_T("\nIndex: %d"), comando.Index);
	_tprintf_s(_T("\nArgs: "));
	for (DWORD i = 0; i < comando.NumArgs; i++) {
		_tprintf_s(_T("'%s' "), comando.Args[i]);
	}
}

BOOL ADDC(const CMD comando, TDATA_BOLSA* threadData, TCHAR* mensagem) {
	DWORD numAcoes = _ttoi(comando.Args[2]);
	DOUBLE preco = _tcstod(comando.Args[3], NULL);
	DWORD numEmpresas;
	BOOL jaExiste = FALSE;

	WaitForSingleObject(threadData->hMutex, INFINITE);
	numEmpresas = *threadData->numEmpresas;
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
	(*threadData->numEmpresas)++;
	_tcscpy_s(threadData->empresas[(*threadData->numEmpresas)-1].nome, SMALL_TEXT, comando.Args[1]);
	threadData->empresas[(*threadData->numEmpresas)-1].numAcoes = numAcoes;
	threadData->empresas[(*threadData->numEmpresas)-1].preco = preco;
	ReleaseMutex(threadData->hMutex);

	SetEvent(threadData->hEvent_Board);
	ResetEvent(threadData->hEvent_Board);

	return TRUE;
}

void LISTC(TDATA_BOLSA* threadData) {
	WaitForSingleObject(threadData->hMutex, INFINITE);
	for (DWORD i = 0; i < *threadData->numEmpresas; i++) {
		PrintEmpresa(threadData->empresas[i]);
	}
	ReleaseMutex(threadData->hMutex);
}

void STOCK(const CMD comando, TDATA_BOLSA* threadData) {
	DOUBLE preco = _tcstod(comando.Args[2], NULL);

	for (DWORD i = 0; i < *threadData->numEmpresas; i++) {
		if (_tcscmp(threadData->empresas[i].nome, comando.Args[1]) == 0) {

			WaitForSingleObject(threadData->hMutex, INFINITE);
			threadData->empresas[i].preco = preco;
			qsort(threadData->empresas, *threadData->numEmpresas, sizeof(EMPRESA), compara_empresas);
			ReleaseMutex(threadData->hMutex);

			return;
		}
	}

	_tprintf(_T("O nome %s nao foi encontrado\n"), comando.Args[1]);
}

void USERS(TDATA_BOLSA* threadData) {
	WaitForSingleObject(threadData->hMutex, INFINITE);
	for (DWORD i = 0; i < *threadData->numUsers; i++) {
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
	char* pStr;

	hFile = CreateFile(FILE_EMPRESAS, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		*codigoErro = GetLastError();
		_tcscpy_s(errorMsg, BIG_TEXT, _T("Erro em CreateFile"));
		return FALSE;
	}

	hMap = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, BIG_TEXT * sizeof(char), NULL);
	if (hMap == NULL) {
		*codigoErro = GetLastError();
		_tcscpy_s(errorMsg, BIG_TEXT, _T("Erro em CreateFileMapping"));
		return FALSE;
	}

	pStr = (char*)MapViewOfFile(hMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, BIG_TEXT * sizeof(char));
	if (pStr == NULL) {
		*codigoErro = GetLastError();
		_tcscpy_s(errorMsg, BIG_TEXT, _T("Erro em MapViewOfFile"));
		return FALSE;
	}

	_tcscpy_s(buff, BIG_TEXT, _T(""));
	//_tprintf_s(_T("\n_tcslen(buff) = %d"), (int)_tcslen(buff));

	if (MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, pStr, BIG_TEXT, buff, BIG_TEXT) == 0) {
		*codigoErro = GetLastError();
		_tcscpy_s(errorMsg, BIG_TEXT, _T("Erro em MapViewOfFile"));
		return FALSE;
	}

	//printf_s("pStr: '%s'", pStr);
	
	_tprintf_s(_T("\nbuff: \n|%s|"), buff);
	//_tprintf_s(_T("\n_tcslen(buff) = %d"), (int)_tcslen(buff));
	buff[_tcslen(buff)-1] = _T('\0');

	/*if (!ProcessaEmpresasDoFicheiro(buff, empresas, numEmpresas)) {
		exit(0);
	}*/

	FlushViewOfFile(pStr, 0);

	UnmapViewOfFile(pStr);

	CloseHandle(hMap);

	CloseHandle(hFile);

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

int compara_empresas(const void* a, const void* b) {
	const EMPRESA* empresa1 = (const EMPRESA*)a;
	const EMPRESA* empresa2 = (const EMPRESA*)b;

	if (empresa1->preco < empresa2->preco) return 1;
	if (empresa1->preco > empresa2->preco) return -1;
	return 0;
}