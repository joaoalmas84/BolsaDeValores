#pragma once

#include <Windows.h>
#include <tchar.h>

#include "Commands.h"

// Thread Data - Processo Cliente
typedef struct {
	BOOL* continua;			// Ponteiro para a flag continua localizada na funcao main 
	
	
	HANDLE hPipe;			// HANDLE do pipe
	CRITICAL_SECTION* pCs;	// Ponteiro para a Critical Section que protege alteracoes as variaveis 
							//	acima declaradas localizada na funcao main
} TDATA_CLIENTE;

//|=========================================================================|
//|===============================| Threads |===============================|
//|=========================================================================|

DWORD WINAPI ThreadRead(LPVOID data);

//|==========================================================================|
//|===============================| Comandos |===============================|
//|==========================================================================|

void ExecutaComando(const CMD comando);

void LOGIN();

void LISTC();

void BUY();

void SELL();

void BALANCE();

void EXIT();

