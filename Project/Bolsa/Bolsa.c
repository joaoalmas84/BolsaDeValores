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
	DWORD numEmpresas;
	EMPRESA empresas[MAX_EMPRESAS];

	HANDLE hMap, hMutex, hEvent; // Evento que avisa a board para ler
	SHM* sharedMemory;

	hMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(SHM), SHARED_MEMORY);
	if (hMap == NULL) {
		PrintError(GetLastError(), _T("Erro em CreateFileMapping"));
		return 0;
	}

	sharedMemory = (SHM*)MapViewOfFile(hMap, FILE_MAP_WRITE, 0, 0, 0);
	if (sharedMemory == NULL) {
		PrintError(GetLastError(), _T("Erro em MapViewOfFile"));
		CloseHandle(hMap);
		return 0;
	}

	hMutex = CreateMutex(NULL, FALSE, SHM_MUTEX);
	if (hMutex == NULL) {
		PrintError(GetLastError(), _T("Erro em CreateMutex"));
		CloseHandle(hMap);
		return 0;
	}

	hEvent = CreateEvent(NULL, TRUE, FALSE, SHM_EVENT);
	if (hEvent == NULL) {
		PrintError(GetLastError(), _T("Erro em CreateEvent"));
		return 1;
	}

	while (1) {
		WaitForSingleObject(ptd->hEvent_Board, INFINITE);

		WaitForSingleObject(ptd->hMutex, INFINITE);
		continua = ptd->continua;
		numEmpresas = *ptd->numEmpresas;
		ReleaseMutex(ptd->hMutex);

		if (!continua) { break; }

		WaitForSingleObject(ptd->hMutex, INFINITE);
		for (DWORD i = 0; i < numEmpresas; i++) {
			empresas[i] = ptd->empresas[i];
		}
		ReleaseMutex(ptd->hMutex);

		ZeroMemory(sharedMemory, sizeof(sharedMemory));
		CopyMemory(sharedMemory, empresas, sizeof(EMPRESA)*numEmpresas);

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

	switch (comando.Index) {
		case 0:
			ADDC(comando, threadData);
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

void ADDC(const CMD comando, TDATA_BOLSA* threadData) {
	// passar para os numero e nao str
	DWORD numAcoes = _ttoi(comando.Args[2]);
	DOUBLE preco = _tcstod(comando.Args[3], NULL);

	DWORD numEmpresas;
	EMPRESA empresas[MAX_EMPRESAS];

	_tprintf_s(_T("\nSkirt"));

	WaitForSingleObject(threadData->hMutex, INFINITE);
	numEmpresas = *threadData->numEmpresas;
	for (DWORD i = 0; i < numEmpresas; i++) {
		empresas[i] = threadData->empresas[i];
	}
	ReleaseMutex(threadData->hMutex);

	if (numEmpresas >= MAX_EMPRESAS) {
		_tprintf(_T("O número máximo de empresas já foi atingido!\n"));
		return;
	}

	// ver se ja existe 
	for (DWORD i = 0; i < numEmpresas; i++) {
		if (_tcscmp(empresas[i].nome, comando.Args[1]) == 0) {
			_tprintf(_T("Empresa ja existe!\n"));
			return;
		}
	}

	// atualizar dados
	_tcscpy_s(empresas[numEmpresas].nome, SMALL_TEXT, comando.Args[1]);
	empresas[numEmpresas].numAcoes = numAcoes;
	empresas[numEmpresas].preco = preco;
	numEmpresas++;

	WaitForSingleObject(threadData->hMutex, INFINITE);
	*threadData->numEmpresas = numEmpresas;
	for (DWORD i = 0; i < numEmpresas; i++) {
		threadData->empresas[i] = empresas[i];
	}
	ReleaseMutex(threadData->hMutex);

	SetEvent(threadData->hEvent_Board);
	ResetEvent(threadData->hEvent_Board);
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
	for (DWORD i = 0; i < *threadData->numClients; i++) {
		PrintUser(threadData->clients[i]);
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

void LerEmpresasDoArquivo(EMPRESA empresas[], DWORD* numEmpresas) {
	FILE* arquivo;
	wchar_t linha[BIG_TEXT * 3]; // Tamanho m�ximo de uma linha no arquivo
	wchar_t nome[SMALL_TEXT];
	DWORD numAcoes;
	DOUBLE preco;

	if (_wfopen_s(&arquivo, FILE_EMPRESAS, _T("r, ccs=UTF-8")) != 0) {
		_tprintf(_T("Erro ao abrir o arquivo.\n"));
		return;
	}

	while (fgetws(linha, sizeof(linha), arquivo) != NULL) {
		if (*numEmpresas >= MAX_EMPRESAS) {
			_tprintf(_T("Número maximo de empresas atingido.\n"));
			break;
		}

		if (_stscanf_s(linha, _T("%s %d %lf"), nome, BIG_TEXT - 1, &numAcoes, &preco) == 3) {
			nome[_tcsclen(nome)] = '\0';
			_tcscpy_s(empresas[*numEmpresas].nome, SMALL_TEXT, nome);
			empresas[*numEmpresas].numAcoes = numAcoes;
			empresas[*numEmpresas].preco = preco;
			(*numEmpresas)++;

		} else { _tprintf(_T("Erro ao ler a linha.\n")); }
	}

	qsort(empresas, *numEmpresas, sizeof(EMPRESA), compara_empresas);

	fclose(arquivo);
}

void SalvarEmpresasNoArquivo(const EMPRESA empresas[], DWORD numEmpresas) {
	FILE* arquivo;

	if (_wfopen_s(&arquivo, FILE_EMPRESAS, _T("w, ccs=UTF-8")) != 0) {
		_tprintf_s(_T("Erro ao abrir o arquivo para escrita.\n"));
		return;
	}

	for (DWORD i = 0; i < numEmpresas; i++) {
		fwprintf(arquivo, _T("%s %d %.2f\n"), empresas[i].nome, empresas[i].numAcoes, empresas[i].preco);
	}

	fclose(arquivo);
}

void LerUsersDoArquivo(USER users[], DWORD* numUsers) {
	FILE* arquivo;
	TCHAR linha[BIG_TEXT * 3];
	TCHAR nome[SMALL_TEXT];
	TCHAR pass[SMALL_TEXT];
	DOUBLE saldo;

	if (_wfopen_s(&arquivo, USERS_FILE, _T("r, ccs=UTF-8")) != 0) {
		_tprintf(_T("Erro ao abrir o arquivo.\n"));
		return;
	}

	while (fgetws(linha, sizeof(linha), arquivo) != NULL) {
		if (*numUsers >= MAX_USERS) {
			_tprintf_s(_T("Número maximo de useres atingido.\n"));
			break;
		}

		if (_stscanf_s(linha, _T("%ls %ls %lf"), nome, SMALL_TEXT, pass, SMALL_TEXT, &saldo) == 3) {
			
			_tcscpy_s(users[*numUsers].nome, SMALL_TEXT, nome);
			nome[_tcslen(nome)] = _T('\0');

			_tcscpy_s(users[*numUsers].nome, SMALL_TEXT, pass);
			pass[_tcslen(pass)] = _T('\0');


			users[*numUsers].carteira.saldo = saldo;
			users[*numUsers].carteira.numEmpresas = 0;
			users[*numUsers].ligado = FALSE;
			(*numUsers)++;
		} else {
			_tprintf_s(_T("Erro ao ler a linha.\n"));
		}
	}

	fclose(arquivo);
}

void SalvarUsersNoArquivo(const USER users[], DWORD numUsers) {
	FILE* arquivo;

	if (_wfopen_s(&arquivo, USERS_FILE, _T("w, ccs=UTF-8")) != 0) {
		_tprintf_s(_T("Erro ao abrir o arquivo para escrita.\n"));
		return;
	}

	for (DWORD i = 0; i < numUsers; i++) {
		fwprintf_s(arquivo, _T("%ls %ls %.2lf\n"), 
			users[i].nome, users[i].pass, users[i].carteira.saldo);
	}

	fclose(arquivo);
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
		PrintError(GetLastError(), _T("Erro no RegCreateKeyEx"));
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
				PrintError(GetLastError(), _T("Erro no RegQueryValueEx"));
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
			PrintError(GetLastError(), _T("Erro no RegSetValueEx"));
			RegCloseKey(chave);
			return -1;
		}
	}

	RegCloseKey(chave);
	return NCLIENTES;
}

void InitializeEmpresas(EMPRESA empresas[]) {
	for (DWORD i = 0; i < MAX_EMPRESAS; i++) {
		_tcscpy_s(empresas[i].nome, SMALL_TEXT, _T(""));
		empresas[i].numAcoes = 0;
		empresas[i].preco = 0.0;
	}
}

void InitializeUsers(USER users[]) {
	for (DWORD i = 0; i < MAX_USERS; i++) {
		for (DWORD j = 0; j < MAX_EMPRESAS; j++) {
			users[i].carteira.acoes[j] = 0;
			_tcscpy_s(users[i].carteira.empresas[j], SMALL_TEXT, _T(""));
			users[i].carteira.numEmpresas = 0;
			users[i].carteira.saldo = 0.0;
		}
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