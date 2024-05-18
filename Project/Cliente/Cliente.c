#include <Windows.h>
#include <tchar.h>

#include "Utils.h"
#include "Commands.h"
#include "Structs.h"

#include "Cliente.h"

//|=========================================================================|
//|===============================| Threads |===============================|
//|=========================================================================|

DWORD WINAPI ThreadRead(LPVOID data) {
	TDATA_CLIENTE* ptd = (TDATA_CLIENTE*)data;
	
	HANDLE hPipe;
	OVERLAPPED ov;

	DWORD nbytes, err, codigo;

	BOOL continua = TRUE;

	_tprintf_s(_T("\n[CLEINTE] ThreadRead started..."));

	EnterCriticalSection(ptd->pCs);
	hPipe = ptd->hPipe;
	LeaveCriticalSection(ptd->pCs);

	ptd->hEv_Read = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (ptd->hEv_Read == NULL) {
		PrintErrorMsg(GetLastError(), _T("CreateEvent"));
		exit(-1);
	}

	ZeroMemory(&ov, sizeof(OVERLAPPED));
	ov.hEvent = ptd->hEv_Read;

	while (1) {
		if (!ReadFile(hPipe, &codigo, sizeof(DWORD), &nbytes, &ov)) {
			err = GetLastError();
			if (err == ERROR_IO_PENDING) {
				WaitForSingleObject(ov.hEvent, INFINITE);

				EnterCriticalSection(ptd->pCs);
				continua = *ptd->continua;
				LeaveCriticalSection(ptd->pCs);

				if (!continua) { break; }

				if (!GetOverlappedResult(hPipe, &ov, &nbytes, FALSE)) {
					err = GetLastError();
					 if (err == ERROR_PIPE_NOT_CONNECTED) {
						_tprintf(_T("\n[CLIENTE] A bolsa foi encerrada...\n"));

						EnterCriticalSection(ptd->pCs);
						*ptd->continua = FALSE;
						LeaveCriticalSection(ptd->pCs);

						if (!CancelSynchronousIo(ptd->hThread_Main)) {
							PrintErrorMsg(GetLastError(), _T("CancelSynchronousIo"));
							exit(-1);
						}

						break;
					} else {
						PrintErrorMsg(err, _T("GetOverlappedResult"));
						exit(-1);
					}
				}
				ResetEvent(ov.hEvent);
			} else {
				PrintErrorMsg(err, _T("ReadFile"));
				exit(-1);
			}
		}

		EnterCriticalSection(ptd->pCs);
		continua = *ptd->continua;
		LeaveCriticalSection(ptd->pCs);

		if (!continua) { break; } 

		if (!GereRespostas(codigo, ptd)) { 
			EnterCriticalSection(ptd->pCs);
			continua = *ptd->continua;
			LeaveCriticalSection(ptd->pCs);
		}
	}

	_tprintf_s(_T("\n[CLEINTE] ThreadRead ending..."));

	ExitThread(6);
}

//|==============================================================================================|
//|===============================| Comunicacao Bolsa -> Cliente |===============================|
//|==============================================================================================|

BOOL GereRespostas(const DWORD codigo, TDATA_CLIENTE* threadData) {
	switch (codigo) {
		case R_LOGIN:
			if (GetOperationResult(threadData->hPipe)) {
				_tprintf_s(_T("\n[CLIENTE] Login efetuado com sucesso!"));
			} else {
				_tprintf_s(_T("\n[CLIENTE] Não foi possível fazer login!"));
			}			
			break;

		case R_LISTA:
			_tprintf_s(_T("\n[CLIENTE] RESPOSTA_LISTA..."));
			if (!GetRespostaLista(threadData->hPipe)) { return FALSE; }
			break;

		case R_COMPRA:
			if (GetOperationResult(threadData->hPipe)) {
				_tprintf_s(_T("\n[CLIENTE] Compra efetuada com sucesso!"));
			} else {
				_tprintf_s(_T("\n[CLIENTE] Não foi possível realizar a compra!"));
			}
			break;

		case R_VENDA:
			if (GetOperationResult(threadData->hPipe)) {
				_tprintf_s(_T("\n[CLIENTE] Venda efetuada com sucesso!"));
			} else {
				_tprintf_s(_T("\n[CLIENTE] Não foi possível realizar a venda!"));
			}
			break;

		case R_BALANCE:
			_tprintf_s(_T("\n[CLIENTE] Balance"));
			if (!GetRespostaBalance(threadData->hPipe)) { return FALSE; }

			break;
		case R_AVISO_LOGIN:
			_tprintf_s(_T("\n[CLIENTE] É necessário fazer Login para utilizar esse comando..."));
			break;
		case R_AVISO_PAUSE:
			_tprintf_s(_T("\n[CLIENTE] Bolsa em pausa..."));
			break;
		default:
			break;
	}
	return TRUE;
}

