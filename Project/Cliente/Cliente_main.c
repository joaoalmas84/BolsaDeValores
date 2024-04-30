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

	BOOL continua = FALSE;

	HANDLE hPipe; // HANDLE do pipe

	USER eu;

	TDATA_CLIENTE threadData;
	HANDLE hThread;
	DWORD threadId;
	CRITICAL_SECTION cs;// Critical Section para proteger alteracoes feitas a estrutura TDATA_CLIENTE

	// Buffer para guardar mensagens de erro
	TCHAR errorMsg[TAM];

	// Buffer para guardar o comando recebido
	TCHAR input[TAM];

	// Estrutura comando
	CMD comando;

#ifdef UNICODE
	setmodeReturn = _setmode(_fileno(stdin), _O_WTEXT);
	setmodeReturn = _setmode(_fileno(stdout), _O_WTEXT);
	setmodeReturn = _setmode(_fileno(stderr), _O_WTEXT);
#endif 

	if (!WaitNamedPipe(PIPE_NAME, NMPWAIT_WAIT_FOREVER)) {
		PrintErrorMsg(GetLastError(), _T("Erro em WaitNamedPipe"));
	}

	hPipe = CreateFile(PIPE_NAME, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 
		FILE_ATTRIBUTE_NORMAL, NULL);
	if (hPipe == INVALID_HANDLE_VALUE) {
		PrintErrorMsg(GetLastError(), _T("Erro em CreateFile"));
	}

	if (!InitializeCriticalSectionAndSpinCount(&cs, 0)) {
		PrintErrorMsg(GetLastError(), _T("Erro em InitializeCriticalSectionAndSpinCount"));
		CloseHandle(hPipe);
		return 1;
	}

	threadData.continua = &continua;
	threadData.hPipe = hPipe;
	threadData.pCs = &cs;

	//hThread = CreateThread(NULL, 0, ThreadRead, (LPVOID)&threadData, 0, &threadId);
	//if (hThread == NULL) {
	//	PrintErrorMsg(GetLastError(), _T("Erro ao lançar ThreadRead"));
	//	DeleteCriticalSection(&cs);
	//	CloseHandle(hPipe);
	//	return 1;
	//}

	system("cls");

	while (1) {
		if (!GetCmd(input)) { continue; }

		if (!ValidaCmd(input, &comando, errorMsg, TRUE)) {
			_tprintf(_T("\n[ERRO] %s."), errorMsg);
		} else {
			ExecutaComando(comando);
			if (comando.Index == 5) { break; }
		}
	}

	DeleteCriticalSection(&cs);

	CloseHandle(hPipe);

	return 0;
}