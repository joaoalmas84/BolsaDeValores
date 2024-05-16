#pragma once

#include <Windows.h>
#include <tchar.h>

#include "Commands.h"

// Thread Data - Processo Cliente
typedef struct {
	BOOL* continua;			// Ponteiro para a flag continua localizada na funcao main 
	
	HANDLE hPipe;			// HANDLE do pipe
	HANDLE hThread;			// HANDLE para thread main para possibilitar interromper operações I/O

	CRITICAL_SECTION* pCs;	// Ponteiro para a Critical Section que protege alteracoes as variaveis 
							//	acima declaradas localizada na funcao main
} TDATA_CLIENTE;

//|=========================================================================|
//|===============================| Threads |===============================|
//|=========================================================================|

DWORD WINAPI ThreadRead(LPVOID data);

//|==============================================================================================|
//|===============================| Comunicacao Bolsa -> Cliente |===============================|
//|==============================================================================================|

BOOL GereRespostas(
	const DWORD codigo,
	TDATA_CLIENTE* threadData);

BOOL GetRespostaLogin(const HANDLE hPipe);

//|==========================================================================|
//|===============================| Comandos |===============================|
//|==========================================================================|

BOOL ExecutaComando(
	const CMD comando, 
	TDATA_CLIENTE* ptd);

BOOL LOGIN(
	const CMD comando, 
	const TDATA_CLIENTE threadData);

BOOL LISTC(
	const CMD comando, 
	const TDATA_CLIENTE threadData);

BOOL BUY(
	const CMD comando, 
	const TDATA_CLIENTE threadData);

BOOL SELL(
	const CMD comando, 
	const TDATA_CLIENTE threadData);

BOOL BALANCE(
	const CMD comando, 
	const TDATA_CLIENTE threadData);

BOOL EXIT(
	const CMD comando, 
	const TDATA_CLIENTE threadData);

