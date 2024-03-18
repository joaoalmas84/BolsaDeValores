#include <Windows.h>
#include <tchar.h>

#include "Bolsa.h"
#include "Bolsa_cmd.h"

void ExecutaComando(const CMD comando) {
	switch (comando.Index) {
		case 0:
			ADDC();
			break;
		case 1:
			LISTC();
			break;
		case 2:
			STOCK();
			break;
		case 3:
			USERS();
			break;
		case 4:
			PAUSE();
			break;
		case 5:
			CLOSE();
			break;
	}

	// Apagar depois
	_tprintf_s(_T("\nComando: "));
	_tprintf_s(_T("\nNome: '%s'"), comando.Nome);
	_tprintf_s(_T("\nNumArgs: %d"), comando.NumArgs);
	_tprintf_s(_T("\nIndex: %d"), comando.Index);
	_tprintf_s(_T("\nArgs: "));
	for (DWORD i = 0; i < comando.NumArgs; i++) {
		_tprintf_s(_T("'%s' "), comando.Args[i]);
	}
}

void ADDC() {

}

void LISTC() {

}

void STOCK() {

}

void USERS() {

}

void PAUSE() {

}

void CLOSE() {

}
