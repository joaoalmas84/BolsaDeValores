#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <fcntl.h> 
#include <io.h>

#include "Bolsa.h"
#include "Commands.h"
#include "Utils.h"

int _tmain(int argc, TCHAR* argv[]) {
	int setmodeReturn;

	DWORD numClintes = -1;

	// Buffer para guardar mensagens de erro
	TCHAR msg[TAM];

	// Buffer para guardar o comando recebido
	TCHAR input[TAM];

	// Estrutura comando
	CMD comando;

	TCHAR c;
	//DWORD res;

#ifdef UNICODE
	setmodeReturn = _setmode(_fileno(stdin), _O_WTEXT);
	setmodeReturn = _setmode(_fileno(stdout), _O_WTEXT);
	setmodeReturn = _setmode(_fileno(stderr), _O_WTEXT);
#endif 

	if ((numClintes = getNCLIENTES()) == -1) {
		return -1;
	}

	while (1) {
		GetCmd(input);
		c = _gettchar();

		if (!ValidaCmd(input, &comando, msg, TRUE)) {
			_tprintf(_T("\n[ERRO] %s."), msg);
		}
		else {
			ExecutaComando(comando);
			if (comando.Index == 5) { break; }
		}
	}
	return 0;
}

