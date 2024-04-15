#include "Board.h"

DWORD WINAPI ThreadOutput(LPVOID data) {
	BOOL* continua = (BOOL*)data;
	TCHAR cmd[TAM_TEXT];

	while (*continua) {
		_tscanf_s(_T("%s"), cmd, TAM_TEXT);

		if (_tcscmp(cmd, _T("CLOSE")) == 0) {
			*continua = FALSE;
		}
	}
	ExitThread(0);
}