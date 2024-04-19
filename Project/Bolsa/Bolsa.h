#pragma once

#include <Windows.h>
#include <tchar.h>
#include <stdlib.h>
#include <string.h>

#include "Commands.h"
#include "Utils.h"

// Thread Data - Processo Bolsa
typedef struct {
	BOOL continua;		
	EMPRESA* empresas;	// Array de empresas registadas
	DWORD* numEmpresas;	// Numero de empresas registadas
	USER* users;		// Array de users registados (ou ligados... ainda nao sei)
	DWORD* numUsers;	// Numero de users registados
	HANDLE hEvent_Board;// Evento para avisar a Board de que a infirmacao foi atualizada
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

void ADDC(
	const CMD comando, 
	TDATA_BOLSA* threadData);

void LISTC(TDATA_BOLSA* threadData);

void STOCK(
	const CMD comando, 
	TDATA_BOLSA* threadData);

void USERS(TDATA_BOLSA* threadData);

void PAUSE();

void CLOSE(TDATA_BOLSA* threadData);

//|====================================================================================|
//|===============================| Ficheiros de Dados |===============================|
//|====================================================================================|

BOOL CarregaEmpresas(
	EMPRESA empresas[], 
	DWORD* numEmpresas,
	TCHAR* errorMsg,
	DWORD* codigoErro);

BOOL SalvaEmpresas(
	const EMPRESA empresas[],
	DWORD numEmpresas,
	TCHAR* msg,
	DWORD* codigoErro);

BOOL CarregaUsers(
	USER users[], 
	DWORD* numUsers,
	TCHAR* msg,
	DWORD* codigoErro);

BOOL SalvaUsers(
	const USER users[],
	DWORD numUsers,
	TCHAR* msg);

//|========================================================================|
//|===============================| Outras |===============================|
//|========================================================================|

DWORD getNCLIENTES();

// Inicializa o array empresas com valores nulos
void InitializeEmpresas(EMPRESA empresas[]);

// Inicializa o array users com valores nulos
void InitializeUsers(USER users[]);

int compara_empresas(
	const void* a, 
	const void* b);
