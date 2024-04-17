#pragma once

// Thread Data - Processo Board
typedef struct {
	BOOL continua;
	DWORD numTop;
	HANDLE hEvents[2]; // 0 -> Event(Reset Auto), 1 -> Event_SHM(Reset Manual)
	HANDLE hMutex;
} TDATA_BOARD;

DWORD WINAPI ThreadRead(LPVOID data);

void PrintTop(EMPRESA empresas[], DWORD numTop);


