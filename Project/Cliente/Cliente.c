#include <Windows.h>
#include <tchar.h>

#include "Cliente.h"
#include "Commands.h"

void ExecutaComando(const CMD comando) {
	switch (comando.Index) {
		case 0:
			LOGIN();
			break;
		case 1:
			LISTC();
			break;
		case 2:
			BUY();
			break;
		case 3:
			SELL();
			break;
		case 4:
			BALANCE();
			break;
		case 5:
			EXIT();
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

void LOGIN() {

}

void LISTC() {

}

void BUY() {

}

void SELL() {

}

void BALANCE() {

}

void EXIT() {

}