BOOL GetOperationResult(const HANDLE hPipe) {
	BOOL resultado;
	DWORD nbytes;

	if (!ReadFile(hPipe, &resultado, sizeof(BOOL), &nbytes, NULL)) {
		PrintErrorMsg(GetLastError(), _T("[CLIENTE] - ReadFile"));
		return FALSE;
	}

	return resultado;
}

BOOL GetRespostaLista(const HANDLE hPipe) {
	DWORD nbytes, numEmpresas;
	TCHAR buffer[BIG_TEXT];

	if (!ReadFile(hPipe, &numEmpresas, sizeof(DWORD), &nbytes, NULL)) {
		PrintErrorMsg(GetLastError(), _T("[CLIENTE] - ReadFile"));
		return FALSE;
	}
	_tprintf_s(_T("\n[CLIENTE] Recebi numEmpresas: %d (%d nbytes)..."), numEmpresas, nbytes);

	for (DWORD i = 0; i < numEmpresas; i++) {
		if (!ReadFile(hPipe, buffer, sizeof(buffer), &nbytes, NULL)) {
			PrintErrorMsg(GetLastError(), _T("[CLIENTE] - ReadFile"));
			return FALSE;
		}
		_tprintf(_T("%s"), buffer);

	}

	return TRUE;
}

BOOL GetRespostaBalance(const HANDLE hPipe) {
	DWORD nbytes;
	TCHAR buff[SMALL_TEXT];

	if (!ReadFile(hPipe, buff, sizeof(buff), &nbytes, NULL)) {
		PrintErrorMsg(GetLastError(), _T("[CLIENTE] - ReadFile"));
		return FALSE;
	}

	_tprintf(_T("\n[CLIENTE] %s"), buff);

	return TRUE;
}

//|==========================================================================|
//|===============================| Comandos |===============================|
//|==========================================================================|

BOOL ExecutaComando(const CMD comando, TDATA_CLIENTE* threadData) {

	// Apagar depois
	_tprintf_s(_T("\nComando: "));
	_tprintf_s(_T("\nNome: '%s'"), comando.Nome);
	_tprintf_s(_T("\nNumArgs: %d"), comando.NumArgs);
	_tprintf_s(_T("\nIndex: %d"), comando.Index);
	_tprintf_s(_T("\nArgs: "));
	for (DWORD i = 0; i < comando.NumArgs; i++) {
		_tprintf_s(_T("'%s' "), comando.Args[i]);
	}

	switch (comando.Index) {
		case 0:
			if (!LOGIN(comando, *threadData)) { return FALSE; }
			break;

		case 1:
			if (!LISTC(comando, *threadData)) { return FALSE; }
			break;

		case 2:
			if (!BUY(comando, *threadData)) { return FALSE; }
			break;

		case 3:
			if (!SELL(comando, *threadData)) { return FALSE; }
			break;

		case 4:
			if (!BALANCE(comando, *threadData)) { return FALSE; }
			break;

		case 5:
			if (!EXIT(threadData)) { return FALSE; }
			break;

		default:
			return FALSE;
			break;
	}

	return TRUE;
}

