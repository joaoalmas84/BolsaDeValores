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

void PrintUser(const USER user) {
	TCHAR buff[BIG_TEXT];
	TCHAR estado[SMALL_TEXT];

	if (user.ligado) {
		_tcscpy_s(estado, SMALL_TEXT, _T("ligado"));
	} else {
		_tcscpy_s(estado, SMALL_TEXT, _T("desligado"));
	}

	_sntprintf_s(buff, BIG_TEXT, BIG_TEXT, _T("\nNome: %s\nEstado: %s\nSaldo: %f\nN.� de empresas: %d\n"),
		user.nome, estado, user.carteira.saldo, user.carteira.numEmpresas);

	_tprintf_s(_T("\n---------------------------------"));
	_tprintf_s(_T("%s"), buff);
}

void PrintEmpresa(const EMPRESA empresa) {
	TCHAR buff[BIG_TEXT];

	_sntprintf_s(buff, BIG_TEXT, BIG_TEXT, _T("\nNome: %s\nN.� de a��es para venda: %d\nPre�o por a��o: %.2f\n"),
		empresa.nome, empresa.numAcoes, empresa.preco);

	_tprintf_s(_T("\n---------------------------------"));
	_tprintf_s(_T("%s"), buff);
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