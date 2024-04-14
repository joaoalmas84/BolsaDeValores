#include <Windows.h>
#include <tchar.h>

#include "Bolsa.h"
#include "Commands.h"

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

DWORD getNCLIENTES(){
	TCHAR chave_completa[TAM] = _T("Software\\BolsaValores_SO2");
	HKEY chave;
	DWORD res;

	if (RegCreateKeyEx(HKEY_CURRENT_USER, chave_completa, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &chave, &res) != ERROR_SUCCESS) {
		PrintError(GetLastError(), _T("Erro no RegCreateKeyEx"));
		return 1;
	}

	DWORD valueType;
	DWORD dataSize;
	DWORD value;

	DWORD queryResult = RegQueryValueEx(chave, NCLIENTES, NULL, &valueType, NULL, &dataSize);

	if (queryResult == ERROR_SUCCESS) {
		if (valueType == REG_DWORD) {

			queryResult = RegQueryValueEx(chave, NCLIENTES, NULL, &valueType, (LPBYTE)&value, &dataSize);

			if (queryResult == ERROR_SUCCESS) {

				RegCloseKey(chave);
				return value;

			}else{

				PrintError(GetLastError(), _T("Erro no RegQueryValueEx"));
				RegCloseKey(chave);
				return -1;

			}

		}else{

			_tprintf_s(_T("\nTipo de dados invalido o Registry foi alterado: %d "), queryResult);
			queryResult = ERROR_FILE_NOT_FOUND;

		}
	}
	if(queryResult == ERROR_FILE_NOT_FOUND){
		_tprintf_s(_T("\nO valor não existe. Criando..."));

		DWORD AUX = STARTING_NUM_OF_NCLIENTES;
		DWORD setResult = RegSetValueEx(chave, NCLIENTES, 0, REG_DWORD, (LPBYTE)(&AUX), sizeof(AUX));

		if (setResult != ERROR_SUCCESS) {
			PrintError(GetLastError(), _T("Erro no RegSetValueEx"));
			RegCloseKey(chave);
			return -1;
		}
	}

	RegCloseKey(chave);
	return STARTING_NUM_OF_NCLIENTES;
}