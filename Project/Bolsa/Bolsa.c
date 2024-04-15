#include <Windows.h>
#include <tchar.h>
#include <stdio.h>

#include "Bolsa.h"
#include "Commands.h"

//|=========================================================================|
//|===============================| Threads |===============================|
//|=========================================================================|

DWORD WINAPI ThreadBoard(LPVOID data) {
	TDATA_BOLSA* info = (TDATA_BOLSA*)data;
	HANDLE hMap, hEvent;
	SHM* dadosMemoria;

	hEvent = CreateEvent(NULL, TRUE, FALSE, NOME_DO_EVENTO_PARA_AVISAR_BOARD);
	if (hEvent == NULL) {
		_tprintf(_T("Erro ao criar o evento. C�digo de erro: %d\n"), GetLastError());
		return 1;
	}

	hMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(SHM), NOME_DO_FILE_MEMORIA_VIRTUAL);
	if (hMap == NULL) {
		_tprintf_s(_T("ERRO NO MAP"));
		CloseHandle(hEvent);
		return 0;
	}

	dadosMemoria = (SHM*)MapViewOfFile(hMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
	if (dadosMemoria == NULL) {
		_tprintf_s(_T("ERRO AO MAPEAR A MEM�RIA"));
		CloseHandle(hMap);
		CloseHandle(hEvent);
		return 0;
	}
	dadosMemoria->continuar = TRUE;
	while (info->continua) {
		for (DWORD i = 0; i < *(info->numEmpresas); i++) {
			dadosMemoria->empresas[i] = info->empresas[i];
			//dadosMemoria->empresas[i] = info->empresas[i];
			//dadosMemoria->numEmpresas = *(info->numEmpresas);
			//_tprintf(_T("Nome: %s, Pre�o: %.2lf, N�mero de A��es: %u\n"), dadosMemoria->empresas[i].nome, dadosMemoria->empresas[i].preco, dadosMemoria->empresas[i].numDeAcao);
		}

		if (!SetEvent(hEvent)) {
			_tprintf(_T("Erro ao sinalizar o evento. C�digo de erro: %d\n"), GetLastError());
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

//|==========================================================================|
//|===============================| Comandos |===============================|
//|==========================================================================|

void ExecutaComando(const CMD comando, EMPRESA* empresas, DWORD* numEmpresas, CARTEIRA* users, DWORD* numUsers, TDATA_BOLSA* threadData, HANDLE hThread) {

	switch (comando.Index) {
		case 0:
			ADDC(comando, empresas, numEmpresas);
			break;
		case 1:
			LISTC(empresas, *numEmpresas);
			break;
		case 2:
			STOCK(empresas, *numEmpresas, comando.Args[1], comando.Args[2]);
			break;
		case 3:
			USERS(users, *numUsers);
			break;
		case 4:
			PAUSE();
			break;
		case 5:
			CLOSE(threadData, hThread);
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

void ADDC(const CMD comando, EMPRESA* empresas, DWORD* numEmpresas) {
	// passar para os numero e nao str
	DOUBLE preco = 0.0;
	DWORD numAcoes = _ttoi(comando.Args[2]);

	if (*numEmpresas >= NUMERO_MAX_DE_EMPRESAS) {
		_tprintf(_T("O número máximo de empresas já foi atingido\n"));
		return;
	}

	if (IsInteger(comando.Args[3])) { preco = _ttoi(comando.Args[3]); }
	else if (IsFloat(comando.Args[3])) { preco = _tcstod(comando.Args[3], NULL); }

	else { return; }
	// ver se ja existe 
	for (DWORD i = 0; i < *numEmpresas; i++) {
		if (_tcscmp(empresas[i].nome, comando.Args[1]) == 0) {
			_tprintf(_T("Empresa ja existe\n"));
			return;
		}
	}
	// criar 
	// proteger com mutex ou outra coisa.
	_tcscpy_s(empresas[*numEmpresas].nome, TAM_TEXT, comando.Args[1]);
	empresas[*numEmpresas].numDeAcao = numAcoes;
	empresas[*numEmpresas].preco = preco;
	(*numEmpresas)++;
	// proteger com mutex ou outra coisa.
}

void LISTC(const EMPRESA* empresas, DWORD numDeEmpresas) {
	// Proteger isto como um mutex
	for (DWORD i = 0; i < numDeEmpresas; i++) {
		_tprintf(_T("Empresa %d :\n\tNome: %s\n\tN�mero De A��o: %d\n\tPre�o: %.3lf \n"),
			i,
			empresas[i].nome,
			empresas[i].numDeAcao,
			empresas[i].preco);
	}
	// Proteger isto como um mutex
}

void STOCK(EMPRESA* empresas, DWORD numDeEmpresas, const TCHAR* nomeDaEmpresa, const TCHAR* strPreco) {
	DOUBLE preco = 0.0;

	if (IsInteger(strPreco)) { preco = _ttoi(strPreco); }
	else if (IsFloat(strPreco)) { preco = _tcstod(strPreco, NULL); }
	else { return; }

	for (DWORD i = 0; i < numDeEmpresas; i++) {
		if (_tcscmp(empresas[i].nome, nomeDaEmpresa) == 0) {
			empresas[i].preco = preco;
			return;
		}
	}

	_tprintf(_T("O nome %s nao foi encontrado\n"), nomeDaEmpresa);
}

void USERS(const CARTEIRA* users, DWORD numUsers) {
	for (DWORD i = 0; i < numUsers; i++) {
		_tprintf(_T("Cliente %d :\n\tNome: %s\n\tsaldo: %.2lf\n"),
			i, users[i].user.nome, users[i].user.saldo);

		if (users[i].user.ligado) {
			_tprintf(_T("\tEstado: lidado\n"));
		} else {
			_tprintf(_T("\tEstado: deslidado\n"));
		}
	}
}

void PAUSE() {

}

void CLOSE(TDATA_BOLSA* threadData, HANDLE hThread) {
	threadData->continua = FALSE;
	WaitForSingleObject(hThread, INFINITE);
}

//|====================================================================================|
//|===============================| Ficheiros de Dados |===============================|
//|====================================================================================|

void LerEmpresasDoArquivo(EMPRESA* empresas, DWORD* numEmpresas) {
	FILE* arquivo;
	wchar_t linha[TAM_TEXT * 3]; // Tamanho m�ximo de uma linha no arquivo
	wchar_t nome[TAM_TEXT];
	DWORD numAcoes;
	DOUBLE preco;

	if (_wfopen_s(&arquivo, FILE_DADOS_EMPRESAS, _T("r, ccs=UTF-8")) != 0) {
		_tprintf(_T("Erro ao abrir o arquivo.\n"));
		return;
	}

	while (fgetws(linha, sizeof(linha), arquivo) != NULL) {
		if (*numEmpresas >= NUMERO_MAX_DE_EMPRESAS) {
			_tprintf(_T("N�mero m�ximo de empresas atingido.\n"));
			break;
		}

		if (_stscanf_s(linha, _T("%s %d %lf"), nome, TAM_TEXT - 1, &numAcoes, &preco) == 3) {
			nome[_tcsclen(nome)] = '\0';
			_tcscpy_s(empresas[*numEmpresas].nome, TAM_TEXT, nome);
			empresas[*numEmpresas].numDeAcao = numAcoes;
			empresas[*numEmpresas].preco = preco;
			(*numEmpresas)++;

		} else { _tprintf(_T("Erro ao ler a linha.\n")); }
	}
	fclose(arquivo);
}

void SalvarEmpresasNoArquivo(EMPRESA* empresas, DWORD numEmpresas) {
	FILE* arquivo;

	if (_wfopen_s(&arquivo, FILE_DADOS_EMPRESAS, _T("w, ccs=UTF-8")) != 0) {
		_tprintf_s(_T("Erro ao abrir o arquivo para escrita.\n"));
		return;
	}

	for (DWORD i = 0; i < numEmpresas; i++) {
		fwprintf(arquivo, _T("%ls %lu %.2lf\n"), empresas[i].nome, empresas[i].numDeAcao, empresas[i].preco);
	}

	fclose(arquivo);
}

void LerUsersDoArquivo(CARTEIRA* users, DWORD* numUsers) {
	FILE* arquivo;
	TCHAR linha[TAM_TEXT * 3];
	TCHAR nome[TAM_TEXT];
	TCHAR pass[TAM_TEXT];
	DOUBLE saldo;

	if (_wfopen_s(&arquivo, FILE_DADOS_USERS, _T("r, ccs=UTF-8")) != 0) {
		_tprintf(_T("Erro ao abrir o arquivo.\n"));
		return;
	}

	while (fgetws(linha, sizeof(linha), arquivo) != NULL) {
		if (*numUsers >= NUMERO_MAX_DE_USERS) {
			_tprintf_s(_T("N�mero m�ximo de usu�rios atingido.\n"));
			break;
		}

		if (_stscanf_s(linha, _T("%ls %ls %lf"), nome, TAM_TEXT, pass, TAM_TEXT, &saldo) == 3) {
			_tcscpy_s(users[*numUsers].user.nome, TAM_TEXT, nome);
			_tcscpy_s(users[*numUsers].user.pass, TAM_TEXT, pass);
			users[*numUsers].user.saldo = saldo;
			users[*numUsers].numEmpresasNaCarteira = 0;
			users[*numUsers].user.ligado = FALSE;
			(*numUsers)++;
		} else {
			_tprintf_s(_T("Erro ao ler a linha.\n"));
		}
	}

	fclose(arquivo);
}

void SalvarUsersNoArquivo(CARTEIRA* users, DWORD numUsers) {
	FILE* arquivo;

	if (_wfopen_s(&arquivo, FILE_DADOS_USERS, _T("w, ccs=UTF-8")) != 0) {
		_tprintf_s(_T("Erro ao abrir o arquivo para escrita.\n"));
		return;
	}

	for (DWORD i = 0; i < numUsers; i++) {
		fwprintf_s(arquivo, _T("%ls %ls %.2lf\n"), 
			users[i].user.nome, users[i].user.pass, users[i].user.saldo);
	}

	fclose(arquivo);
}

//|========================================================================|
//|===============================| Outras |===============================|
//|========================================================================|

DWORD getNCLIENTES() {
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

			} else {
				PrintError(GetLastError(), _T("Erro no RegQueryValueEx"));
				RegCloseKey(chave);
				return -1;
			}

		} else {
			_tprintf_s(_T("\nTipo de dados invalido o Registry foi alterado: %d "), queryResult);
			queryResult = ERROR_FILE_NOT_FOUND;
		}
	}
	if (queryResult == ERROR_FILE_NOT_FOUND) {
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