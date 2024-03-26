#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <fcntl.h> 
#include <io.h>

int _tmain(int argc, TCHAR* argv[]) {
	int setmodeReturn;


#ifdef UNICODE
	setmodeReturn = _setmode(_fileno(stdin), _O_WTEXT);
	setmodeReturn = _setmode(_fileno(stdout), _O_WTEXT);
	setmodeReturn = _setmode(_fileno(stderr), _O_WTEXT);
#endif 

	if (argc != 2) {
		_tprintf_s(_T("O programa Board recebe 1 e apenas 1 argumento de entrada\nsendo este o n.º de empresas a listar"));
		return 1;
	}

	return 0;
}