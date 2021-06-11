#include <windows.h>
#include <Windowsx.h>
#include <tchar.h>
#include "resource.h"
#include "controller.h"
#include "Winuser.h"

#include "../HF/structs.h"
#include "prepareEnv.h"
#include "textUI.h"
#include "sharedMemory.h"

LRESULT CALLBACK TrataEventos(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK TrataEventosTerminal(HWND, UINT, WPARAM, LPARAM);


TCHAR szProgName[] = TEXT("Ficha8");
pDATA data;

HBITMAP hBmp;
HDC bmpDC;
BITMAP bmp;
int xBitmap;
int yBitmap;

int limDir;
HWND hWndGlobal;
HWND hMutex;

HDC memDC = NULL;
HBITMAP hBitmapDB;

//DWORD WINAPI MovimentaImagem(LPVOID lParam)
//{
//	int dir = 1; //1 para direita -1 para esquerda
//	int salto = 2;
//
//	while (1)
//	{
//		WaitForSingleObject(hMutex, INFINITE);
//
//		xBitmap += dir * salto;
//
//		if (xBitmap <= 0)
//		{
//			xBitmap = 0;
//			dir = 1;
//		}
//		else if (xBitmap >= limDir)
//		{
//			xBitmap = limDir;
//			dir = -1;
//		}
//
//		ReleaseMutex(hMutex);
//
//		InvalidateRect(hWndGlobal, NULL, FALSE);
//		Sleep(1);
//	}
//}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
{
	HWND hWnd;
	MSG lpMsg;
	WNDCLASSEX wcApp;
	//HANDLE hAccel;

	wcApp.cbSize = sizeof(WNDCLASSEX);
	wcApp.hInstance = hInst;

	wcApp.lpszClassName = szProgName;
	wcApp.lpfnWndProc = TrataEventos;

	wcApp.style = CS_HREDRAW | CS_VREDRAW;

	//wcApp.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1));
	//wcApp.hIconSm = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1));
	//wcApp.hCursor = LoadCursor(hInst, MAKEINTRESOURCE(IDC_POINTER));
	wcApp.lpszMenuName = MAKEINTRESOURCE(ID_MENU_PRINCIPAL);
	wcApp.style = CS_HREDRAW | CS_VREDRAW;
	wcApp.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wcApp.hIconSm = LoadIcon(NULL, IDI_INFORMATION);
	wcApp.hCursor = LoadCursor(NULL, IDC_ARROW);

	//wcApp.cbClsExtra = sizeof(dados);
	wcApp.cbClsExtra = 0;
	wcApp.cbWndExtra = 0;
	wcApp.hbrBackground = CreateSolidBrush(RGB(132, 219, 255));

	if (!RegisterClassEx(&wcApp))
		return(0);

	hWnd = CreateWindow(
		szProgName,
		TEXT("SO2 - Controlador A�reo"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		556,
		601
		,
		(HWND)HWND_DESKTOP,
		(HMENU)NULL,
		(HINSTANCE)hInst,
		0);

	/*CoppyPaste do controlador.c* -----------------------------------------------------------------------------------------------------------------------------------------------------------*/
	

	data = malloc(sizeof(DATA));
	if (data == NULL) {
		_ftprintf(stderr, L"Memorry alloc fail for data\n");
		return -1;
	}

	data->nrPassengers = 0;
	data->passengers = malloc(sizeof(Passenger) * MAX_PASSENGERS);
	InitializeCriticalSection(&data->passengersCriticalSection);

	//previne poder ter mais do que uma inst�ncia do mesmo programa a correr em simult�neo.
	CreateMutexA(0, FALSE, "Local\\$controlador$"); // try to create a named mutex
	if (GetLastError() == ERROR_ALREADY_EXISTS) {   // did the mutex already exist?
		//Se tiver tempo fazer mesagebox...
		return -1;								  // quit; mutex is released automatically
	}

	//Coloca as variaveis no registry caso n�o estejam onde � esperado...
	if (!prepareEnvironment(KEY_PATH)) {
		//puts("Registry fail!");
		return -1;
	}

	//Obt�m vari�veis que supostamente est�o no Registry
	data->maxAirplanes = getMax(MAX_AIRPLANES, KEY_PATH);
	data->maxAirports = getMax(MAX_AIRPORTS, KEY_PATH);
	if (!data->maxAirplanes || !data->maxAirports) {
		//_tprintf(L"Registry vars fail\n");
		return -1;
	}

	//estas fun��es internamente fornecem mais informa��es caso algo falhe
	if (!createAirportSpace(data) || !createMap(data) || !createAirplaneSpace(data) || !createProducerConsumer(data)) {
		//_ftprintf(stderr, L"File mapping fail\n");
		return -1;
	}

	_tprintf(L"Max airports do registry :  %d\nMax airplanes do registry : %d\nComando 'help' para mais informa��es.\n\n",
		data->maxAirports, data->maxAirplanes);

	initProducerConsumerThread(data);
	initControlPassengerRegisterThread(data);
	/*FinalCoppyPaste do controlador.c ------------------------------------------------------------------------------------------------------------------------------------------------------------*/

	HDC hdc;
	RECT rect;

	hBmp = (HBITMAP)LoadImage(NULL, _T("control.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	GetObject(hBmp, sizeof(bmp), &bmp);

	hdc = GetDC(hWnd);
	bmpDC = CreateCompatibleDC(hdc);
	SelectObject(bmpDC, hBmp);
	ReleaseDC(hWnd, hdc);

	GetClientRect(hWnd, &rect);
	//xBitmap = (rect.right / 2) - (bmp.bmWidth / 2);
	//yBitmap = (rect.bottom / 2) - (bmp.bmHeight / 2);

	limDir = rect.right - bmp.bmWidth;
	hWndGlobal = hWnd;

	hMutex = CreateMutex(NULL, FALSE, NULL);
	//CreateThread(NULL, 0, MovimentaImagem, NULL, 0, NULL);

	//dadosPartilhados.numOperacoes = 5; // Apenas para testar...
	//LONG_PTR x = SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)&data);

	ShowWindow(hWnd, nCmdShow);

	//hAccel = LoadAccelerators(NULL, MAKEINTRESOURCE(IDR_ACCELERATOR));

	while (GetMessage(&lpMsg, NULL, 0, 0))
	{
		//if (!TranslateAccelerator(hWnd, hAccel, &lpMsg))
		{
			TranslateMessage(&lpMsg);
			DispatchMessage(&lpMsg);
		}
	}

	UnmapViewOfFile(data->producerConsumer);
	CloseHandle(data->objProducerConsumer);
	CloseHandle(data->emptiesSemaphore);
	CloseHandle(data->itemsSemaphore);
	CloseHandle(data->airportsMutex);
	CloseHandle(data->mapMutex);
	UnmapViewOfFile(data->map);
	CloseHandle(data->objMap);
	UnmapViewOfFile(data->airports);
	CloseHandle(data->objAirports);
	free(data);

	return((int)lpMsg.wParam);
}

LRESULT CALLBACK TrataEventos(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
	TCHAR nomeAeroporto[500];
	TCHAR infoAeroporto[500];
	HDC hdc;
	RECT rect;
	PAINTSTRUCT ps;
	int x, y, primeiraVez = 1;

	//data = GetWindowLongPtr(hWnd, GWLP_USERDATA);

	switch (messg)
	{

	case WM_CREATE:

		break;

	case WM_PAINT:
				hdc = BeginPaint(hWnd, &ps);
				GetClientRect(hWnd, &rect);

				if (memDC == NULL)
				{
					memDC = CreateCompatibleDC(hdc);
					hBitmapDB = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);
					SelectObject(memDC, hBitmapDB);
					DeleteObject(hBitmapDB);
				}
		
				FillRect(memDC, &rect, CreateSolidBrush(RGB(132, 219, 255)));
				SetTextColor(hdc, RGB(0, 255, 0));
				SetBkMode(memDC, TRANSPARENT);
				WaitForSingleObject(hMutex, INFINITE);
				for (int i = 0; i < data->nrAirports; i++) {
					//xBitmap = (data->airports[i].coordinates[0] / 2) - (bmp.bmWidth / 2);
					//yBitmap = (data->airports[i].coordinates[1] / 2) - (bmp.bmHeight / 2);
					xBitmap = (data->airports[i].coordinates[0] / 2);
					yBitmap = (data->airports[i].coordinates[1] / 2);
					BitBlt(memDC, xBitmap, yBitmap, bmp.bmWidth, bmp.bmHeight, bmpDC, 0, 0, SRCCOPY);
				}
				ReleaseMutex(hMutex);
		
				BitBlt(hdc, 0, 0, rect.right, rect.bottom, memDC, 0, 0, SRCCOPY);

				EndPaint(hWnd, &ps);
				break;

	case WM_LBUTTONDOWN:
		x  = GET_X_LPARAM(lParam);
		y = GET_Y_LPARAM(lParam);
		for (int i = 0; i < data->nrAirports; i++) {
			int airplaneX = data->airports[i].coordinates[0] /2;
			int airplaneY = data->airports[i].coordinates[1] /2;
			if (x >= airplaneX && y >= airplaneY && x <= (airplaneX + bmp.bmWidth) && y <= (airplaneY + bmp.bmHeight)) {
				_stprintf_s(nomeAeroporto,500, _T("Aeroporto '%s'\n"), data->airports[i].name);
				_stprintf_s(infoAeroporto,500, _T("Coordenadas [%d,%d]\n"), data->airports[i].coordinates[0], data->airports[i].coordinates[1]);
				for(int j = 0; j < getNumberOfAirplanes(data); j++)
					if(!_tcscmp(data->planes[j].actualAirport, data->airports[i].name)){
						if (primeiraVez) {
							_stprintf_s(infoAeroporto, 500, _T("%s\nAvi�es neste aeroporto:\n"), infoAeroporto);
							primeiraVez--;
						}
						_stprintf_s(infoAeroporto, 500, _T("%sID '%d'"), infoAeroporto, data->planes[j].planeID);
						if (_tcscmp(data->planes[j].destinAirport, _T("NULL")))
							_stprintf_s(infoAeroporto, 500, _T("%s com destino � '%s'\n"), infoAeroporto, data->planes[j].destinAirport);
						else
							_stprintf_s(infoAeroporto, 500, _T("%s\n"), infoAeroporto);
						_stprintf_s(infoAeroporto, 500, _T("%sCapacidade: %3d\n"), infoAeroporto, data->planes[j].maxCapacity);
						_stprintf_s(infoAeroporto, 500, _T("%sVelocidade: %3d\n"), infoAeroporto, data->planes[j].velocity);
						_stprintf_s(infoAeroporto, 500, _T("%s--------------------------------------------------\n"),infoAeroporto);
					}
				MessageBox(hWnd, infoAeroporto, nomeAeroporto, 0);

			}
		}
		InvalidateRect(hWnd, NULL, FALSE);
		break;
	case WM_COMMAND:

		switch (LOWORD(wParam))
		{
		//case ID_LOGIN:
		//	DialogBox(NULL, MAKEINTRESOURCE(IDD_DIALOG_LOGIN), hWnd, TrataEventosLogin);
		//	EnableMenuItem(GetMenu(hWnd), ID_CONSULTA, MF_ENABLED);
		//	EnableMenuItem(GetMenu(hWnd), ID_LEVANTAMENTO, MF_ENABLED);
		//	break;

		case ID_ABRIRTERMINAL:
		//case ID_ACCELERATOR_C:
			DialogBox(NULL, MAKEINTRESOURCE(IDD_TERMINAL), NULL, TrataEventosTerminal);
			break;

		}

		break;

	case WM_ERASEBKGND:
		return TRUE;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, messg, wParam, lParam);
		break;
	}

	return(0);

}

LRESULT CALLBACK TrataEventosTerminal(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
	HWND hwndList;
	TCHAR welcome[300] = _T("Comando 'help' \na qualquer momento para rever informa��es dos comandos.");
	TCHAR comando[300];
	TCHAR retorno[10][500];

	switch (messg)
	{
	case WM_INITDIALOG:

		hwndList = GetDlgItem(hWnd, IDC_CONSOLA);
		data->hwndList = hwndList;

		SendMessage(hwndList, LB_ADDSTRING, 0, welcome);
		interpretaComandoControladorGUI(_T("help"), data, hwndList);

		break;

		// quando ha um evento sobre a listbox ha um evento WM_COMMAND
	case WM_COMMAND:

		if (LOWORD(wParam) == IDOK)
		{
			hwndList = GetDlgItem(hWnd, IDC_CONSOLA);
			GetDlgItemText(hWnd, IDC_COMANDO, comando, 300);
			SetDlgItemTextA(hWnd, IDC_COMANDO, _T(""));
			interpretaComandoControladorGUI(comando, data, hwndList);
			InvalidateRect(hWndGlobal, NULL, FALSE);
			SendMessage(hwndList, WM_VSCROLL, SB_BOTTOM, NULL); //s� demorei 30m a procura desta linha ! adoro win API!
		}

		break;

	case WM_CLOSE:

		EndDialog(hWnd, 0);
		return TRUE;
	}

	return FALSE;
}

//
//LRESULT CALLBACK TrataEventos(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
//{
//	HDC hdc;
//	RECT rect;
//	PAINTSTRUCT ps;
//	static TCHAR charAtual = '?';
//	static PosChar posicoes[500];
//	static int totalPos = 0;
//
//	switch (messg)
//	{
//	case WM_PAINT:
//		hdc = BeginPaint(hWnd, &ps);
//		GetClientRect(hWnd, &rect);
//
//		if (memDC == NULL)
//		{
//			memDC = CreateCompatibleDC(hdc);
//			hBitmapDB = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);
//			SelectObject(memDC, hBitmapDB);
//			DeleteObject(hBitmapDB);
//		}
//
//		FillRect(memDC, &rect, CreateSolidBrush(RGB(50, 50, 50)));
//		SetTextColor(hdc, RGB(0, 255, 0));
//		SetBkMode(memDC, TRANSPARENT);
//
//		WaitForSingleObject(hMutex, INFINITE);
//		BitBlt(memDC, xBitmap, yBitmap, bmp.bmWidth, bmp.bmHeight, bmpDC, 0, 0, SRCCOPY);
//		ReleaseMutex(hMutex);
//
//		BitBlt(hdc, 0, 0, rect.right, rect.bottom, memDC, 0, 0, SRCCOPY);
//
//		for (int i = 0; i < totalPos; i++) {
//			rect.left = posicoes[i].xPos;
//			rect.top = posicoes[i].yPos;
//			DrawText(hdc, &posicoes[i].c, 1, &rect, DT_NOCLIP | DT_SINGLELINE);
//		}
//		EndPaint(hWnd, &ps);
//		break;
//
//	case WM_SIZE:
//
//		WaitForSingleObject(hMutex, INFINITE);
//		xBitmap = (LOWORD(lParam) / 2) - (bmp.bmWidth / 2);
//		yBitmap = (HIWORD(lParam) / 2) - (bmp.bmHeight / 2);
//		limDir = LOWORD(lParam) - bmp.bmWidth;
//		ReleaseDC(hWnd, memDC);
//		memDC = NULL;
//		ReleaseMutex(hMutex);
//		break;
//
//	case WM_LBUTTONDOWN:
//		posicoes[totalPos].xPos = GET_X_LPARAM(lParam);
//		posicoes[totalPos].yPos = GET_Y_LPARAM(lParam);
//		posicoes[totalPos].c = charAtual;
//		totalPos++;
//		InvalidateRect(hWnd, NULL, FALSE);
//		break;
//
//	case WM_ERASEBKGND:
//		return TRUE;
//
//	case WM_CHAR:
//		charAtual = (TCHAR)wParam;
//		break;
//
//	case WM_CLOSE:
//		if (MessageBox(hWnd, TEXT("Tem a certeza que quer sair?"),
//			_T("Confirma��o"), MB_YESNO | MB_ICONQUESTION) == IDYES)
//		{
//			DestroyWindow(hWnd);
//		}
//		break;
//
//	case WM_DESTROY:	// Destruir a janela e terminar o programa 
//						// "PostQuitMessage(Exit Status)"		
//		PostQuitMessage(0);
//		break;
//	default:
//		// Neste exemplo, para qualquer outra mensagem (p.e. "minimizar","maximizar","restaurar")
//		// n�o � efectuado nenhum processamento, apenas se segue o "default" do Windows
//		return(DefWindowProc(hWnd, messg, wParam, lParam));
//		break;  // break tecnicamente desnecess�rio por causa do return
//	}
//	return(0);
//}