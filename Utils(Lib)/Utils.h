#pragma once

#include <Windows.h>
#include <tchar.h>
#include <stdlib.h>
#include <time.h>

#include "Utils.h"
#define TAM_TEXT 200
//#define NOME_FILE_MEMORY _T("DATAMEMORY.bin") // teste
#define NUMERO_INICIAL_DE_USERES_EMPRESAS 5 // nao sei ja para que serve
#define NUMERO_MAX_EMPRESAS_PASSAR_POR_MEMORIA_VIRTUAL 10
#define NOME_DO_FILE_MEMORIA_VIRTUAL _T("memoriaVirtual") // nome do ficheiro virtual;
#define NOME_DO_EVENTO_PARA_AVISAR_BOARD _T("eventoParaBoard") // nome do evento para avisar board teste;

// Estrutura de dados das threads
typedef struct {
	int whatever;
} TDATA;

typedef struct {
	TCHAR nome[TAM_TEXT];
	DWORD numDeAcao;
	DOUBLE preco;
}EMPRESA;

typedef struct {
	DWORD numDeEmpresasExistentes;
	EMPRESA empresas[NUMERO_MAX_EMPRESAS_PASSAR_POR_MEMORIA_VIRTUAL];
	BOOL continuar;
}STRUCT_MEMORIA_VIRTUAL;

typedef struct {
	TCHAR nome[TAM_TEXT];
	TCHAR pass[TAM_TEXT];
	DOUBLE saldo;
	BOOL ligado;
}USERES;

typedef struct {
	USERES user;
	TCHAR nomeEmpresas[NUMERO_INICIAL_DE_USERES_EMPRESAS][TAM_TEXT];
	DWORD numDeAcoes[NUMERO_INICIAL_DE_USERES_EMPRESAS];
	DWORD numdeEmpresasNaCarteira;
}CARTEIRA_DE_ACOES;

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