#pragma once

#include <Windows.h>
#include <tchar.h>
#include <stdlib.h>

#include "pch.h"
#include "Utils.h"

// estrutura de dados das threads
typedef struct {
	unsigned int* pSoma;
	unsigned int* pNumBloco;
	BOOL* pContinua;

	CRITICAL_SECTION* pCs;
	HANDLE hEv;
} TDATA;

// Preenche as estruturas TDATA com a informa��o pretendida (example)
DWORD PreencheThreadData(
	TDATA arrayThreadData[],
	const DWORD numThreads,
	const TDATA example);

// Lan�a as treads
DWORD LancaThreads(
	HANDLE* arrayThreadHandler,
	TDATA* arrayThreadData,
	DWORD* arrayThreadID,
	LPTHREAD_START_ROUTINE funcaoThread,
	const DWORD numThreads);

// Preenche a LookUpTable de gest�o das threads
void PreencheLookupTable(
	DWORD* lookUpTable,
	const DWORD tam);

// Altera��o da lookuptable e do array de handles das threads
DWORD Alter_LookUpTable(
	DWORD* lookUpTable,
	HANDLE* threadHandlerArray,
	const DWORD position,
	const DWORD numThreads);

// Fun��o auxiliar para dar print � LookUpTable
void Print_LookUpTable(
	const DWORD* lookUpTable,
	const DWORD tam);

// Recebe o codigo de erro e descodifica-o. Opcionalmente recebe tamb�m uma mensagem introduzida pelo user
void PrintError(
	const DWORD codigoErro,
	const TCHAR* msg);