#pragma once

#include <Windows.h>
#include <tchar.h>
#include <stdlib.h>
#include <string.h>

#include "Commands.h"
#include "Utils.h"
#include "Structs.h"

#include "Bolsa.h"

// Thread Data - Processo Bolsa
typedef struct {
	BOOL* continua;		// Ponteiro para a flag continua localizada na funcao main 

	EMPRESA* empresas;	// Ponteiro para o array de empresas localizado na funcao main
	DWORD* numEmpresas;	// Ponteiro para o numero de empresas registadas localizado na funcao main
	
	USER* users;		// Ponteiro para o array de users localizado na funcao main
	DWORD* numUsers;	// Ponteiro para o numero de users registadas localizado na funcao main

	HANDLE hEvent_Board;	// Evento para avisar a Board de que a informacao foi atualizada
	CRITICAL_SECTION* pCs;	// Ponteiro para a Critical Section que protege alteracoes as variaveis 
							//	acima declaradas localizada na funcao main
} TDATA_BOLSA;

//|=========================================================================|
//|===============================| Threads |===============================|
//|=========================================================================|

DWORD WINAPI ThreadBoard(LPVOID data);

DWORD WINAPI ThreadGetClients(LPVOID data);

DWORD WINAPI ThreadClient(LPVOID data);

//|==========================================================================|
//|===============================| Comandos |===============================|
//|==========================================================================|

void ExecutaComando(
	const CMD comando, 
	TDATA_BOLSA* threadData);

BOOL ADDC(
	const CMD comando, 
	TDATA_BOLSA* threadData, 
	TCHAR* mensagem);

void LISTC(TDATA_BOLSA* threadData);

void STOCK(
	const CMD comando, 
	TDATA_BOLSA* threadData);

void USERS(TDATA_BOLSA* threadData);

void PAUSE();

void CLOSE(TDATA_BOLSA* threadData);

//|===============================================================================================|
//|===============================| Ficheiros de Dados - Empresas |===============================|
//|===============================================================================================|

// Le o conteudo do ficheiro "empresas.txt" e envia-o para a funcao "ProcessaEmpresasDoFicheiro(...);"
BOOL CarregaEmpresas(
	EMPRESA empresas[], 
	DWORD* numEmpresas);

// Recebe o conteudo do ficheiro "empresas.txt" e separa-o por linhas e envia cada linha para a funcao
// "GetEmpresa(...);" 
BOOL ProcessaEmpresasDoFicheiro(
	const TCHAR* buff,
	EMPRESA empresas[],
	DWORD* numEmpresas);

// Recebe uma linha de texto no formato <nome> <n. de acoes> <preco> e transofrma essa informacao numa
// estrutura EMPRESA
BOOL GetEmpresa(
	const TCHAR* str, 
	EMPRESA* empresa, 
	DWORD* numEmpresas);

// Constroi uma string com a informacao contida no array empresas e escreve-a para o ficheiro 
// "empresas.txt"
BOOL SalvaEmpresas(
	const EMPRESA empresas[],
	const DWORD numEmpresas);

//|============================================================================================|
//|===============================| Ficheiros de Dados - Users |===============================|
//|============================================================================================|

// Le o conteudo do ficheiro "usres.txt" e envia-o para a funcao "ProcessaUsersDoFicheiro(...);"
BOOL CarregaUsers(
	USER users[],
	DWORD* numUsers);

// Recebe o conteudo do ficheiro "users.txt" e separa-o por linhas e envia cada linha para a funcao
// "GetUser(...);"
BOOL ProcessaUsersDoFicheiro(
	const TCHAR* buff,
	USER users[],
	DWORD* numUsers);

// Recebe uma linha de texto no formato <nome> <password> <saldo> e transofrma essa informacao numa
// estrutura USER
BOOL GetUser(
	const TCHAR* str,
	USER* user,
	DWORD* numUsers);

// Constroi uma string com a informacao contida no array users e escreve-a para o ficheiro 
// "users.txt"
BOOL SalvaUsers(
	const USER users[],
	const DWORD numUsers);

//|========================================================================|
//|===============================| Outras |===============================|
//|========================================================================|

DWORD getNCLIENTES();

EMPRESA* AlocaEmpresas();

void InicializaEmpresas(EMPRESA* empresas);

USER* AlocaUsers();

void InicializaUsers(USER* users);

int ComparaEmpresas(
	const void* a, 
	const void* b);
