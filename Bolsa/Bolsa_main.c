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

	// Variaveis referentes ao Registry
	HKEY chave;
	TCHAR chave_completa[TAM], chave_nome[TAM], par_nome[TAM], par_valor[TAM];

	// Buffer para guardar mensagens de erro
	TCHAR msg[TAM];

	// Buffer para guardar o comando recebido
	TCHAR input[TAM];

	// Estrutura comando
	CMD comando;

	TCHAR c;
	DWORD res;

#ifdef UNICODE
	setmodeReturn = _setmode(_fileno(stdin), _O_WTEXT);
	setmodeReturn = _setmode(_fileno(stdout), _O_WTEXT);
	setmodeReturn = _setmode(_fileno(stderr), _O_WTEXT);
#endif 

	//if (argc != 2) {
	//	_tprintf_s(_T("O programa Bolsa recebe 1 e apenas 1 argumento de entrada\nsendo este o nome do ficheiro com a informação relativa aos clientes."));
	//	return 1;
	//}

	_tcscpy_s(chave_nome, TAM, _T("NCLIENTES"));

	_stprintf_s(chave_completa, TAM, _T("BolsaValores_SO2\\%s"), chave_nome);

	if (RegCreateKeyEx(HKEY_CURRENT_USER, chave_completa, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &chave, &res) != ERROR_SUCCESS) {
		PrintError(GetLastError(), _T("Erro no RegCreateKeyEx"));
		return 1;
	}

	while (1) {
		GetCmd(input);
		c = _gettchar();

		if (!ValidaCmd(input, &comando, msg, TRUE)) {
			_tprintf(_T("\n[ERRO] %s."), msg);
		} else {
			ExecutaComando(comando);
			if (comando.Index == 5) { break; }
		}
	}

	RegCloseKey(chave);
	return 0;
}

