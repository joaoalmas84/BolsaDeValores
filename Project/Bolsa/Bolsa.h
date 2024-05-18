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

	DWORD pauseTime;
	BOOL* pause;

	EMPRESA* empresas;		// Ponteiro para o array de empresas localizado na funcao main
	DWORD* numEmpresas;		// Ponteiro para o numero de empresas registadas localizado na funcao main
	
	USER* users;			// Ponteiro para o array de users localizado na funcao main
	DWORD* numUsers;		// Ponteiro para o numero de users registadas localizado na funcao main

	HANDLE hEvent_Board;	// Evento para avisar a Board de que a informacao foi atualizada

	CRITICAL_SECTION* pCs;	// Ponteiro para a Critical Section que protege alteracoes as variaveis 
							// acima declaradas localizada na funcao main
	HANDLE hThreadMain;

	HANDLE hEv_Read[NCLIENTES];

	HANDLE hEv_Conn;
	HANDLE hEv_Pause;

	TCHAR* ultimaTransacao;

} TDATA_BOLSA;

// Thread Data - ThreadClient
typedef struct {
	DWORD id;
	BOOL ligado;
	HANDLE hPipe;
	HANDLE hSemClientes;

	TCHAR nomeUser[SMALL_TEXT];

	TDATA_BOLSA* ptd;
} TD_WRAPPER;

//|=========================================================================|
//|===============================| Threads |===============================|
//|=========================================================================|

DWORD WINAPI ThreadBoard(LPVOID data);

DWORD WINAPI ThreadGetClients(LPVOID data);

DWORD WINAPI ThreadClient(LPVOID data);

DWORD WINAPI ThreadPause(LPVOID data);

//|==============================================================================================|
//|===============================| Comunicacao Cliente -> Bolsa |===============================|
//|==============================================================================================|

BOOL GerePedidos(
	TD_WRAPPER* threadData,
	const DWORD codigo);

BOOL GetLogin(
	const TD_WRAPPER* threadData,
	_LOGIN* login);

BOOL GetCompra(
	const TD_WRAPPER* threadData,
	COMPRA* compra);

BOOL GetVenda(
	const TD_WRAPPER* threadData,
	VENDA* venda);

//|========================================================================================|
//|===============================| Validação de operações |===============================|
//|========================================================================================|

BOOL ValidaLogin(
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

BOOL SendRespostaLogin(
	const TD_WRAPPER* threadData,
	const RESPOSTA_LOGIN r_login);

BOOL SendRespostaLista(
	const TD_WRAPPER* threadData,
	RESPOSTA_LISTA r_lista);

void MakeRespostaLista(
	const EMPRESA empresa, 
	const DWORD index, 
	TCHAR buffer[]);

BOOL SendRespostaCompra(
	const TD_WRAPPER* threadData,
	RESPOSTA_COMPRA r_compra);

BOOL SendRespostaVenda(
	const TD_WRAPPER* threadData,
	RESPOSTA_VENDA r_venda);

BOOL SendRespostaBalance(
	const TD_WRAPPER* threadData,
	const RESPOSTA_BALANCE r_balance);

BOOL SendAvisoLogin(const TD_WRAPPER* threadData);

BOOL SendAvisoPause(const TD_WRAPPER* threadData);

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

BOOL PAUSE(
	const CMD comando, 
	TDATA_BOLSA* threadData);

BOOL CLOSE(TDATA_BOLSA* threadData);

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

DWORD GetNCLIENTES();

EMPRESA* AlocaEmpresas();

void InicializaEmpresas(EMPRESA* empresas);

USER* AlocaUsers();

void InicializaUsers(USER* users);

int ComparaEmpresas(
	const void* a, 
	const void* b);

DWORD GetHandlePipeLivre(HANDLE hPipes[NCLIENTES]);

USER* GetPtrToUser(const TCHAR* Nome, TDATA_BOLSA* dados);
