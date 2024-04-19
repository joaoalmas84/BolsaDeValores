#pragma once

// Thread Data - Processo Board
typedef struct {
	BOOL continua;


	HANDLE* hEvent_Exit; // -> Event(Reset Auto)
	HANDLE hMutex_SHM;
	HANDLE hMutex;
} TDATA_BOARD;

DWORD WINAPI ThreadGetChar(LPVOID data);

void PrintTop(const EMPRESA empresas[], DWORD numTop);


