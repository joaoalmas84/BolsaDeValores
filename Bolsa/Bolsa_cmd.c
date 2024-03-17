#include <Windows.h>
#include <tchar.h>

#include "Bolsa_cmd.h"

BOOL ValidaCmd(TCHAR* frase, TCHAR* msg) {
	// Nomes dos comandos
	const TCHAR Nomes_Comandos[][TAM] = { _T("addc"), _T("listc"), _T("stock"),
								_T("users"),  _T("pause"),  _T("close") };

	// N.º de argumentos que cada comando recebe (contando com o nome do mesmo)
	const DWORD Args_Comandos[] = { 4, 1, 3, 1, 2, 1 };

	// Estrutura que guarda a info do comando
	CMD cmd;

	LimpaEspacos(frase);

	ProcessaCmd(frase, &cmd);
	if (!CheckName(cmd, Nomes_Comandos, &cmd.Index, msg)) { return FALSE; }

	if (!CheckNumArgs(cmd, Args_Comandos, msg)) { return FALSE; }

	if (!CheckArgsConsistency(cmd, msg)) { return FALSE; }

	return TRUE;
}

void ProcessaCmd(const TCHAR* frase, CMD* cmd) {
	TCHAR* fraseCpy;
	TCHAR* nextToken = NULL;

	fraseCpy = _tcsdup(frase);

	_tcscpy_s(cmd->Nome, TAM, _tcstok_s(fraseCpy, _T(" "), &nextToken));

	cmd->NumArgs = GetNumArgs(frase) > 4 ? 4 : GetNumArgs(frase);

	GetArgs(frase, cmd->NumArgs, cmd->Args);

	free(fraseCpy);
}

BOOL CheckName(const CMD cmd, const TCHAR comandos[][TAM], DWORD* index, TCHAR* msg) {
	for (DWORD i = 0; i < N_COMANDOS; i++) {
		if (_tcscmp(ToLowerString(cmd.Nome), ToLowerString(comandos[i])) == 0) {
			*index = i;
			msg = NULL;
			return TRUE;
		}
	}
	*index = -1;
	_tcscpy_s(msg, TAM, _T("Comando desconhecido"));
	return FALSE;
}

BOOL CheckNumArgs(const CMD cmd, const DWORD* arrayArgsComandos, TCHAR* msg) {
	if (cmd.NumArgs == arrayArgsComandos[cmd.Index]) { return TRUE; }
	else if (cmd.NumArgs > arrayArgsComandos[cmd.Index]) {
		_stprintf_s(msg, TAM, _T("Argumentos a mais, o comando '%s' só recebe %d argumentos"),
			cmd.Nome, arrayArgsComandos[cmd.Index]);
		return FALSE;
	} else {
		_stprintf_s(msg,TAM, _T("Argumentos a menos, o comando '%s' recebe %d argumentos"), 
			cmd.Nome, arrayArgsComandos[cmd.Index]);
		return FALSE;
	}
}

BOOL CheckArgsConsistency(const CMD cmd, TCHAR* msg) {
	switch (cmd.Index) {
		case 0:
			if (IsInteger(cmd.Args[2]) && IsFloat(cmd.Args[3])) { return TRUE; }
			else { 
				_stprintf_s(msg, TAM, _T("No comando '%s' os argumentos n.º 2 e 3 são inteiros"), cmd.Nome);
				return FALSE; 
			}
			break;
		case 2:
			if (IsInteger(cmd.Args[2])) { return TRUE; }
			else {
				_stprintf_s(msg, TAM, _T("No comando '%s' o argumento n.º 2 é um inteiro"), cmd.Nome);
				return FALSE;
			}
			break;
		case 4:
			if (IsInteger(cmd.Args[1])) { return TRUE; }
			else {
				_stprintf_s(msg, TAM, _T("No comando '%s' o argumento n.º 1 são inteiros"), cmd.Nome);
				return FALSE;
			}
			break;
		default:
			return TRUE;
			break;
	}
}
//|========================================================================================|
//|=================================| Funções auxiliares |=================================|
//|========================================================================================|

void GetCmd(TCHAR* cmd) {
	_tprintf_s(_T("\nComando->"));
	_tscanf_s(_T("%[^\n]"), cmd, TAM);
}

DWORD GetNumArgs(const TCHAR* cmd) {
	DWORD cont = 1;
	for (DWORD i = 0; i < _tcslen(cmd); i++) {
		if (isspace(cmd[i])) { cont++; }
	}
	return cont;
}

void GetArgs(const TCHAR* cmd, const DWORD numArgs, TCHAR args[][TAM]) {
	TCHAR* cmdCpy, *nextToken = NULL;
	DWORD cont = 0;

	cmdCpy = _tcsdup(cmd);

	_tcscpy_s(args[cont++], TAM, _tcstok_s(cmdCpy, _T(" \t"), &nextToken));

	while (cont < numArgs) {
		_tcscpy_s(args[cont], TAM, _tcstok_s(NULL, _T(" \t"), &nextToken));
		cont++;
	}

	free(cmdCpy);
}

void LimpaEspacos(TCHAR* frase) {
	size_t dim = _tcslen(frase);

	for (DWORD i = 0; i < dim; ++i) {
		if (isspace(frase[i]) && isspace(frase[i + 1])
			|| isspace(frase[i]) && i == 0
			|| isspace(frase[i]) && frase[i + 1] == _T('\0')) {
			for (DWORD j = i; j < dim; ++j) {
				frase[j] = frase[j + 1];
			}
			i--;
		}
	}
}

TCHAR* ToLowerString(const TCHAR* s) {
	TCHAR* aux = malloc(sizeof(TCHAR) * (_tcslen(s)));
	int i;

	if (aux == NULL) {return NULL;}

	for (i = 0; s[i] != '\0'; ++i) {
		aux[i] = tolower(s[i]);
	}
	aux[i] = '\0';
	
	return aux;
}

BOOL IsInteger(const TCHAR* str) {
	TCHAR* endPtr;
	long value = _tcstol(str, &endPtr, 10); // Convert string to long

	// Check if the conversion was successful
	if (str == endPtr || *endPtr != _T('\0')) {
		// Conversion failed or entire string was not converted
		return FALSE;
	}

	return TRUE;
}

BOOL IsFloat(const TCHAR* str) {
	TCHAR* endPtr;
	double res;

	res = _tcstod(str, &endPtr); // Convert string to double

	// Check if the entire string was converted
	if (str == endPtr || *endPtr != '\0') {
		// Conversion failed or entire string was not converted
		return FALSE;
	}

	return TRUE;
}