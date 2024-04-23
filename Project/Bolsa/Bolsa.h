#pragma once

#include <Windows.h>
#include <tchar.h>
#include <stdlib.h>
#include <string.h>

#include "Commands.h"
#include "Utils.h"

// Thread Data - Processo Bolsa
typedef struct {
	BOOL* continua;		// Ponteiro para a flag continua localizada na funcao main

	EMPRESA* empresas;	// Ponteiro para o array de empresas localizado na funcao main
	DWORD* numEmpresas;	// Ponteiro para o numero de empresas registadas localizado na funcao main
	
	USER* users;		// Ponteiro para o array de users localizado na função main
	DWORD* numUsers;	// Ponteiro para o numero de users registadas localizado na funcao main

	HANDLE hEvent_Board;// Evento para avisar a Board de que a informacao foi atualizada
	HANDLE hMutex;		
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

BOOL CarregaEmpresas(
	EMPRESA empresas[], 
	DWORD* numEmpresas);

BOOL ProcessaEmpresasDoFicheiro(
	const TCHAR* buff,
	EMPRESA empresas[],
	DWORD* numEmpresas);

BOOL GetEmpresa(
	const TCHAR* str, 
	EMPRESA* empresa, 
	DWORD* numEmpresas);

BOOL SalvaEmpresas(
	const EMPRESA empresas[],
	const DWORD numEmpresas);

//|============================================================================================|
//|===============================| Ficheiros de Dados - Users |===============================|
//|============================================================================================|

BOOL CarregaUsers(
	USER users[],
	DWORD* numUsers);

BOOL ProcessaUsersDoFicheiro(
	const TCHAR* buff,
	USER users[],
	DWORD* numUsers);

BOOL GetUser(
	const TCHAR* str,
	USER* user,
	DWORD* numUsers);

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
