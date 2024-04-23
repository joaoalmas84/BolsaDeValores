#include <Windows.h>
#include <tchar.h>
#include <stdlib.h>

#include "pch.h"
#include "Utils.h"

BOOL FileExists(const LPCWSTR fileName) {
	return GetFileAttributes(fileName) != INVALID_FILE_ATTRIBUTES;
}

void PrintErrorMsg(const DWORD codigoErro, const TCHAR* msg) {
	TCHAR erro[SMALL_TEXT];

	if (codigoErro == -1) {
		_tprintf_s(_T("\n[ERRO] %s"), msg);
		return;
	}

	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, codigoErro, LANG_USER_DEFAULT, erro, SMALL_TEXT, NULL);
	
	_tprintf_s(_T("\n[ERRO] %d - %s"), codigoErro, erro);

	if (msg != NULL && _tccmp(msg, _T("")) != 0) {
		_tprintf_s(_T("(%s)"), msg);
	}
}

void PrintEmpresas(const EMPRESA empresas[], const DWORD numEmpresas) {
	for (DWORD i = 0; i < numEmpresas; i++) {
		_tprintf_s(_T("\n--------------------------------+"));
		_tprintf_s(_T("%d\nNome: '%s'\nN.º de ações para venda: %d\nPreço por ação: %.2f\n"),
		i+1, empresas[i].nome, empresas[i].numAcoes, empresas[i].preco);
	}
}

void PrintUsers(const USER users[], const DWORD numUsers) {
	TCHAR estado[SMALL_TEXT];

	for (DWORD i = 0; i < numUsers; i++) {
		if (users[i].ligado) { _tcscpy_s(estado, SMALL_TEXT, _T("ligado")); }
		else { _tcscpy_s(estado, SMALL_TEXT, _T("desligado")); }
		
		_tprintf_s(_T("\n--------------------------------+"));
		_tprintf_s(_T("\nNome: %s\nPass: %s\nEstado: %s\nSaldo: %.2f\nN.º de empresas: %d\n"),
		users[i].nome, users[i].pass, estado, users[i].carteira.saldo, users[i].carteira.numEmpresas);
	}
}

BOOL IsInteger(const TCHAR* str) {
	TCHAR* endPtr;
	long value = _tcstol(str, &endPtr, 10);

	if (str == endPtr || *endPtr != _T('\0')) { return FALSE; }
	else { return TRUE; }
}

BOOL IsDouble(const TCHAR* str) {
	TCHAR* endPtr;
	double res;

	res = _tcstod(str, &endPtr);

	if (str == endPtr || *endPtr != '\0') { return FALSE; }
	else { return TRUE; }
}

TCHAR* ToLowerString(const TCHAR* s) {
	TCHAR* aux = malloc(sizeof(TCHAR) * (_tcslen(s)));
	int i;

	if (aux == NULL) { return NULL; }

	for (i = 0; s[i] != '\0'; ++i) {
		aux[i] = tolower(s[i]);
	}
	aux[i] = '\0';

	return aux;
}

void InitRand() {
	srand((unsigned)time(NULL));
}

DWORD RandomValue(DWORD max, DWORD min) {
	return rand() % (max - min + 1) + min;
}

DWORD PreencheThreadData(TDATA arrayThreadData[], const DWORD numThreads, const TDATA example) {
	DWORD i;
	for (i = 0; i < numThreads; i++) {
		arrayThreadData[i].whatever = 1;
	}
	return i;
}

DWORD LancaThreads(HANDLE arrayThreadHandler[], TDATA arrayThreadData[], DWORD arrayThreadID[], LPTHREAD_START_ROUTINE funcaoThread, const DWORD numThreads) {
	for (DWORD i = 0; i < numThreads; i++) {
		arrayThreadHandler[i] = CreateThread(NULL, 0, funcaoThread,
			(LPVOID)&arrayThreadData[i], CREATE_SUSPENDED, &arrayThreadID[i]);

		if (arrayThreadHandler[i] == NULL) { return GetLastError(); }

	}
	return 0;
}

void PreencheLookupTable(DWORD* lookUpTable, const DWORD tam) {
	for (DWORD i = 0; i < tam; i++) {
		lookUpTable[i] = i;
	}
}

DWORD Alter_LookUpTable(DWORD* lookUpTable, HANDLE threadHandlerArray[], const DWORD position, const DWORD numThreads) {
	if (position < numThreads - 1) {
		lookUpTable[position] = lookUpTable[numThreads - 1];
		threadHandlerArray[position] = threadHandlerArray[numThreads - 1];
	}
	return lookUpTable[position];
}

void Print_LookUpTable(const DWORD* lookUpTable, const DWORD tam) {
	_tprintf_s(_T("\n\nLookUpTable: |"));
	for (DWORD i = 0; i < tam; i++) {
		_tprintf_s(_T(" %d |"), lookUpTable[i]);
	}
	_tprintf_s(_T("\t"));
}