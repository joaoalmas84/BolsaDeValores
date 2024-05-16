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
	HANDLE hEv_ov;
	OVERLAPPED ov;

	DWORD nbytes, err, codigo;

	BOOL continua = TRUE;

	_tprintf_s(_T("\n[CLEINTE] ThreadRead started..."));

	EnterCriticalSection(ptd->pCs);
	hPipe = ptd->hPipe;
	LeaveCriticalSection(ptd->pCs);

	hEv_ov = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (hEv_ov == NULL) {
		PrintErrorMsg(GetLastError(), _T("CreateEvent"));
		exit(-1);
	}

	ZeroMemory(&ov, sizeof(OVERLAPPED));
	ov.hEvent = hEv_ov;

	while (1) {
		if (!ReadFile(hPipe, &codigo, sizeof(DWORD), &nbytes, &ov)) {
			err = GetLastError();
			if (err == ERROR_IO_PENDING) {
				WaitForSingleObject(hEv_ov, INFINITE);
				if (!GetOverlappedResult(hPipe, &ov, &nbytes, FALSE)) {
					err = GetLastError();
					if (err == ERROR_PIPE_NOT_CONNECTED) {
						_tprintf(TEXT("\n[CLIENTE] O servidor foi encerrado...\n"));
						break;
					} else {
						PrintErrorMsg(err, _T("GetOverlappedResult"));
						exit(-1);
					}
				}
				ResetEvent(hEv_ov);
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
			_tprintf_s(_T("\n[CLIENTE] A Bolsa desligou-se..."));
			EnterCriticalSection(ptd->pCs);
			continua = *ptd->continua;
			LeaveCriticalSection(ptd->pCs);
		}
	}

	ExitThread(6);
}

//|==============================================================================================|
//|===============================| Comunicacao Bolsa -> Cliente |===============================|
//|==============================================================================================|

BOOL GereRespostas(const DWORD codigo, TDATA_CLIENTE* threadData) {
	switch (codigo) {
		case R_LOGIN:
			GetRespostaLogin(threadData->hPipe) ? _tprintf_s(_T("\n[CLIENTE] Login success...")) : _tprintf_s(_T("\n[CLIENTE] Login NOT success..."));
			break;
		case R_LISTA:
			break;
		case R_COMPRA:
			break;
		case R_VENDA:
			break;
		case R_BALANCE:
			break;
		case R_CLOSE:
			return FALSE;
			break;
		default:
			break;
	}
	return TRUE;
}

BOOL GetRespostaLogin(const HANDLE hPipe) {
	BOOL resultado;
	DWORD nbytes;

	if (!ReadFile(hPipe, &resultado, sizeof(BOOL), &nbytes, NULL)) {
		PrintErrorMsg(GetLastError(), _T("[CLIENTE] - ReadFile"));
		return FALSE;
	}

	return resultado;

}

//|==========================================================================|
//|===============================| Comandos |===============================|
//|==========================================================================|

BOOL ExecutaComando(const CMD comando, TDATA_CLIENTE* ptd) {

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
			if (!LOGIN(comando, *ptd)) { return FALSE; }
			break;

		case 1:
			if (!LISTC(comando, *ptd)) { return FALSE; }
			break;

		case 2:
			if (!BUY(comando, *ptd)) { return FALSE; }
			break;

		case 3:
			if (!SELL(comando, *ptd)) { return FALSE; }
			break;

		case 4:
			if (!BALANCE(comando, *ptd)) { return FALSE; }
			break;

		case 5:
			if (!EXIT(comando, *ptd)) { return FALSE; }

			EnterCriticalSection(ptd->pCs);
			ptd->continua = FALSE;
			LeaveCriticalSection(ptd->pCs);

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

BOOL EXIT(const CMD comando, const TDATA_CLIENTE threadData) {
	DWORD err, nbytes, codigo = P_EXIT;

	if (!WriteFile(threadData.hPipe, &codigo, sizeof(DWORD), &nbytes, NULL)) {
		err = GetLastError();
		if (err == ERROR_PIPE_NOT_CONNECTED) {
			return FALSE;
		} else {
			PrintErrorMsg(err, _T("WriteFile"));
			exit(-1);
		}
	}
	_tprintf_s(_T("\n[CLIENTE] Enviei codigo = P_EXIT (%d bytes)..."), nbytes);

	return TRUE;
}