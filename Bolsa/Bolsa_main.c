#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <fcntl.h> 
#include <io.h>

#include "Bolsa.h"
#include "Bolsa_cmd.h"

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

	/*if (argc != 2) {
		_tprintf_s(_T("O programa Bolsa recebe 1 e apenas 1 argumento de entrada\nsendo este o nome do ficheiro com a informação relativa aos clientes."));
		return 1;
	}*/

	while (1) {
		GetCmd(input);
		c = _gettchar();

		if (!ValidaCmd(input, msg, &comando)) {
			_tprintf(_T("\n[ERRO] %s."), msg);
			continue;
		}
	
		ExecutaComando(comando);

		if (comando.Index == 5) { break; }
	}

	return 0;
}