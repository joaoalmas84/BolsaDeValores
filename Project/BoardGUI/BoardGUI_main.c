#include "BoardGUI.h"

int WINAPI _tWinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPTSTR lpCmdLine, int nCmdShow) {
	HWND hWnd;
	MSG lpMsg;
	WNDCLASSEX wcApp;

	wcApp.cbSize = sizeof(WNDCLASSEX);
	wcApp.hInstance = hInst;
	wcApp.lpszClassName = TEXT("Base");
	wcApp.lpfnWndProc = TrataEventos;
	wcApp.style = CS_HREDRAW | CS_VREDRAW;

	wcApp.hIcon = (HICON)LoadImage(NULL, L"icone.ico", IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
	wcApp.hIconSm = (HICON)LoadImage(NULL, L"icone.ico", IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
	wcApp.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcApp.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);

	wcApp.cbClsExtra = sizeof(DATA_GLOBAL);
	wcApp.cbWndExtra = 0;
	wcApp.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(255, 255, 255));

	if (!RegisterClassEx(&wcApp))
		return(0);

	hWnd = CreateWindow(TEXT("Base"), TEXT("Board - GUI"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, (HWND)HWND_DESKTOP, (HMENU)NULL, (HINSTANCE)hInst, 0);


	ShowWindow(hWnd, nCmdShow);

	UpdateWindow(hWnd);


	while (GetMessage(&lpMsg, NULL, 0, 0) > 0) {
		TranslateMessage(&lpMsg);
		DispatchMessage(&lpMsg);
	}


	return (int)lpMsg.wParam;
}