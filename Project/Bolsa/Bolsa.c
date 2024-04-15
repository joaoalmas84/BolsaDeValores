#include <Windows.h>
#include <tchar.h>

#include "Bolsa.h"
#include "Commands.h"

void ExecutaComando(const CMD comando) {


	// NAO FAZ sentido isto estar aqui isto nao e poo!!!
	/*switch (comando.Index) {
		case 0:
			ADDC();
			break;
		case 1:
			//LISTC();
			break;
		case 2:
			//STOCK();
			break;
		case 3:
			//USERS();
			break;
		case 4:
			PAUSE();
			break;
		case 5:
			CLOSE();
			break;
	}*/
	// NAO FAZ sentido isto caralho !!!

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

void ADDC(EMPRESA* empresas, DWORD* numDeEmpresas, const CMD* comando) {
	// ver se ha espaco
	if (*numDeEmpresas >= NUMERO_INICIAL_DE_EMPRESAS) {
		_tprintf(_T("Numero de empresas ja esta no maximo\n"));
		return;
	}
	
	// passar para os numero e nao str
	DOUBLE preco = 0.0;
	DWORD numAcoes = _ttoi(comando->Args[2]);
	if (IsInteger(comando->Args[3])) { preco = _ttoi(comando->Args[3]); }
	else if (IsFloat(comando->Args[3])) { preco = _tcstod(comando->Args[3], NULL); }
	else { return; }
	// ver se ja existe 
	for (DWORD i = 0; i < *numDeEmpresas; i++) {
		if (_tcscmp(empresas[i].nome, comando->Args[1]) == 0) {
			_tprintf(_T("Empresa ja existe\n"));
			return;
		}
	}
	// criar 
	// proteger com mutex ou outra coisa.
	_tcscpy_s(empresas[*numDeEmpresas].nome, TAM_TEXT * sizeof(TCHAR), comando->Args[1]);
	empresas[*numDeEmpresas].numDeAcao = numAcoes;
	empresas[*numDeEmpresas].preco = preco;
	(*numDeEmpresas)++;
	// proteger com mutex ou outra coisa.

}

void LISTC(const EMPRESA *empresas, DWORD numDeEmpresas) {

	// Proteger isto como uma seção crítica
	for (DWORD i = 0; i < numDeEmpresas; i++) {
		_tprintf(_T("Empresa %d :\n\tNome: %s\n\tNúmero De Ação: %d\n\tPreço: %.3lf \n"), 
			i, 
			empresas[i].nome, 
			empresas[i].numDeAcao, 
			empresas[i].preco);
	}
	// Proteger isto como uma seção crítica
}

void STOCK(EMPRESA* empresas, DWORD numDeEmpresas, const TCHAR* nomeDaEmpresa, const TCHAR* strPreco) {
	DOUBLE preco = 0.0;

	if (IsInteger(strPreco)) {preco = _ttoi(strPreco);}
	else if (IsFloat(strPreco)) {preco = _tcstod(strPreco, NULL);}
	else {return;}

	for (DWORD i = 0; i < numDeEmpresas; i++) {
		if (_tcscmp(empresas[i].nome, nomeDaEmpresa) == 0) {
			empresas[i].preco = preco;
			return;
		}
	}

	_tprintf(_T("O nome %s nao foi encontrado\n"), nomeDaEmpresa);

}

void USERS(const CARTEIRA_DE_ACOES* useres, DWORD numDeUseres) {
	for (DWORD i = 0; i < numDeUseres; i++) {
		_tprintf(_T("Cliente %d :\n\tNome: %s\n\tsaldo: %.2lf\n"),
			i,
			useres[i].user.nome,
			useres[i].user.saldo);
		if (useres[i].user.ligado) {
			_tprintf(_T("\tEstado: lidado\n"));
		}else {
			_tprintf(_T("\tEstado: deslidado\n"));
		}
		
	}
}

void PAUSE() {

}

void CLOSE(dadosDaThreadDeMemoria* infoThreadMemoria, HANDLE hthreadMemoria) {
	infoThreadMemoria->continua = FALSE;
	WaitForSingleObject(hthreadMemoria, INFINITE);
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

void LerEmpresasDoArquivo(EMPRESA* empresas, DWORD* quantidade) {
	FILE* arquivo;
	wchar_t linha[TAM_TEXT * 3]; // Tamanho máximo de uma linha no arquivo
	wchar_t nome[TAM_TEXT];
	DWORD numDeAcao;
	DOUBLE preco;

	if (_wfopen_s(&arquivo, FILE_DADOS_BESA_EMPRESAS, _T("r, ccs=UTF-8")) != 0) {
		_tprintf(_T("Erro ao abrir o arquivo.\n"));
		return;
	}

	while (fgetws(linha, sizeof(linha), arquivo) != NULL) {
		if (*quantidade >= NUMERO_INICIAL_DE_EMPRESAS) {
			_tprintf(_T("Número máximo de empresas atingido.\n"));
			break;
		}

		if (_stscanf_s(linha, _T("%s %d %lf"), nome, TAM_TEXT - 1, &numDeAcao, &preco) == 3) {
			nome[_tcsclen(nome)] = '\0';
			_tcscpy_s(empresas[*quantidade].nome, TAM_TEXT, nome);
			empresas[*quantidade].numDeAcao = numDeAcao;
			empresas[*quantidade].preco = preco;
			(*quantidade)++;

		}
		else {
			_tprintf(_T("Erro ao ler a linha.\n"));
		}
	}
	fclose(arquivo);
}

void LerUseresDoArquivo(CARTEIRA_DE_ACOES* useres, DWORD* quantidade) {
	FILE* arquivo;
	TCHAR linha[TAM_TEXT * 3];
	TCHAR nome[TAM_TEXT];
	TCHAR pass[TAM_TEXT];
	DOUBLE saldo;

	if (_wfopen_s(&arquivo, FILE_DADOS_BESA_USERES, _T("r, ccs=UTF-8")) != 0) {
		_tprintf(_T("Erro ao abrir o arquivo.\n"));
		return;
	}

	while (fgetws(linha, sizeof(linha), arquivo) != NULL) {
		if (*quantidade >= NUMERO_INICIAL_DE_USERES) {
			_tprintf_s(_T("Número máximo de usuários atingido.\n"));
			break;
		}

		if (_stscanf_s(linha, _T("%ls %ls %lf"), nome, TAM_TEXT, pass, TAM_TEXT, &saldo) == 3) {
			_tcscpy_s(useres[*quantidade].user.nome, TAM_TEXT, nome);
			_tcscpy_s(useres[*quantidade].user.pass, TAM_TEXT, pass);
			useres[*quantidade].user.saldo = saldo;
			useres[*quantidade].numdeEmpresasNaCarteira = 0;
			useres[*quantidade].user.ligado = FALSE;
			(*quantidade)++;
		}else {
			_tprintf_s(_T("Erro ao ler a linha.\n"));
		}
	}

	fclose(arquivo);
}

void SalvarEmpresasNoArquivo(EMPRESA* empresas, DWORD quantidade) {
	FILE* arquivo;

	if (_wfopen_s(&arquivo, FILE_DADOS_BESA_EMPRESAS, _T("w, ccs=UTF-8")) != 0) {
		_tprintf_s(_T("Erro ao abrir o arquivo para escrita.\n"));
		return;
	}

	for (int i = 0; i < quantidade; i++) {
		fwprintf(arquivo, _T("%ls %lu %.2lf\n"), empresas[i].nome, empresas[i].numDeAcao, empresas[i].preco);
	}

	fclose(arquivo);
}

void SalvarUseresNoArquivo(CARTEIRA_DE_ACOES* useres, DWORD quantidade) {
	FILE* arquivo;

	if (_wfopen_s(&arquivo, FILE_DADOS_BESA_USERES, _T("w, ccs=UTF-8")) != 0) {
		_tprintf_s(_T("Erro ao abrir o arquivo para escrita.\n"));
		return;
	}

	for (int i = 0; i < quantidade; i++) {
		fwprintf_s(arquivo, _T("%ls %ls %.2lf\n"), useres[i].user.nome, useres[i].user.pass, useres[i].user.saldo);
	}

	fclose(arquivo);
}

DWORD WINAPI ThreadMemoria(LPVOID lpParam) {
	dadosDaThreadDeMemoria* info = (dadosDaThreadDeMemoria*)lpParam;
	HANDLE hMap, hEvent;
	STRUCT_MEMORIA_VIRTUAL* dadosMemoria;

	hEvent = CreateEvent(NULL, TRUE, FALSE, NOME_DO_EVENTO_PARA_AVISAR_BOARD);
	if (hEvent == NULL) {
		_tprintf(_T("Erro ao criar o evento. Código de erro: %d\n"), GetLastError());
		return 1;
	}

	hMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(STRUCT_MEMORIA_VIRTUAL), NOME_DO_FILE_MEMORIA_VIRTUAL);
	if (hMap == NULL) {
		_tprintf_s(_T("ERRO NO MAP"));
		CloseHandle(hEvent);
		return 0;
	}

	dadosMemoria = (STRUCT_MEMORIA_VIRTUAL*)MapViewOfFile(hMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
	if (dadosMemoria == NULL) {
		_tprintf_s(_T("ERRO AO MAPEAR A MEMÓRIA"));
		CloseHandle(hMap);
		CloseHandle(hEvent);
		return 0;
	}
	dadosMemoria->continuar = TRUE;
	while (info->continua) {
		for (DWORD i = 0; i < *(info->numDeEmpresas); i++) {

			dadosMemoria->empresas[i] = info->empresas[i];
			dadosMemoria->numDeEmpresasExistentes = *(info->numDeEmpresas);
			//_tprintf(_T("Nome: %s, Preço: %.2lf, Número de Ações: %u\n"), dadosMemoria->empresas[i].nome, dadosMemoria->empresas[i].preco, dadosMemoria->empresas[i].numDeAcao);
		}

		if (!SetEvent(hEvent)) {
			_tprintf(_T("Erro ao sinalizar o evento. Código de erro: %d\n"), GetLastError());
			CloseHandle(hEvent);
			return 1;
		}
		ResetEvent(hEvent);

		Sleep(1000);
	}

	dadosMemoria->continuar = FALSE;
	SetEvent(hEvent);


	

	// Libera recursos
	UnmapViewOfFile(dadosMemoria);
	CloseHandle(hMap);
	CloseHandle(hEvent);
	ExitThread(0);
}