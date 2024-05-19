#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <fcntl.h> 
#include <io.h>

#include "Commands.h"
#include "Utils.h"

#include "Cliente.h"


int _tmain(int argc, TCHAR* argv[]) {
	int setmodeReturn;
	DWORD err;

	BOOL continua = TRUE;
	BOOL loggedIn = FALSE;

	HANDLE hPipe; // HANDLE do pipe

	TDATA_CLIENTE threadData;
	HANDLE hThread_Read;

	CRITICAL_SECTION cs;// Critical Section para proteger alteracoes feitas a estrutura TDATA_CLIENTE

	// Buffer para guardar mensagens de erro
	TCHAR errorMsg[SMALL_TEXT];

	// Buffer para guardar o comando recebido
	TCHAR input[SMALL_TEXT];

	// Estrutura comando
	CMD comando;

#ifdef UNICODE
	setmodeReturn = _setmode(_fileno(stdin), _O_WTEXT);
	setmodeReturn = _setmode(_fileno(stdout), _O_WTEXT);
	setmodeReturn = _setmode(_fileno(stderr), _O_WTEXT);
#endif 

	_tprintf_s(_T("\n[CLIENTE] A estabelecer ligação com a Bolsa... (%s)"), PIPE_NAME);
	if (!WaitNamedPipe(PIPE_NAME, NMPWAIT_WAIT_FOREVER)) {
		err = GetLastError();
		if (err == ERROR_FILE_NOT_FOUND) {
			_tprintf_s(_T("\n[CLIENTE] O processo Bolsa não se encontra em execução...\n"));
			return -1;
		} else {
			PrintErrorMsg(err, _T("WaitNamedPipe"));
			return -1;
		}
	}

	_tprintf_s(_T("\n[CLIENTE] Ligação estabelecida..."));
	hPipe = CreateFile(PIPE_NAME, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);
	if (hPipe == INVALID_HANDLE_VALUE) {
		PrintErrorMsg(GetLastError(), _T("CreateFile"));
	}

	if (!InitializeCriticalSectionAndSpinCount(&cs, 0)) {
		PrintErrorMsg(GetLastError(), _T("InitializeCriticalSectionAndSpinCount"));
		CloseHandle(hPipe);
		return 1;
	}

	threadData.continua = &continua;
	threadData.loggedIn = &loggedIn;
	threadData.hPipe = hPipe;
	threadData.hThread_Main = OpenThread(THREAD_ALL_ACCESS, FALSE, GetCurrentThreadId());
	if (threadData.hThread_Main == NULL) {
		PrintErrorMsg(GetLastError(), _T("OpenThread"));
		exit(-1);
	}
	threadData.pCs = &cs;

	// Lançamento da ThreadRead
	hThread_Read = CreateThread(NULL, 0, ThreadRead, (LPVOID)&threadData, 0, NULL);
	if (hThread_Read == NULL) {
		PrintErrorMsg(GetLastError(), _T("CreateThread"));
		DeleteCriticalSection(&cs);
		CloseHandle(hPipe);
		return 1;
	}

	Sleep(1);  // <- Este sleep apenas serve para manipular a ordem em que aparecem os printf's

	_tprintf_s(_T("\n\n[CLIENTE] Bem vindo..."));

	while (1) {
		if (!GetCmd(input)) { 

			EnterCriticalSection(threadData.pCs);
			continua = *threadData.continua;
			LeaveCriticalSection(threadData.pCs);

			if (continua) { continue; }
			else { break; }
		}

		if (!ValidaCmd(input, &comando, errorMsg, FALSE)) {
			_tprintf(_T("\n[ERRO] %s."), errorMsg);
		} else {
			if (!ExecutaComando(comando, &threadData)) {
				_tprintf(_T("\n[ERRO] Execução do comando '%s' falhou."), comando.Nome);
			}
			if (comando.Index == 5) { break; }
		}
	}

	_tprintf_s(_T("\n[CLIENTE] A terminar..."));

	WaitForSingleObject(hThread_Read, INFINITE);

	DeleteCriticalSection(&cs);

	CloseHandle(hPipe);

	return 0;
}