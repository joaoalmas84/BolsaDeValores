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
	EMPRESA* empresas;
	DWORD* numEmpresas;
	USER* clients;
	DWORD* numClients;
	HANDLE hEvent_Board; // 0 -> EventExit; 1: EventBoard;
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

void ExecutaComando(const CMD comando, TDATA_BOLSA* threadData);

void ADDC(const CMD comando, TDATA_BOLSA* threadData);

void LISTC(TDATA_BOLSA* threadData);

void STOCK(const CMD comando, TDATA_BOLSA* threadData);

void USERS(TDATA_BOLSA* threadData);

void PAUSE();

void CLOSE(TDATA_BOLSA* threadData);

//|====================================================================================|
//|===============================| Ficheiros de Dados |===============================|
//|====================================================================================|

void LerEmpresasDoArquivo(EMPRESA empresas[], DWORD* numEmpresas);

void SalvarEmpresasNoArquivo(const EMPRESA empresas[], DWORD numEmpresas);

void LerUsersDoArquivo(USER users[], DWORD* numUsers);

void SalvarUsersNoArquivo(const USER users[], DWORD numUsers);

//|========================================================================|
//|===============================| Outras |===============================|
//|========================================================================|

DWORD getNCLIENTES();

void InitializeEmpresas(EMPRESA empresas[]);

void InitializeUsers(USER users[]);

int compara_empresas(const void* a, const void* b);
