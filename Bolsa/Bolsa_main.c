#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <fcntl.h> 
#include <io.h>

#include "Bolsa.h"
#include "Commands.h"


int _tmain(int argc, TCHAR* argv[]) {

	int setmodeReturn;
	DWORD numClintes = -1;
	// Buffer para guardar mensagens de erro
	TCHAR msg[TAM];
	// Buffer para guardar o comando recebido
	TCHAR input[TAM];
	// Estrutura comando
	CMD comando;
	TCHAR c;
	//DWORD res;

	//dados das empreas 
	EMPRESA empresas[NUMERO_INICIAL_DE_EMPRESAS];
	DWORD numDeEmpresas = 0; // num existente de empresas

	CARTEIRA_DE_ACOES useres[NUMERO_INICIAL_DE_USERES];
	DWORD numDeUseres = 0; // num existente de carteiras


	dadosDaThreadDeMemoria infoThreadMemoria;
	HANDLE hThreadMemoria;
	DWORD dwThreadMemoriaId;
	infoThreadMemoria.empresas = empresas;
	infoThreadMemoria.numDeEmpresas = &numDeEmpresas;
	infoThreadMemoria.continua = TRUE;

#ifdef UNICODE
	setmodeReturn = _setmode(_fileno(stdin), _O_WTEXT);
	setmodeReturn = _setmode(_fileno(stdout), _O_WTEXT);
	setmodeReturn = _setmode(_fileno(stderr), _O_WTEXT);
#endif 

	if ((numClintes = getNCLIENTES()) == -1) {
		return -1;
	}

	LerEmpresasDoArquivo(empresas, &numDeEmpresas);
	LerUseresDoArquivo(useres, &numDeUseres);

	hThreadMemoria = CreateThread(
		NULL,                   // Atributos de segurança (não especificados neste exemplo)
		0,                      // Tamanho padrão da pilha (não especificado neste exemplo)
		ThreadMemoria,         // Função de entrada da thread
		&infoThreadMemoria,                   // Parâmetro para a função de entrada da thread (não especificado neste exemplo)
		0,                      // Flags de criação (não especificado neste exemplo)
		&dwThreadMemoriaId      // Identificador da thread
	);
	if (hThreadMemoria == NULL) {
		PrintError(GetLastError(), _T("Erro ao criar a thread. Código de erro: %d\n"));
		return 1;
	}

	while (1) {
		GetCmd(input);
		c = _gettchar();

		if (!ValidaCmd(input, &comando, msg, TRUE)) {
			_tprintf(_T("\n[ERRO] %s."), msg);
		}else {
			ExecutaComando(comando, empresas, &numDeEmpresas, useres, &numDeUseres, &infoThreadMemoria, hThreadMemoria);
			if (comando.Index == 5) { break; }
		}
	}

	SalvarEmpresasNoArquivo(empresas, numDeEmpresas);
	SalvarUseresNoArquivo(useres, numDeUseres);
	return 0;
}

