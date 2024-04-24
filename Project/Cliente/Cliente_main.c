#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <fcntl.h> 
#include <io.h>

#include "Cliente.h"
#include "Commands.h"
#include "Utils.h"

int _tmain(int argc, TCHAR* argv[]) {
	int setmodeReturn;

	USER eu;

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

	return 0;
}