#pragma once

#include <Windows.h>
#include <tchar.h>
#include <stdlib.h>
#include <time.h>

#include "Utils.h"

#define BIG_TEXT 500	// Tamanho de buffer para textos
#define SMALL_TEXT 100	// Tamanho de buffer para nomes

// Macros
#define _NCLIENTES _T("NCLIENTES")			// nome da RegKey NCLIENTES
#define FILE_EMPRESAS _T("empresas.txt")	// nome do ficheiro de emprecas 
#define FILE_USERS _T("users.txt")			// nome do ficheiro de users 
#define SHARED_MEMORY _T("SHARED_MEMORY")	// nome da memoria partilhada
#define SHM_EVENT _T("SHM_EVENT")			// nome do evento que avisa a board
#define SHM_MUTEX _T("SHM_MUTEX")			// nome do mutex utilizado pela bolsa e pela board
#define SEM_BOLSA _T("SEM_BOLSA")			// nome do semaforo que impede que haja mais que um processo bolsa em simultaneo

// Valores Constantes
#define NCLIENTES 5				// valor default para a RegKey NCLIENTES 
#define MAX_EMPRESAS 30			// numero maximo de empresas
#define MAX_USERS 20			// numero maximo de users
#define MAX_EMPRESAS_TO_SHM 10	// numero maximo de empresas a que passar para Shared Memory

//|============================================================================|
//|===============================| Estruturas |===============================|
//|============================================================================|

// Apagar se nao for necessario
typedef struct {
	int whatever;
} TDATA;

// Estrutura que representa uma Empresa
typedef struct {
	TCHAR nome[SMALL_TEXT];
	DWORD numAcoes;
	DOUBLE preco;
} EMPRESA;

// Estrutura que representa a carteira do User
typedef struct {
	TCHAR empresas[MAX_EMPRESAS][SMALL_TEXT];	// array com os nomes empresas das quais este User possui acoes
	DWORD acoes[MAX_EMPRESAS];					// acoes que possui de cada empresa
	DWORD numEmpresas;							// numero de empresas das quais este User possui acoes
	DOUBLE saldo;
} CARTEIRA;

// Estrutura que representa o User
typedef struct {
	TCHAR nome[SMALL_TEXT];
	TCHAR pass[SMALL_TEXT];
	CARTEIRA carteira;
	BOOL ligado;
} USER;

// SharedMemory entre Bolsa e Board
typedef struct {
	EMPRESA empresas[MAX_EMPRESAS_TO_SHM];
} SHM;

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

// Mostra no ecra toda a informacao relativa ao user
void PrintUser(const USER user);

// Mostra no ecra toda a informacao relativa a empresa
void PrintEmpresa(const EMPRESA empresa);

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
DWORD PreencheThreadData(
	TDATA arrayThreadData[],
	const DWORD numThreads,
	const TDATA example);

// APAGAR SE NAO FOR NECESSARIO
// Lanca as threads
DWORD LancaThreads(
	HANDLE arrayThreadHandler[],
	TDATA arrayThreadData[],
	DWORD arrayThreadID[],
	LPTHREAD_START_ROUTINE funcaoThread,
	const DWORD numThreads);

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
