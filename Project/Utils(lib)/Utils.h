#pragma once

#include <Windows.h>
#include <tchar.h>
#include <stdlib.h>


#include "Structs.h"

//|=========================================================================|
//|===============================| Funções |===============================|
//|=========================================================================|

// file existe: return TRUE 
// file nao existe: return FALSE  
BOOL FileExists(const LPCWSTR fileName);

// Recebe o codigo de erro, descodifica-o numa mensagem e mostra-a no ecra
// Opcionalmente recebe tambem uma mensagem introduzida pelo user
void PrintErrorMsg(
	const DWORD codigoErro,
	const TCHAR* msg);

// Mostra no ecra toda a informacao relativa a empresa
void PrintEmpresas(
	const EMPRESA empresas[],
	const DWORD numEmpresas);

// Mostra no ecra toda a informacao relativa aos users
void PrintUsers(
	const USER users[], 
	const DWORD numUsers);

// Verifica se str é um inteiro
BOOL IsInteger(const TCHAR* str);

// Verifica se str é um float
BOOL IsDouble(const TCHAR* str);

// Devolve uma string igual à passada por parâmetro mas toda em lowercase
TCHAR* ToLowerString(const TCHAR* s);

// APAGAR SE NAO FOR NECESSARIO
// Inicializa gerador de valores aleatorios
void InitRand();

// APAGAR SE NAO FOR NECESSARIO
// Gera um valor aleat�rio entre min e max
DWORD RandomValue(
	DWORD max,
	DWORD min);

// APAGAR SE NAO FOR NECESSARIO
// Preenche as estruturas TDATA com a informacao pretendida (example)
//DWORD PreencheThreadData(
//	TDATA arrayThreadData[],
//	const DWORD numThreads,
//	const TDATA example);

// APAGAR SE NAO FOR NECESSARIO
// Lanca as threads
//DWORD LancaThreads(
//	HANDLE arrayThreadHandler[],
//	TDATA arrayThreadData[],
//	DWORD arrayThreadID[],
//	LPTHREAD_START_ROUTINE funcaoThread,
//	const DWORD numThreads);

// APAGAR SE NAO FOR NECESSARIO
// Preenche a LookUpTable de gestao das threads
void PreencheLookupTable(
	DWORD* lookUpTable,
	const DWORD tam);

// APAGAR SE NAO FOR NECESSARIO
// Alteracao da lookuptable e do array de handles das threads
DWORD Alter_LookUpTable(
	DWORD* lookUpTable,
	HANDLE threadHandlerArray[],
	const DWORD position,
	const DWORD numThreads);

// APAGAR SE NAO FOR NECESSARIO
// Funcao auxiliar para dar print � LookUpTable
void Print_LookUpTable(
	const DWORD* lookUpTable,
	const DWORD tam);
