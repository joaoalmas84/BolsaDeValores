#pragma once

#include <Windows.h>
#include <tchar.h>

#include "Commands.h"
#include "Utils.h"
#define NCLIENTES _T("NCLIENTES")
#define STARTING_NUM_OF_NCLIENTES 5 // NUMERO default PARA O NCLIENTES 
#define NUMERO_INICIAL_DE_EMPRESAS 30 // numero inicial para as empresas
#define NUMERO_INICIAL_DE_USERES 20  // numero inicial para as usares
#define FILE_DADOS_BESA_EMPRESAS _T("dadosEmpresas.txt") // nome do ficheiro de emprecas 
#define FILE_DADOS_BESA_USERES _T("dadosUseres.txt")  // nome do ficheiro de useres 


typedef struct{
	BOOL continua;
	EMPRESA* empresas;
	DWORD* numDeEmpresas;
}dadosDaThreadDeMemoria;


void ExecutaComando(const CMD comando);

void ADDC(EMPRESA* empresas, DWORD* numDeEmpresas, const CMD* comando);

void LISTC(const EMPRESA* empresas, DWORD numDeEmpresas);

void STOCK(EMPRESA* empresas, DWORD numDeEmpresas, const TCHAR* nomeDaEmpresa, const TCHAR* preco);

void USERS(const CARTEIRA_DE_ACOES* useres, DWORD numDeUseres);

void PAUSE();

void CLOSE(dadosDaThreadDeMemoria* infoThreadMemoria, HANDLE hthreadMemoria);

DWORD getNCLIENTES();

void LerEmpresasDoArquivo(EMPRESA* empresas, DWORD* numEmpresas);

void SalvarEmpresasNoArquivo(EMPRESA* empresas, DWORD numEmpresas);

void LerUseresDoArquivo(CARTEIRA_DE_ACOES* useres, DWORD* quantidade);

void SalvarUseresNoArquivo(CARTEIRA_DE_ACOES* useres, DWORD quantidade);

DWORD WINAPI ThreadMemoria(LPVOID lpParam);
