#pragma once

// Thread Data - Processo Board
typedef struct {
	BOOL continua;
	HANDLE hEvents[2]; // [0] Evento SHM(Reset Manual); [1] Evento Exit(Reset Atomatico)
	HANDLE hMutex_SHM;
	HANDLE hMutex;
} TDATA_BOARD;

DWORD WINAPI ThreadGetChar(LPVOID data);

void PrintTop(const EMPRESA empresas[], DWORD numTop);


