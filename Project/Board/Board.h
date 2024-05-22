#pragma once

#include "Utils.h"
#include "Structs.h"

#include "Board.h"

// Thread Data - Processo Board
typedef struct {
	BOOL* continua;
	CRITICAL_SECTION* pCs;
	HANDLE hEvent_Exit; // Evento Exit (Reset Atomatico)
	HANDLE hMutex_SHM;
} TDATA_BOARD;

DWORD WINAPI ThreadGetChar(LPVOID data);

void PrintTop(
	const EMPRESA empresas[], 
	DWORD numTop, 
	DWORD numEmpresas);


