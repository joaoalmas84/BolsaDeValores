#include "BoardGUI.h"
#include "strsafe.h"

// Função para tratar os eventos do dialogBox de informação sobre nos criadores!
INT_PTR CALLBACK TrataEventosInfo(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam) {
	// Trata as mensagens da dialogBox.
	switch (messg)
	{
	case WM_CLOSE:
	case WM_COMMAND:
		EndDialog(hWnd, 0);
		break;
	}
	return 0;
}
// Função para tratar os eventos do dialogBox para mudar o valor de x no gráfico.
INT_PTR CALLBACK TrataEventosMAXgrafico(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam) {
	// Vai buscar a informação que está na janela principal.
	DATA_GLOBAL* dadosPartilhados = (DATA_GLOBAL*)GetWindowLongPtr(GetParent(hWnd), GWLP_USERDATA);
	// Trata as mensagens da dialogBox.
	switch (messg)
	{
	case WM_CLOSE:
		EndDialog(hWnd, 0);
		return 1;
		break;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK) {
			// Vai buscar o valor que o usuário colocou na dialogbox e verificar 
			// se está dentro do intervalo de valor do gráfico de 0 até o max_dword.
			dadosPartilhados->valorGrafico = intervalo(MAX_DWORD, 0, getNumberOfBox(hWnd, IDC_EDIT1, &dadosPartilhados->valorGrafico));

			EndDialog(hWnd, 0);
			return 1;
		}
		else if (LOWORD(wParam) == IDCANCEL) {
			EndDialog(hWnd, 0);
			return 1;
		}

		break;
	}
	return 0;
}
// Obter o valor de um item do dialogBox.
DWORD getNumberOfBox(HWND hWnd, int ID, DWORD* var) {
	TCHAR buf[50];
	TCHAR* endPtr;

	GetWindowText(GetDlgItem(hWnd, ID), buf, lstrlen(buf));
	return wcstoul(buf, &endPtr, 10);

}
// Estabelecer um intervalo para o valor enviado
DWORD intervalo(DWORD MAX, DWORD MIN, DWORD var) {
	if (var <= MIN) { return MIN; }
	if (var >= MAX) { return MAX; }
	return var;
}
// Função para tratar os eventos do dialogBox para mudar o valor de y no gráfico, número de empresas.
INT_PTR CALLBACK TrataEventosNumero(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam) {
	// Vai buscar a informação que está na janela principal.
	DATA_GLOBAL* dadosPartilhados = (DATA_GLOBAL*)GetWindowLongPtr(GetParent(hWnd), GWLP_USERDATA);
	switch (messg)
	{
	case WM_CLOSE:
		EndDialog(hWnd, 0);
		return 1;
		break;

	case WM_COMMAND:

		if (LOWORD(wParam) == IDOK) {

			// Vai buscar o valor que o usuário colocou na dialogbox e verificar se está dentro do 
			// intervalo de valor do gráfico de 0 até o max_empresas.
			dadosPartilhados->NumEmpresas = intervalo(MAX_EMPRESAS_BoardGUI, MINIMO_EMPRESAS, getNumberOfBox(hWnd, IDC_EDIT2, &dadosPartilhados->NumEmpresas));
			EndDialog(hWnd, 0);
			return 1;
		}
		else if (LOWORD(wParam) == IDCANCEL) {
			EndDialog(hWnd, 0);
			return 1;
		}
		break;
	}
	return 0;
}
// Função para tratar os eventos da janela principal.
LRESULT CALLBACK TrataEventos(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam) {
	HDC hdc;
	PAINTSTRUCT ps;
	RECT rect;
	// Tem que ser estático ou colocado no contexto do struct DATA_GLOBAL 
	// para poder buscar a informação da janela principal e não perder os dados. 
	// Decidimos por estático mesmo que não seja o melhor.
	static data_thread data;
	static HBITMAP hBmp[2];
	static HDC bmpDC[2];
	static BITMAP bmp[2];
	static DWORD xBitmap, yBitmap;

	// Vai buscar a informação que está na janela principal.
	DATA_GLOBAL* dadosPartilhados; //= (DATA_GLOBAL*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

	
	switch (messg) {
		// Inicia todas as structs e threads necessárias antes da abertura da janela.
	case WM_CREATE: {
		// Iniciar DATA_GLOBAL como valores defult.
		if (!Pdefult_DATA_GLOBAL(&dadosPartilhados)) {
			exit(1);
		}
			
		// Iniciar as imagens necessárias.
		hBmp[0] = (HBITMAP)LoadImage(NULL, TEXT("grafico.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		GetObject(hBmp[0], sizeof(bmp[0]), &bmp[0]);

		hBmp[1] = (HBITMAP)LoadImage(NULL, TEXT("barras.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		GetObject(hBmp[1], sizeof(bmp[1]), &bmp[1]);

		hdc = GetDC(hWnd);
		bmpDC[0] = CreateCompatibleDC(hdc);
		bmpDC[1] = CreateCompatibleDC(hdc);
		SelectObject(bmpDC[0], hBmp[0]);
		SelectObject(bmpDC[1], hBmp[1]);

		ReleaseDC(hWnd, hdc);


		// Verificar o tamanho da janela para ser responsivo e dividir para colocar 
		// a imagem do gráfico no meio e das barras.		GetClientRect(hWnd, &rect);
		xBitmap = (rect.right / 2) - (bmp[0].bmWidth / 2);
		yBitmap = (rect.bottom / 2) - (bmp[0].bmHeight / 2);


		// O menu não é necessário.
		EnableMenuItem(GetMenu(hWnd), ID_INFORMATIONABOUT, MF_ENABLED);
		EnableMenuItem(GetMenu(hWnd), ID_SETSCALE, MF_ENABLED);
		EnableMenuItem(GetMenu(hWnd), ID_CHANGECOMPANYNUMBER, MF_ENABLED);
		EnableMenuItem(GetMenu(hWnd), ID_CLOSE, MF_ENABLED);


		// Iniciar a informação que vai para a thread e criar a thread, claro.
		data;
		data.hEvent_Exit = &dadosPartilhados->hEvent_Exit;
		data.continua = &dadosPartilhados->continua;
		data.hWnd = hWnd;
		data.cs = &dadosPartilhados->cs;
		data.sharedMemory = &(dadosPartilhados->sharedMemory);
		dadosPartilhados->hThread = CreateThread(NULL, 0, ThreadMemoria, &data, 0, NULL);
		if (dadosPartilhados->hThread == NULL) {
			exit(1);
		}

		LONG_PTR x = SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)dadosPartilhados);

		break;
	}
	case WM_DESTROY: {
		// Isto é chamado todas as vezes antes de ela ser destruída.
		// Ir buscar info da janela principal.
		dadosPartilhados = (DATA_GLOBAL*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		if (dadosPartilhados == NULL) {
			return 0;
		}

		// Parar a thread da memória compartilhada.
		EnterCriticalSection(&dadosPartilhados->cs);
		dadosPartilhados->continua = FALSE;
		LeaveCriticalSection(&dadosPartilhados->cs);
		// Avisar a thread.
		SetEvent(dadosPartilhados->hEvent_Exit);
		// Esperar por ela.
		WaitForSingleObject(dadosPartilhados->hThread, INFINITE);
		// Deletar as CriticalSection.
		DeleteCriticalSection(&dadosPartilhados->csInterfase);
		DeleteCriticalSection(&dadosPartilhados->cs);
		CloseHandle(dadosPartilhados->hThread);
		CloseHandle(dadosPartilhados->hEvent_Exit);
		// Liberar a memória que é alocada, como o malloc, cujo nome não me lembro.
		free(dadosPartilhados);
		PostQuitMessage(0);
		break;
	}
	case WM_CLOSE: {
		// Manda o evento  para o WM_DESTROY.
		DestroyWindow(hWnd);
		break;
	}
	case WM_COMMAND: {
		// Para os eventos do menu.
		switch (LOWORD(wParam))
		{
		case ID_INFORMATIONABOUT:
			DialogBoxW(NULL, MAKEINTRESOURCE(IDD_DIALOG2), hWnd, TrataEventosInfo);
			break;
		case ID_SETSCALE:
			DialogBoxW(NULL, MAKEINTRESOURCE(IDD_DIALOG3), hWnd, TrataEventosMAXgrafico);
			InvalidateRect(hWnd, NULL, TRUE);
			break;
		case ID_CHANGECOMPANYNUMBER:
			DialogBoxW(NULL, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, TrataEventosNumero);
			InvalidateRect(hWnd, NULL, TRUE);
			break;
		case ID_CLOSE:
			// Manda o evento  para o WM_DESTROY.
			DestroyWindow(hWnd);
			break;
		}
		break;
	}
	case WM_PAINT: {
		// Vai buscar a informação.
		dadosPartilhados = (DATA_GLOBAL*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		if (dadosPartilhados == NULL) {
			return 0;
		}
		// Começar a pintar.
		hdc = BeginPaint(hWnd, &ps);
		TCHAR buf[50];
		// Cor do texto.
		SetTextColor(hdc, RGB(0, 0, 0));
		SetBkMode(hdc, TRANSPARENT);
		GetClientRect(hWnd, &rect);
		// Pôr a imagem do gráfico.
		BitBlt(hdc, xBitmap, yBitmap, bmp[0].bmWidth, bmp[0].bmHeight, bmpDC[0], 0, 0, SRCCOPY);
		// Vai buscar os valores de valorGrafico, NumEmpresas.
		EnterCriticalSection(&dadosPartilhados->csInterfase);
		DWORD valorGrafico = dadosPartilhados->valorGrafico;
		DWORD NumEmpresas = dadosPartilhados->NumEmpresas;
		LeaveCriticalSection(&dadosPartilhados->csInterfase);
		// Imprimir os valores "X" do gráfico.
		for (DOUBLE i = 0, j = 0; i <= valorGrafico && j < 11; i += valorGrafico / 10, j++) {
			swprintf_s(buf, 50, _T("%.0f"), i);
			TextOut(hdc, xBitmap - lstrlen(buf) * 9, (int)(bmp[0].bmHeight + yBitmap - 10 - j * 29), (LPCWSTR)&buf, lstrlen(buf));
		}

		// Serve para as legendas do lado.
		// serve para rodar o texto vi aqui : https://learn.microsoft.com/pt-br/windows/win32/gdi/rotating-lines-of-text
		HGDIOBJ hfnt, hfntPrev;
		PLOGFONT plf = (PLOGFONT)LocalAlloc(LPTR, sizeof(LOGFONT));
		plf->lfWeight = FW_NORMAL;
		plf->lfEscapement = 3150;
		hfnt = CreateFontIndirect(plf);
		hfntPrev = SelectObject(hdc, hfnt);
		// serve para rodar o texto vi aqui : https://learn.microsoft.com/pt-br/windows/win32/gdi/rotating-lines-of-text

		// Pôr as barras do tamanho certo e pôr os valores e nome das empresas.
		DWORD destancia_lateral = xBitmap;
		for (DWORD i = 0; i < NumEmpresas; i++) {
			destancia_lateral += 10;
			EnterCriticalSection(&dadosPartilhados->cs);
			setBarras(hdc, dadosPartilhados, destancia_lateral, yBitmap, bmp[1], bmpDC[1], i, bmp[0], 70);
			LeaveCriticalSection(&dadosPartilhados->cs);
			destancia_lateral += bmp[1].bmWidth;

		}

		// Tirar as letras nome sem estar de lado.
		// serve para rodar o texto vi aqui : https://learn.microsoft.com/pt-br/windows/win32/gdi/rotating-lines-of-text
		SelectObject(hdc, hfntPrev);
		DeleteObject(hfnt);
		LocalFree((LOCALHANDLE)plf);
		// serve para rodar o texto vi aqui : https://learn.microsoft.com/pt-br/windows/win32/gdi/rotating-lines-of-text

		EndPaint(hWnd, &ps);
		break;
	}
	case WM_SIZE: {
		// Pôr a nova informação para o gráfico e mandar o evento para o WM_PAINT.
		xBitmap = (LOWORD(lParam) / 2) - (bmp[0].bmWidth / 2);
		yBitmap = (HIWORD(lParam) / 2) - (bmp[0].bmHeight / 2);
		InvalidateRect(hWnd, NULL, TRUE);
		break;
	}
	default: {
		return(DefWindowProc(hWnd, messg, wParam, lParam));
		break;
	}
	}
	return 0;
}
//Thread para receber informações da memória compartilhada
DWORD WINAPI ThreadMemoria(LPVOID data) {
	// quese a mesma coisa que o board normal
	data_thread* info = (data_thread*)data;

	//int setmodeReturn; // Evitar o warning da funcao setmode();

	DWORD codigoErro;

	//DWORD N; // Numero de empresas a listar (tamanho do top)

	EMPRESA empresas[MAX_EMPRESAS_TO_SHM];
	BOOL continuaMain = TRUE;

	// Variaveis da SharedMemory
	HANDLE hMap;
	HANDLE hMutex_SHM;
	HANDLE hEvent_SHM; // Evento SHM (Reset Manual);

	// Array de Eventos para o WaitForMultipleObjects();
	HANDLE hEvents[2]; // [0]: Evento_SHM; [1]: Evento_Exit


	hMutex_SHM = OpenMutex(SYNCHRONIZE, FALSE, SHM_MUTEX);
	if (hMutex_SHM == NULL) {
		codigoErro = GetLastError();
		if (codigoErro == ERROR_FILE_NOT_FOUND) {
			exit(1);
		}
		else {
			exit(1);
		}
		return 1;
	}

	hMap = OpenFileMapping(FILE_MAP_READ, FALSE, SHARED_MEMORY);
	if (hMap == NULL) {
		CloseHandle(hMutex_SHM);
		exit(1);
	}

	EnterCriticalSection(info->cs);
	(*(*info).sharedMemory) = (SHM*)MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
	if (info->sharedMemory == NULL) {
		CloseHandle(hMutex_SHM);
		CloseHandle(hMap);
		exit(1);
	}
	LeaveCriticalSection(info->cs);

	hEvent_SHM = OpenEvent(SYNCHRONIZE, FALSE, SHM_EVENT);
	if (hEvent_SHM == NULL) {
		CloseHandle(hMutex_SHM);
		CloseHandle(hMap);
		exit(1);
	}

	WaitForSingleObject(hMutex_SHM, INFINITE);
	CopyMemory(empresas, (*(*info).sharedMemory)->empresas, sizeof(EMPRESA) * MAX_EMPRESAS_TO_SHM);
	ReleaseMutex(hMutex_SHM);
	InvalidateRect((info->hWnd), NULL, TRUE);

	hEvents[0] = hEvent_SHM;
	hEvents[1] = *(info->hEvent_Exit);

	while (continuaMain) {


		WaitForMultipleObjects(2, hEvents, FALSE, INFINITE);

		WaitForSingleObject(hMutex_SHM, INFINITE);
		EnterCriticalSection(info->cs);
		CopyMemory(empresas, (*(*info).sharedMemory)->empresas, sizeof(EMPRESA) * MAX_EMPRESAS_TO_SHM);
		continuaMain = *(info->continua);
		LeaveCriticalSection(info->cs);
		ReleaseMutex(hMutex_SHM);

		InvalidateRect((info->hWnd), NULL, TRUE);
	}
	FlushViewOfFile((*(*info).sharedMemory), 0);
	CloseHandle(hMap);
	CloseHandle(hMutex_SHM);
	ExitThread(0);
}
// Função para inicializar a estrutura.
DATA_GLOBAL defult_DATA_GLOBAL() {
	DATA_GLOBAL dados;
	dados.continua = TRUE;
	dados.hEvent_Exit = NULL;
	dados.hThread = NULL;
	dados.sharedMemory = NULL;
	dados.valorGrafico = 3000;
	dados.NumEmpresas = 10;

	if (!InitializeCriticalSectionAndSpinCount(&dados.csInterfase, 0)) {
		exit(1);
	}

	if (!InitializeCriticalSectionAndSpinCount(&dados.cs, 0)) {
		exit(1);
	}

	return dados;
}
//Função para inicializar a estrutura.
BOOL Pdefult_DATA_GLOBAL(DATA_GLOBAL** dadosFora) {
	DATA_GLOBAL* dados = (DATA_GLOBAL*)malloc(sizeof(DATA_GLOBAL));

	dados->continua = TRUE;
	dados->hThread = NULL;
	dados->sharedMemory = NULL;
	dados->valorGrafico = 3000;
	dados->NumEmpresas = 10;

	if (!InitializeCriticalSectionAndSpinCount(&dados->csInterfase, 0)) {
		return FALSE;
	}

	if (!InitializeCriticalSectionAndSpinCount(&dados->cs, 0)) {
		DeleteCriticalSection(&dados->csInterfase);
		return FALSE;
	}

	dados->hEvent_Exit = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (dados->hEvent_Exit == NULL) {
		DeleteCriticalSection(&dados->csInterfase);
		DeleteCriticalSection(&dados->cs);
		return FALSE;
	}

	(*dadosFora) = dados;
	return TRUE;
}
//Função para iniciar as barras com a forma que queremos.
void setBarras(HDC hdc, DATA_GLOBAL* dadosPartilhados, DWORD destancia_lateral, DWORD yBitmap, BITMAP bmp, HDC bmpDC, DWORD i, BITMAP bmpgrafico, DWORD alturaDosPrecos) {
	// ve se tem memoria
	if (dadosPartilhados->sharedMemory == NULL) {
		TCHAR textoDeErro[] = _T("sem dados erro");
		TextOut(hdc, destancia_lateral + bmp.bmWidth / 2, bmpgrafico.bmHeight + yBitmap, textoDeErro, lstrlen(textoDeErro));
	}
	else {
		TCHAR buf[50];
		DOUBLE multiplicador = dadosPartilhados->sharedMemory->empresas[i].preco / dadosPartilhados->valorGrafico;
		if (multiplicador > 1) { multiplicador = 1; }
		DOUBLE remover = bmp.bmHeight * (1 - multiplicador);
		// printa a barra como o tamanho proprocional 
		BitBlt(hdc, destancia_lateral, (int)(yBitmap + remover), bmp.bmWidth, (int)(bmp.bmHeight - remover), bmpDC, 0, 0, SRCCOPY);
		// escreve nome da empresa
		TextOut(hdc, destancia_lateral + bmp.bmWidth / 2, bmpgrafico.bmHeight + yBitmap, dadosPartilhados->sharedMemory->empresas[i].nome, lstrlen(dadosPartilhados->sharedMemory->empresas[i].nome));
		swprintf_s(buf, 50, _T("%.2f"), dadosPartilhados->sharedMemory->empresas[i].preco);
		//escreve preco
		TextOut(hdc, destancia_lateral + bmp.bmWidth / 2, (int)(yBitmap + remover - alturaDosPrecos), buf, lstrlen(buf));
	}
}

