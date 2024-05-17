#pragma once

#include <Windows.h>
#include <tchar.h>
#include <stdlib.h>
#include <string.h>

#include "Commands.h"
#include "Utils.h"
#include "Structs.h"

// Thread Data - Processo Bolsa
typedef struct {
	DWORD nclientes;		// Numero de clientes que podem estar ligados em simultaneo (Apagar se não for necessário)

	BOOL* continua;			// Ponteiro para a flag continua localizada na funcao main 

	EMPRESA* empresas;		// Ponteiro para o array de empresas localizado na funcao main
	DWORD* numEmpresas;		// Ponteiro para o numero de empresas registadas localizado na funcao main
	
	USER* users;			// Ponteiro para o array de users localizado na funcao main
	DWORD* numUsers;		// Ponteiro para o numero de users registadas localizado na funcao main

	HANDLE hEvent_Board;	// Evento para avisar a Board de que a informacao foi atualizada
	CRITICAL_SECTION* pCs;	// Ponteiro para a Critical Section que protege alteracoes as variaveis 
							//	acima declaradas localizada na funcao main

	HANDLE hThreads[NCLIENTES];
} TDATA_BOLSA;

// Thread Data - ThreadClient
typedef struct {
	DWORD id;
	BOOL ligado;
	HANDLE hPipe;
	HANDLE hSemClientes;
	TDATA_BOLSA* ptd;
} TD_WRAPPER;

//|=========================================================================|
//|===============================| Threads |===============================|
//|=========================================================================|

DWORD WINAPI ThreadBoard(LPVOID data);

DWORD WINAPI ThreadGetClients(LPVOID data);

DWORD WINAPI ThreadClient(LPVOID data);

//|==============================================================================================|
//|===============================| Comunicacao Cliente -> Bolsa |===============================|
//|==============================================================================================|

BOOL GerePedidos(
	TD_WRAPPER* threadData,
	const DWORD codigo);

BOOL GetLogin(
	const DWORD id, 
	const HANDLE hPipe, 
	_LOGIN* login);


BOOL GetCompra(
	const DWORD id,
	const HANDLE hPipe);

BOOL GetVenda(
	const DWORD id,
	const HANDLE hPipe);

//|========================================================================================|
//|===============================| Validação de operações |===============================|
//|========================================================================================|

RESPOSTA_LOGIN ValidaLogin(
	TD_WRAPPER* threadData,
	const _LOGIN login);

BOOL ValidaCompra(
	TD_WRAPPER* threadData,
	const COMPRA compra);

BOOL ValidaVenda(
	TD_WRAPPER* threadData,
	const VENDA venda);

//|==============================================================================================|
//|===============================| Comunicacao Bolsa -> Cliente |===============================|
//|==============================================================================================|

BOOL SendAvisoLogin(
	const HANDLE hPipe,
	const DWORD id);

BOOL SendRespostaLogin(
	const HANDLE hPipe, 
	const DWORD id, 
	const RESPOSTA_LOGIN r_login);

BOOL SendRespostaLista();

BOOL SendRespostaCompra();

BOOL SendRespostaVenda();

BOOL SendRespostaBalance();

//|==========================================================================|
//|===============================| Comandos |===============================|
//|==========================================================================|

BOOL ExecutaComando(
	const CMD comando, 
	TDATA_BOLSA* threadData,
	TCHAR* errorMsg);

BOOL ADDC(
	const CMD comando, 
	TDATA_BOLSA* threadData,
	TCHAR* mensagem);

void LISTC(TDATA_BOLSA* threadData);

BOOL STOCK(
	const CMD comando, 
	TDATA_BOLSA* threadData,
	TCHAR* mensagem);

void USERS(TDATA_BOLSA* threadData);

void PAUSE();

void CLOSE();

//|===============================================================================================|
//|===============================| Ficheiros de Dados - Empresas |===============================|
//|===============================================================================================|

// Le o conteudo do ficheiro "empresas.txt" e envia-o para a funcao "ProcessaEmpresasDoFicheiro(...);"
BOOL CarregaEmpresas(
	EMPRESA empresas[], 
	DWORD* numEmpresas);

// Recebe o conteudo do ficheiro "empresas.txt" e separa-o por linhas e envia cada linha para a funcao
// "GetEmpresa(...);" 
BOOL ProcessaEmpresasDoFicheiro(
	const TCHAR* buff,
	EMPRESA empresas[],
	DWORD* numEmpresas);

// Recebe uma linha de texto no formato <nome> <n. de acoes> <preco> e transofrma essa informacao numa
// estrutura EMPRESA
BOOL GetEmpresa(
	const TCHAR* str, 
	EMPRESA* empresa, 
	DWORD* numEmpresas);

// Constroi uma string com a informacao contida no array empresas e escreve-a para o ficheiro 
// "empresas.txt"
BOOL SalvaEmpresas(
	const EMPRESA empresas[],
	const DWORD numEmpresas);

//|============================================================================================|
//|===============================| Ficheiros de Dados - Users |===============================|
//|============================================================================================|

// Le o conteudo do ficheiro "usres.txt" e envia-o para a funcao "ProcessaUsersDoFicheiro(...);"
BOOL CarregaUsers(
	USER users[],
	DWORD* numUsers);

// Recebe o conteudo do ficheiro "users.txt" e separa-o por linhas e envia cada linha para a funcao
// "GetUser(...);"
BOOL ProcessaUsersDoFicheiro(
	const TCHAR* buff,
	USER users[],
	DWORD* numUsers);

// Recebe uma linha de texto no formato <nome> <password> <saldo> e transofrma essa informacao numa
// estrutura USER
BOOL GetUser(
	const TCHAR* str,
	USER* user,
	DWORD* numUsers);

// Constroi uma string com a informacao contida no array users e escreve-a para o ficheiro 
// "users.txt"
BOOL SalvaUsers(
	const USER users[],
	const DWORD numUsers);

//|========================================================================|
//|===============================| Outras |===============================|
//|========================================================================|

DWORD getNCLIENTES();

EMPRESA* AlocaEmpresas();

void InicializaEmpresas(EMPRESA* empresas);

USER* AlocaUsers();

void InicializaUsers(USER* users);

int ComparaEmpresas(
	const void* a, 
	const void* b);

DWORD getHandlePipeLivre(HANDLE hPipes[NCLIENTES]);
