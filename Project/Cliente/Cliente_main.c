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

	BOOL continua = TRUE;

	HANDLE hPipe; // HANDLE do pipe

	TDATA_CLIENTE threadData;
	HANDLE hThread;

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

	_tprintf_s(_T("\n[CLIENTE] À Espera do named pipe '%s'... (WaitNamedPipe)"), PIPE_NAME);
	if (!WaitNamedPipe(PIPE_NAME, NMPWAIT_WAIT_FOREVER)) {
		PrintErrorMsg(GetLastError(), _T("WaitNamedPipe"));
	}

	_tprintf_s(_T("\n[CLIENTE] Ligação ao pipe do servidor estabelecida... (CreateFile)"));
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
	threadData.hPipe = hPipe;
	threadData.hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, GetCurrentThreadId());
	if (threadData.hThread == NULL) {
		PrintErrorMsg(GetLastError(), _T("OpenThread"));
		exit(-1);
	}
	threadData.pCs = &cs;

	hThread = CreateThread(NULL, 0, ThreadRead, (LPVOID)&threadData, 0, NULL);
	if (hThread == NULL) {
		PrintErrorMsg(GetLastError(), _T("CreateThread"));
		DeleteCriticalSection(&cs);
		CloseHandle(hPipe);
		return 1;
	}

	//system("cls");

	_tprintf_s(_T("\n\n[CLIENTE] Bem vindo..."));

	_tprintf_s(_T("\n\n[CLIENTE] Frase: "));


	while (1) {
		if (!GetCmd(input)) { continue; }

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

	WaitForSingleObject(hThread, INFINITE);

	DeleteCriticalSection(&cs);

	CloseHandle(hPipe);

	return 0;
}