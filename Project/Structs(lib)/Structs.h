#pragma once

#include <Windows.h>
#include <tchar.h>
#include <stdlib.h>

// Macros
#define _NCLIENTES _T("NCLIENTES")			// nome da RegKey NCLIENTES
#define FILE_EMPRESAS _T("empresas.txt")	// nome do ficheiro de emprecas 
#define FILE_USERS _T("users.txt")			// nome do ficheiro de users 
#define START_EVENT _T("START_EVENT")		// nome do evento que obriga a Board a esperar pela Bolsa 
#define SHARED_MEMORY _T("SHARED_MEMORY")	// nome da memoria partilhada
#define SHM_EVENT _T("SHM_EVENT")			// nome do evento que avisa a board
#define SHM_MUTEX _T("SHM_MUTEX")			// nome do mutex utilizado pela bolsa e pela board
#define SEM_BOLSA _T("SEM_BOLSA")			// nome do semaforo que impede que haja mais que um processo bolsa em simultaneo
#define SEM_CLIENTES _T("SEM_CLIENTES")		// nome do semaforo que regula o N.º de processos cliente em execução
#define PIPE_NAME _T("\\\\.\\pipe\\comunicacao")	// nome do named pipe utilizado na comunicacao Cliente <-> Blosa

// Valores Constantes
#define BIG_TEXT 500			// Tamanho de buffer para textos
#define SMALL_TEXT 100			// Tamanho de buffer para frases
#define NCLIENTES 5				// valor default para a RegKey NCLIENTES 
#define MAX_EMPRESAS 30			// numero maximo de empresas
#define MAX_USERS 20			// numero maximo de users
#define MAX_EMPRESAS_TO_SHM 10	// numero maximo de empresas a que passar para Shared Memory
#define MAX_POSSE_EMPRESAS 5	// um utilizador nao pode ter acoes em mais do que 5 empresas diferentes

// Comunicacao NamedPipes:
// Cliente -> Board (Pedido)
#define P_LOGIN 1	// <- maior
#define P_COMPRA 2
#define P_VENDA 3
#define P_LISTA 4

// Board -> Cliente (Resposta)
#define R_LOGIN 1	// <- maior
#define R_COMPRA 2
#define R_VENDA 3
#define R_LISTA 4

//|============================================================================|
//|===============================| Estruturas |===============================|
//|============================================================================|

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