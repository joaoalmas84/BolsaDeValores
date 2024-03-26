#pragma once

#include <Windows.h>
#include <tchar.h>
#include <stdlib.h>
#include <time.h>

#include "Utils.h"

// Estrutura de dados das threads
typedef struct {
	int whatever;
} TDATA;

// Inicializa gerador de valores aleatórios
void InitRand();

// Gera um valor aleatório entre min e max
DWORD RandomValue(
	DWORD max, 
	DWORD min);

// Verifica se o ficheiro com o nome fileName existe e preenche dwCreationDisposition com o valor correspondente
BOOL CheckFileExistence(
	const LPCWSTR fileName,
	DWORD* dwCreationDisposition);


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