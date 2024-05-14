#pragma once

#include <windows.h>
#include <tchar.h>
#include <windows.h>
#include <windowsx.h>
#include <CommCtrl.h>
#include "resource.h"
#include "Structs.h"

#define MAX_EMPRESAS_BoardGUI 10
#define MINIMO_EMPRESAS 0
#define MAX_DWORD 4294967295



typedef struct {
	CRITICAL_SECTION cs;
	CRITICAL_SECTION csInterfase;
	SHM* sharedMemory;
	BOOL continua;
	HANDLE hEvent_Exit;
	HANDLE hThread;
	DWORD NumEmpresas;
	DWORD valorGrafico;

}DATA_GLOBAL;

typedef struct {
	CRITICAL_SECTION* cs;
	HWND hWnd;
	BOOL* continua;
	SHM** sharedMemory;
	HANDLE* hEvent_Exit;
}data_thread;

DWORD WINAPI ThreadMemoria(LPVOID data);

LRESULT CALLBACK TrataEventos(HWND hWnd, UINT messg/*codigo*/, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK TrataEventosInfo(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);

DWORD intervalo(DWORD MAX, DWORD MIN, DWORD var);

DWORD getNumberOfBox(HWND hWnd, int ID, DWORD* var);

DATA_GLOBAL defult_DATA_GLOBAL();

void setBarras(HDC hdc, DATA_GLOBAL* dadosPartilhados, DWORD destancia_lateral, DWORD yBitmap, BITMAP bmp, HDC bmpDC, DWORD i, BITMAP bmpgrafico, DWORD alturaDosPrecos);

BOOL Pdefult_DATA_GLOBAL(DATA_GLOBAL** dados);