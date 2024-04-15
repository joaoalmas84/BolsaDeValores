#pragma once

#include <windows.h>
#include <tchar.h>

#define N_COMANDOS 6
#define TAM 200

// estrutura que guarda a informa��o do comando
typedef struct {
	TCHAR Nome[TAM];
	DWORD NumArgs;
	TCHAR Args[4][TAM];
	DWORD Index;
} CMD;

//|========================================================================================|
//|===============================| Valida��o dos comandos |===============================|
//|========================================================================================|

// Valida��o do comando introduzido pelo utilizador
// RETURNS:
// TRUE: sucesso
// FALSE: insucesso (ver msg para mais informa��o)
BOOL ValidaCmd(
	const TCHAR* frase, // frase introduzida pelo utilizador
	CMD* comando,		// estrutura comando a ser preenchida
	TCHAR* msg,			// mensagem no caso das coisas correrem mal
	BOOL bolsa);		// TRUE: bolsa; FALSE: cliente;

// Preenche a vari�vel cmd com toda a informa��o sobre o comando
void ProcessaCmd(
	const TCHAR* frase, // frase introduzida pelo utilizador
	CMD* cmd);			// estrutura que guarda a info do comando

// RETURNS:
// TRUE: comando existe. (index = indice do comando e msg = NULL)
// FALSE: comando n�o existe. (mais informa��o em msg)
BOOL CheckName(
	const CMD cmd,					// comando introduzido
	const TCHAR comandos[][TAM],	// array com os nomes dos comandos
	DWORD* index,					// indice do comando introduzido
	TCHAR* msg);					// mensagem no caso das coisas correrem mal

// RETURNS:
// TRUE: n.� de argumentos do comando correto (msg = NULL)
// FALSE: n.� de argumentos do comando incorreto (mais informa��o em msg)
BOOL CheckNumArgs(
	const CMD cmd,					// comando introduzido
	const DWORD* arrayArgsComandos, // array com o n.� de argumentos de cada comando
	TCHAR* msg);					// mensagem no caso das coisas correrem mal


// RETURNS:
// TRUE: argumentos validos
// FALSE: argumentos invalidos (mais informa��o em msg)
BOOL CheckArgsConsistency_Bolsa(
	const CMD cmd,			// comando introduzido
	TCHAR* msg);			// mensagem no caso das coisas correrem mal

// RETURNS:
// TRUE: argumentos validos
// FALSE: argumentos invalidos (mais informa��o em msg)
BOOL CheckArgsConsistency_Cliente(
	const CMD cmd,			// comando introduzido
	TCHAR* msg);			// mensagem no caso das coisas correrem mal

//|========================================================================================|
//|=================================| Fun��es auxiliares |=================================|
//|========================================================================================|

// Recebe o comando introduzido pelo utilizador e guarda-o na vari�vel cmd
void GetCmd(TCHAR* cmd);

// Retorna quantos argumentos cont�m o cmd guardado na vari�vel cmd
DWORD GetNumArgs(const TCHAR* cmd);

// Organiza a frase na variavel cmd num array de strings
void GetArgs(
	const TCHAR* cmd,
	const DWORD numArgs,
	TCHAR args[][TAM]);

// Limpa espacos a mais na frase
void LimpaEspacos(TCHAR* frase);

// Devolve uma string igual � passada por par�metro mas toda em lowercase
TCHAR* ToLowerString(const TCHAR* s);

// Verifica se str � um inteiro
BOOL IsInteger(const TCHAR* str);

// Verifica se str � um float
BOOL IsFloat(const TCHAR* str);
