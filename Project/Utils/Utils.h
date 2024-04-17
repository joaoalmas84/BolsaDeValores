#pragma once

#include <Windows.h>
#include <tchar.h>
#include <stdlib.h>
#include <time.h>

#include "Utils.h"

#define BIG_TEXT 200
#define SMALL_TEXT 20

// Macros
#define _NCLIENTES _T("NCLIENTES")			
#define FILE_EMPRESAS _T("empresas.txt")	// nome do ficheiro de emprecas 
#define USERS_FILE _T("users.txt")			// nome do ficheiro de users 
#define SHARED_MEMORY _T("SHARED_MEMORY")	// nome da memoria partilhada
#define SHM_EVENT _T("SHM_EVENT")			// nome do evento que avisa a board
#define SHM_MUTEX _T("SHM_MUTEX")			// nome do mutex utilizado pela bolsa e pela board
#define SEMAPHORE _T("SEMAPHORE")

// Valores Constantes
#define NCLIENTES 5				// valor default para NCLIENTES 
#define MAX_EMPRESAS 30			// numero maximo de empresas
#define MAX_USERS 20			// numero maximo de users
#define MAX_EMPRESAS_TO_SHM 10	// numero maximo de empresas a que passar para Shared Memory

//|============================================================================|
//|===============================| Estruturas |===============================|
//|============================================================================|

typedef struct {
	int whatever;
} TDATA;

// Estrutura que representa uma Empresa
typedef struct {
	TCHAR nome[SMALL_TEXT];
	DWORD numAcoes;
	DOUBLE preco;
} EMPRESA;

// Estrutura que representa a carteira do Cliente
typedef struct {
	TCHAR empresas[MAX_EMPRESAS][SMALL_TEXT];	// array com os nomes empresas das quais este User possui acoes
	DWORD acoes[MAX_EMPRESAS];					// acoes que possui de cada empresa
	DWORD numEmpresas;							// numero de empresas das quais este User possui acoes
	DOUBLE saldo;
} CARTEIRA;

// Estrutura que representa o Cliente
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

// Inicializa gerador de valores aleat�rios
void InitRand();

// Gera um valor aleat�rio entre min e max
DWORD RandomValue(
	DWORD max,
	DWORD min);

// Verifica se o ficheiro com o nome fileName existe e preenche dwCreationDisposition com o valor correspondente
BOOL CheckFileExistence(
	const LPCWSTR fileName,
	DWORD* dwCreationDisposition);


// Preenche as estruturas TDATA com a informa��o pretendida (example)
DWORD PreencheThreadData(
	TDATA arrayThreadData[],
	const DWORD numThreads,
	const TDATA example);

// Lanca as threads
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

void PrintUser(const USER user);

void PrintEmpresa(const EMPRESA empresa);