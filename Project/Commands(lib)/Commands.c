#include <Windows.h>
#include <tchar.h>
#include <stdlib.h>

#include "pch.h"
#include "Commands.h"
#include "Utils.h"

//|========================================================================================|
//|===============================| Validação dos comandos |===============================|
//|========================================================================================|

BOOL ValidaCmd(const TCHAR* frase, CMD* comando, TCHAR* msg, BOOL bolsa) {
	// Nomes dos comandos da Bolsa
	const TCHAR Nomes_Comandos_Bolsa[][TAM] = { _T("addc"), _T("listc"), _T("stock"),
									_T("users"),  _T("pause"),  _T("close") };

	// N.� de argumentos que cada comando da Bolsa recebe (contando com o nome do mesmo)
	const DWORD Args_Comandos_Bolsa[] = { 4, 1, 3, 1, 2, 1 };

	// Nomes dos comandos do Cliente
	const TCHAR Nomes_Comandos_Cliente[][TAM] = { _T("login"), _T("listc"),  _T("buy"),
									_T("sell"), _T("balance"), _T("exit") };

	// N.� de argumentos que cada comando do Cliente recebe (contando com o nome do mesmo)
	const DWORD Args_Comandos_Cliente[] = { 3, 1, 3, 3, 1, 1 };

	TCHAR* fraseCpy = _tcsdup(frase);

	LimpaEspacos(fraseCpy);

	ProcessaCmd(fraseCpy, comando);

	if (bolsa) {
		if (!CheckName(*comando, Nomes_Comandos_Bolsa, &comando->Index, msg)) { return FALSE; }

		if (!CheckNumArgs(*comando, Args_Comandos_Bolsa, msg)) { return FALSE; }

		if (!CheckArgsConsistency_Bolsa(*comando, msg)) { return FALSE; }
	} else {
		if (!CheckName(*comando, Nomes_Comandos_Cliente, &comando->Index, msg)) { return FALSE; }

		if (!CheckNumArgs(*comando, Args_Comandos_Cliente, msg)) { return FALSE; }

		if (!CheckArgsConsistency_Cliente(*comando, msg)) { return FALSE; }
	}

	free(fraseCpy);

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
	TCHAR* nameCpy = _tcsdup(cmd.Nome);

	for (DWORD i = 0; i < N_COMANDOS; i++) {
		if (_tcscmp(ToLowerString(nameCpy), ToLowerString(comandos[i])) == 0) {
			free(nameCpy);
			*index = i;
			msg = NULL;
			return TRUE;
		}
	}

	free(nameCpy);
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
	}
	else {
		_stprintf_s(msg, TAM, _T("Argumentos a menos, o comando '%s' recebe %d argumentos"),
			cmd.Nome, arrayArgsComandos[cmd.Index]);
		return FALSE;
	}
}

BOOL CheckArgsConsistency_Bolsa(const CMD cmd, TCHAR* msg) {

	switch (cmd.Index) {
	case 0:
		_stprintf_s(msg, TAM, _T("No comando '%s' os argumentos n.º 2 e 3 são valores numéricos maiores que 0."), cmd.Nome);

		if (IsInteger(cmd.Args[2]) && (IsDouble(cmd.Args[3]) || IsInteger(cmd.Args[3])) ) { 
			if (_ttoi(cmd.Args[2]) > 0 && _tcstod(cmd.Args[3], 0) > 0) { return TRUE; }
			else { return FALSE; }
		} else { return FALSE; }

		break;
	case 2:
		_stprintf_s(msg, TAM, _T("No comando '%s' o argumento n.º 2 é um valor numérico maior que 0."), cmd.Nome);

		if (IsInteger(cmd.Args[2]) || IsDouble(cmd.Args[2])) { 
			if (_tcstod(cmd.Args[2], 0) > 0) { return TRUE; }
			else { return FALSE; }
		} else { return FALSE; }

		break;
	case 4:
		_stprintf_s(msg, TAM, _T("No comando '%s' o argumento n.º 1 é um valor numérico inteiro maior que 0"), cmd.Nome);

		if (IsInteger(cmd.Args[1])) { 
			if (_ttoi(cmd.Args[1]) > 0) { return TRUE; }
			else { return FALSE; }
		} else { return FALSE; }

		break;
	default:
		return TRUE;
		break;
	}
}

BOOL CheckArgsConsistency_Cliente(const CMD cmd, TCHAR* msg) {
	switch (cmd.Index) {
	case 2:
	case 3:
		_stprintf_s(msg, TAM, _T("No comando '%s' o argumento n.º 2 é um inteiro"), cmd.Nome);

		if (IsInteger(cmd.Args[2])) { 
			if (_ttoi(cmd.Args[2]) > 0) { return TRUE; }
			else { return FALSE; }
		} else { return FALSE; }

		break;
	default:
		return TRUE;
		break;
	}
}

//|========================================================================================|
//|=================================| Funcoes auxiliares |=================================|
//|========================================================================================|

BOOL GetCmd(TCHAR* input) {
	_tcscpy_s(input, TAM, _T(""));

	_tprintf_s(_T("\nComando -> "));
	_getts_s(input, TAM);

	return (_tcscmp(input, _T("")) == 0) ? FALSE : TRUE;
}

DWORD GetNumArgs(const TCHAR* cmd) {
	DWORD cont = 1;
	for (DWORD i = 0; i < _tcslen(cmd); i++) {
		if (isspace(cmd[i])) { cont++; }
	}
	return cont;
}

void GetArgs(const TCHAR* cmd, const DWORD numArgs, TCHAR args[][TAM]) {
	TCHAR* cmdCpy, * nextToken = NULL;
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
	DWORD i = 0;

	while (i < dim) {
		if (_istspace(frase[i]) && _istspace(frase[i + 1])
			|| _istspace(frase[i]) && i == 0
			|| _istspace(frase[i]) && frase[i + 1] == _T('\0')) {
			for (DWORD j = i; j < dim; ++j) {
				frase[j] = frase[j + 1];
			}
			i--;
		}
		i++;
	}
}