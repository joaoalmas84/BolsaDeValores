#pragma once

#include <Windows.h>
#include <tchar.h>
#include <stdlib.h>
#include <time.h>

#include "Utils.h"

// Macros
#define _NCLIENTES _T("NCLIENTES")			// nome da RegKey NCLIENTES
#define FILE_EMPRESAS _T("empresas.txt")	// nome do ficheiro de emprecas 
#define FILE_USERS _T("users.txt")			// nome do ficheiro de users 
#define START_EVENT _T("START_EVENT")		// nome do evento que obriga a Board a esperar pela Bolsa 
#define SHARED_MEMORY _T("SHARED_MEMORY")	// nome da memoria partilhada
#define SHM_EVENT _T("SHM_EVENT")			// nome do evento que avisa a board
#define SHM_MUTEX _T("SHM_MUTEX")			// nome do mutex utilizado pela bolsa e pela board
#define SEM_BOLSA _T("SEM_BOLSA")			// nome do semaforo que impede que haja mais que um processo bolsa em simultaneo

// Valores Constantes
#define BIG_TEXT 500			// Tamanho de buffer para textos
#define SMALL_TEXT 100			// Tamanho de buffer para frases
#define NCLIENTES 5				// valor default para a RegKey NCLIENTES 
#define MAX_EMPRESAS 30			// numero maximo de empresas
#define MAX_USERS 20			// numero maximo de users
#define MAX_EMPRESAS_TO_SHM 10	// numero maximo de empresas a que passar para Shared Memory
#define MAX_POSSE_EMPRESAS 5	// um utilizador nao pode ter acoes em mais do que 5 empresas diferentes

// Comunicacao NamedPipes:
// Cliente -> Board 
#define P_LOGIN 1
#define P_COMPRA 2
#define P_VENDA 3
#define P_LISTA 4

// Board -> Cliente 
#define R_LOGIN 1
#define R_COMPRA 2
#define R_VENDA 3
#define R_LISTA 4

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

typedef struct {
	TCHAR empresasNomes[MAX_POSSE_EMPRESAS][SMALL_TEXT];// array com os nomes empresas das quais este User possui acoes
	DWORD acoes[MAX_POSSE_EMPRESAS];					// acoes que possui de cada empresa
} POSSE_EMPRESA;

// Estrutura que representa a carteira do User
typedef struct {
	POSSE_EMPRESA posse_empresas[MAX_POSSE_EMPRESAS];	// representacao simplista das empresas dentro da carteira
	DWORD numEmpresas;									// numero de empresas das quais este User possui acoes
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

//|==============================================================================================|
//|===============================| Comunicacao Cliente -> Board |===============================|
//|==============================================================================================|

// Pedido de Login
typedef struct {
	TCHAR name[SMALL_TEXT];
	TCHAR pass[SMALL_TEXT];
} _LOGIN;

typedef struct {
	DWORD codigo; // <- P_LOGIN
	_LOGIN login;
} PEDIDO_LOGIN;

// Pedido de Compra
typedef struct {
	TCHAR nomeEmpresa[SMALL_TEXT];
	DWORD numAcoes;
} COMPRA;

typedef struct {
	DWORD codigo; // <- P_COMPRA
	COMPRA compra;
} PEDIDO_COMPRA;

// Pedido de Venda
typedef struct {
	TCHAR nomeEmpresa[SMALL_TEXT];
	DWORD numAcoes;
} VENDA;

typedef struct {
	DWORD codigo; // <- P_VENDA
	VENDA venda;
} PEDIDO_VENDA;

// Pedido de Lista
// basta enviar codigo = P_LISTA

//|==============================================================================================|
//|===============================| Comunicacao Board -> Cliente |===============================|
//|==============================================================================================|

// Resposta Login
typedef struct {
	DWORD codigo; // <- R_LOGIN
	BOOL resultado;
	USER dados;
} RESPOSTA_LOGIN;

// Resposta Compra
typedef struct {
	DWORD codigo; // <- R_COMPRA
	BOOL resultado;
} RESPOSTA_COMPRA;

// Resposta Venda
typedef struct {
	DWORD codigo; // <- R_VENDA
	BOOL resultado;
} RESPOSTA_VENDA;

// Resposta Lista
typedef struct {
	DWORD codigo; // <- R_LISTA
	TCHAR lista[BIG_TEXT];
} RESPOSTA_LISTA;

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
void PrintEmpresas(const EMPRESA empresas[], const DWORD numEmpresas);

// Mostra no ecra toda a informacao relativa aos users
void PrintUsers(const USER users[], const DWORD numUsers);

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
