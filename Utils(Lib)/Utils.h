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

// Preenche as estruturas TDATA com a informação pretendida (example)
DWORD PreencheThreadData(
	TDATA arrayThreadData[],
	const DWORD numThreads,
	const TDATA example);

// Lança as treads
DWORD LancaThreads(
	HANDLE* arrayThreadHandler,
	TDATA* arrayThreadData,
	DWORD* arrayThreadID,
	LPTHREAD_START_ROUTINE funcaoThread,
	const DWORD numThreads);

// Preenche a LookUpTable de gestão das threads
void PreencheLookupTable(
	DWORD* lookUpTable,
	const DWORD tam);

// Alteração da lookuptable e do array de handles das threads
DWORD Alter_LookUpTable(
	DWORD* lookUpTable,
	HANDLE* threadHandlerArray,
	const DWORD position,
	const DWORD numThreads);

// Função auxiliar para dar print à LookUpTable
void Print_LookUpTable(
	const DWORD* lookUpTable,
	const DWORD tam);

// Recebe o codigo de erro e descodifica-o. Opcionalmente recebe também uma mensagem introduzida pelo user
void PrintError(
	const DWORD codigoErro,
	const TCHAR* msg);