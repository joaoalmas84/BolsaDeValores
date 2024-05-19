#pragma once

#include <Windows.h>
#include <tchar.h>

#include "Commands.h"

// Thread Data - Processo Cliente
typedef struct {
	BOOL* continua;			// Ponteiro para a flag continua localizada na funcao main 
	BOOL* loggedIn;			// Esta flag impede que o cliente tenha sessão iniciadda em 2 contas ao mesmo tempo

	HANDLE hPipe;			// HANDLE do pipe
	HANDLE hThread_Main;	// HANDLE para possibilitar interromper operações I/O da thread main
	HANDLE hEv_Read;		// HANDLE para o EVENTO do Overlapped I/O da thread Read

	CRITICAL_SECTION* pCs;	// Ponteiro para a Critical Section que protege alteracoes as variaveis 
							// acima declaradas localizada na funcao main
} TDATA_CLIENTE;

//|=========================================================================|
//|===============================| Threads |===============================|
//|=========================================================================|

DWORD WINAPI ThreadRead(LPVOID data);

//|==========================================================================|
//|===============================| Comandos |===============================|
//|==========================================================================|

BOOL ExecutaComando(
	const CMD comando,
	TDATA_CLIENTE* threadData);

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

BOOL EXIT(TDATA_CLIENTE* threadData);

//|==============================================================================================|
//|===============================| Comunicacao Bolsa -> Cliente |===============================|
//|==============================================================================================|

BOOL GereRespostas(
	const DWORD codigo,
	TDATA_CLIENTE* threadData);

BOOL GetOperationResult(const HANDLE hPipe);

BOOL GetRespostaLista(const HANDLE hPipe);

BOOL GetRespostaBalance(const HANDLE hPipe);

