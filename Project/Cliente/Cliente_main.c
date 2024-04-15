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

	// Buffer para guardar mensagens de erro
	TCHAR msg[TAM];

	// Buffer para guardar o comando recebido
	TCHAR input[TAM];

	// Estrutura comando
	CMD comando;

	TCHAR c;

#ifdef UNICODE
	setmodeReturn = _setmode(_fileno(stdin), _O_WTEXT);
	setmodeReturn = _setmode(_fileno(stdout), _O_WTEXT);
	setmodeReturn = _setmode(_fileno(stderr), _O_WTEXT);
#endif 

	while (1) {
		GetCmd(input);
		c = _gettchar();

		if (!ValidaCmd(input, &comando, msg, FALSE)) {
			_tprintf(_T("\n[ERRO] %s."), msg);
		} else {
			ExecutaComando(comando);

			if (comando.Index == 5) { break; }
		}

	}


	return 0;
}