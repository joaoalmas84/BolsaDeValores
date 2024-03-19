#include <Windows.h>
#include <tchar.h>
#include <stdlib.h>

#include "pch.h"
#include "Utils.h"

DWORD PreencheThreadData(TDATA arrayThreadData[], const DWORD numThreads, const TDATA example) {
	DWORD i;
	for (i = 0; i < numThreads; i++) {
		arrayThreadData[i].pSoma = example.pSoma;
		arrayThreadData[i].pNumBloco = example.pNumBloco;
		arrayThreadData[i].pContinua = example.pContinua;
		arrayThreadData[i].pCs = example.pCs;
		arrayThreadData[i].hEv = example.hEv;
	}
	return i;
}

DWORD LancaThreads(HANDLE* arrayThreadHandler, TDATA* arrayThreadData, DWORD* arrayThreadID, LPTHREAD_START_ROUTINE funcaoThread, const DWORD numThreads) {
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

DWORD Alter_LookUpTable(DWORD* lookUpTable, HANDLE* threadHandlerArray, const DWORD position, const DWORD numThreads) {
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

void PrintError(const DWORD codigoErro, const TCHAR* msg) {
	TCHAR erro[200];

	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, codigoErro, LANG_USER_DEFAULT, erro, 200, NULL);
	_tprintf_s(_T("\n[ERRO] %d-%s\n"), codigoErro, erro);

	if (msg != NULL && _tccmp(msg, _T("")) != 0) {
		_tprintf_s(_T("\n(%s)"), msg);
	}
}