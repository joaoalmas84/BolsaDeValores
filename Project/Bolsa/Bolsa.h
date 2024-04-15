#pragma once

#include <Windows.h>
#include <tchar.h>

#include <stdlib.h>
#include <string.h>

#include "Commands.h"
#include "Utils.h"

// Macros
#define NCLIENTES _T("NCLIENTES")
#define FILE_DADOS_EMPRESAS _T("dadosEmpresas.txt") // nome do ficheiro de emprecas 
#define FILE_DADOS_USERS _T("dadosUsers.txt")		// nome do ficheiro de users 

// Valores Constantes
#define STARTING_NUM_OF_NCLIENTES 5	// número default PARA O NCLIENTES 
#define NUMERO_MAX_DE_EMPRESAS 30	// numero inicial para as empresas
#define NUMERO_MAX_DE_USERS 20		// numero inicial para as users

typedef struct {
	BOOL continua;
	EMPRESA* empresas;
	DWORD* numEmpresas;
} TDATA_BOLSA;

//|=========================================================================|
//|===============================| Threads |===============================|
//|=========================================================================|

DWORD WINAPI ThreadBoard(LPVOID data);

//|==========================================================================|
//|===============================| Comandos |===============================|
//|==========================================================================|

void ExecutaComando(
	const CMD comando, 
	EMPRESA* empresas, 
	DWORD* numEmpresas, 
	CARTEIRA* users,
	DWORD* numUsers,
	TDATA_BOLSA* threadData,
	HANDLE* hThread);

void ADDC(
	const CMD comando,
	EMPRESA* empresas,
	DWORD* numEmpresas);

void LISTC(
	const EMPRESA* empresas, 
	DWORD numDeEmpresas);

void STOCK(
	EMPRESA* empresas, 
	DWORD numEmpresas, 
	const TCHAR* nomeDaEmpresa,
	const TCHAR* preco);

void USERS(
	const CARTEIRA* users, 
	DWORD numUsers);

void PAUSE();

void CLOSE(
	TDATA_BOLSA* infoThreadMemoria,
	HANDLE hthreadMemoria);

//|====================================================================================|
//|===============================| Ficheiros de Dados |===============================|
//|====================================================================================|

void LerEmpresasDoArquivo(EMPRESA* empresas, DWORD* numEmpresas);

void SalvarEmpresasNoArquivo(EMPRESA* empresas, DWORD numEmpresas);

void LerUsersDoArquivo(CARTEIRA* users, DWORD* numUsers);

void SalvarUsersNoArquivo(CARTEIRA* users, DWORD numUsers);

//|========================================================================|
//|===============================| Outras |===============================|
//|========================================================================|

DWORD getNCLIENTES();

int compara_empresas(const void* a, const void* b);