BOOL LOGIN(const CMD comando, const TDATA_CLIENTE threadData) {
	DWORD err, nbytes;
	PEDIDO_LOGIN p_login;

	HANDLE hPipe;

	EnterCriticalSection(threadData.pCs);
	hPipe = threadData.hPipe;
	LeaveCriticalSection(threadData.pCs);

	p_login.codigo = P_LOGIN;
	_tcscpy_s(p_login.login.nome, SMALL_TEXT, comando.Args[1]);
	_tcscpy_s(p_login.login.pass, SMALL_TEXT, comando.Args[2]);

	if (!WriteFile(hPipe, &p_login, sizeof(PEDIDO_LOGIN), &nbytes, NULL)) {
		err = GetLastError();
		if (err == ERROR_PIPE_NOT_CONNECTED) {
			_tprintf_s(_T("\n[CLIENTE] Bolsa Desligou..."));
			return FALSE;
		} else {
			PrintErrorMsg(err, _T("WriteFile"));
			exit(-1);
		}
	}
	_tprintf_s(_T("\n[CLIENTE] Enviei PEDIDO_LOGIN (%d bytes)..."), nbytes);

	return TRUE;
}

BOOL LISTC(const CMD comando, const TDATA_CLIENTE threadData) {
	DWORD err, nbytes, codigo = P_LISTA;

	if (!WriteFile(threadData.hPipe, &codigo, sizeof(DWORD), &nbytes, NULL)) {
		err = GetLastError();
		if (err == ERROR_PIPE_NOT_CONNECTED) {
			return FALSE;
		}
		else {
			PrintErrorMsg(err, _T("WriteFile"));
			exit(-1);
		}
	}
	_tprintf_s(_T("\n[CLIENTE] Enviei codigo = P_LISTA (%d bytes)..."), nbytes);

	return TRUE;
}

BOOL BUY(const CMD comando, const TDATA_CLIENTE threadData) {
	DWORD err, nbytes;
	PEDIDO_COMPRA p_compra;

	p_compra.codigo = P_COMPRA;
	_tcscpy_s(p_compra.compra.nomeEmpresa, SMALL_TEXT, comando.Args[1]);
	p_compra.compra.numAcoes = _ttoi(comando.Args[2]);

	if (!WriteFile(threadData.hPipe, &p_compra, sizeof(PEDIDO_COMPRA), &nbytes, NULL)) {
		err = GetLastError();
		if (err == ERROR_PIPE_NOT_CONNECTED) {
			return FALSE;
		} else {
			PrintErrorMsg(err, _T("WriteFile"));
			exit(-1);
		}
	}
	_tprintf_s(_T("\n[CLIENTE] Enviei PEDIDO_COMPRA (%d bytes)..."), nbytes);

	return TRUE;
}

BOOL SELL(const CMD comando, const TDATA_CLIENTE threadData) {
	DWORD err, nbytes;
	PEDIDO_VENDA p_venda;

	p_venda.codigo = P_VENDA;
	_tcscpy_s(p_venda.venda.nomeEmpresa, SMALL_TEXT, comando.Args[1]);
	p_venda.venda.numAcoes = _ttoi(comando.Args[2]);

	if (!WriteFile(threadData.hPipe, &p_venda, sizeof(PEDIDO_VENDA), &nbytes, NULL)) {
		err = GetLastError();
		if (err == ERROR_PIPE_NOT_CONNECTED) {
			return FALSE;
		}
		else {
			PrintErrorMsg(err, _T("WriteFile"));
			exit(-1);
		}
	}
	_tprintf_s(_T("\n[CLIENTE] Enviei PEDIDO_VENDA (%d bytes)..."), nbytes);

	return TRUE;
}

BOOL BALANCE(const CMD comando, const TDATA_CLIENTE threadData) {
	DWORD err, nbytes, codigo = P_BALANCE;

	if (!WriteFile(threadData.hPipe, &codigo, sizeof(DWORD), &nbytes, NULL)) {
		err = GetLastError();
		if (err == ERROR_PIPE_NOT_CONNECTED) {
			return FALSE;
		}
		else {
			PrintErrorMsg(err, _T("WriteFile"));
			exit(-1);
		}
	}
	_tprintf_s(_T("\n[CLIENTE] Enviei codigo = P_BALANCE (%d bytes)..."), nbytes);

	return TRUE;
}

BOOL EXIT(TDATA_CLIENTE* threadData) {

	EnterCriticalSection(threadData->pCs);
	*threadData->continua = FALSE;
	LeaveCriticalSection(threadData->pCs);

	SetEvent(threadData->hEv_Read);

	return TRUE;
}