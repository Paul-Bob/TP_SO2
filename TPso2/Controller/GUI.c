#include <windows.h>
#include <Windowsx.h>
#include <tchar.h>
#include "controller.h"
#include "Winuser.h"

#include "../HF/structs.h"
#include "prepareEnv.h"
#include "textUI.h"
#include "sharedMemory.h"
#include "resource.h"

LRESULT CALLBACK TrataEventos(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK TrataEventosTerminal(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK TrataEventosNovoAeroporto(HWND, UINT, WPARAM, LPARAM);


TCHAR szProgName[] = TEXT("Ficha8");
pDATA data;

HBITMAP hBmp, hBmpPN, hBmpPS, hBmpPE, hBmpPO, hBmpPNE, hBmpPNO, hBmpPSE, hBmpPSO;
HDC bmpDC, bmpDCPN, bmpDCPS, bmpDCPE, bmpDCPO, bmpDCPNE, bmpDCPNO, bmpDCPSE, bmpDCPSO;
BITMAP bmp, bmpPN, bmpPS, bmpPE, bmpPO, bmpPNE, bmpPNO, bmpPSE, bmpPSO;
int xBitmap;
int yBitmap;

int limDir;
HWND hMutex;

HDC memDC = NULL;
HBITMAP hBitmapDB;

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

	wcApp.lpszMenuName = MAKEINTRESOURCE(ID_MENU_PRINCIPAL);
	wcApp.style = CS_HREDRAW | CS_VREDRAW;
	wcApp.hIcon = LoadIcon(hInst, (LPWSTR)IDI_ICON3);
	wcApp.hIconSm = LoadIcon(hInst, (LPWSTR)IDI_ICON3);
	wcApp.hCursor = LoadCursor(NULL, IDC_ARROW);

	//wcApp.cbClsExtra = sizeof(dados);
	wcApp.cbClsExtra = 0;
	wcApp.cbWndExtra = 0;
	wcApp.hbrBackground = CreateSolidBrush(RGB(132, 219, 255));

	if (!RegisterClassEx(&wcApp))
		return(0);

	hWnd = CreateWindow(
		szProgName,
		TEXT("SO2 - Controlador Aéreo"),
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

	//previne poder ter mais do que uma instância do mesmo programa a correr em simultâneo.
	CreateMutexA(0, FALSE, "Local\\$controlador$"); // try to create a named mutex
	if (GetLastError() == ERROR_ALREADY_EXISTS) {   // did the mutex already exist?
		//Se tiver tempo fazer mesagebox...
		return -1;								  // quit; mutex is released automatically
	}

	//Coloca as variaveis no registry caso não estejam onde é esperado...
	if (!prepareEnvironment(KEY_PATH)) {
		//puts("Registry fail!");
		return -1;
	}

	//Obtém variáveis que supostamente estão no Registry
	data->maxAirplanes = getMax(MAX_AIRPLANES, KEY_PATH);
	data->maxAirports = getMax(MAX_AIRPORTS, KEY_PATH);
	if (!data->maxAirplanes || !data->maxAirports) {
		//_tprintf(L"Registry vars fail\n");
		return -1;
	}

	//estas funções internamente fornecem mais informações caso algo falhe
	if (!createAirportSpace(data) || !createMap(data) || !createAirplaneSpace(data) || !createProducerConsumer(data)) {
		//_ftprintf(stderr, L"File mapping fail\n");
		return -1;
	}

	_tprintf(L"Max airports do registry :  %d\nMax airplanes do registry : %d\nComando 'help' para mais informações.\n\n",
		data->maxAirports, data->maxAirplanes);

	initProducerConsumerThread(data);
	initControlPassengerRegisterThread(data);
	/*FinalCoppyPaste do controlador.c ------------------------------------------------------------------------------------------------------------------------------------------------------------*/

	HDC hdc;
	RECT rect;

	hBmp = (HBITMAP)LoadImage(NULL, _T("control.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	hBmpPN = (HBITMAP)LoadImage(NULL, _T("planeN.bmp"), IMAGE_BITMAP, 20, 20, LR_LOADFROMFILE);
	hBmpPS = (HBITMAP)LoadImage(NULL, _T("planeS.bmp"), IMAGE_BITMAP, 20, 20, LR_LOADFROMFILE);
	hBmpPE = (HBITMAP)LoadImage(NULL, _T("planeE.bmp"), IMAGE_BITMAP, 20, 20, LR_LOADFROMFILE);
	hBmpPO = (HBITMAP)LoadImage(NULL, _T("planeW.bmp"), IMAGE_BITMAP, 20, 20, LR_LOADFROMFILE);
	hBmpPNE = (HBITMAP)LoadImage(NULL, _T("planeNE.bmp"), IMAGE_BITMAP, 20, 20, LR_LOADFROMFILE);
	hBmpPNO = (HBITMAP)LoadImage(NULL, _T("planeNW.bmp"), IMAGE_BITMAP, 20, 20, LR_LOADFROMFILE);
	hBmpPSE = (HBITMAP)LoadImage(NULL, _T("planeSE.bmp"), IMAGE_BITMAP, 20, 20, LR_LOADFROMFILE);
	hBmpPSO = (HBITMAP)LoadImage(NULL, _T("planeSW.bmp"), IMAGE_BITMAP, 20, 20, LR_LOADFROMFILE);
	GetObject(hBmp, sizeof(bmp), &bmp);
	GetObject(hBmpPN, sizeof(bmpPN), &bmpPN);
	GetObject(hBmpPS, sizeof(bmpPS), &bmpPS);
	GetObject(hBmpPE, sizeof(bmpPE), &bmpPE);
	GetObject(hBmpPO, sizeof(bmpPO), &bmpPO);
	GetObject(hBmpPNE, sizeof(bmpPNE), &bmpPNE);
	GetObject(hBmpPNO, sizeof(bmpPNO), &bmpPNO);
	GetObject(hBmpPSE, sizeof(bmpPSE), &bmpPSE);
	GetObject(hBmpPSO, sizeof(bmpPSO), &bmpPSO);

	hdc = GetDC(hWnd);
	bmpDC = CreateCompatibleDC(hdc);
	bmpDCPN = CreateCompatibleDC(hdc);
	bmpDCPS = CreateCompatibleDC(hdc);
	bmpDCPE = CreateCompatibleDC(hdc);
	bmpDCPO = CreateCompatibleDC(hdc);
	bmpDCPNE = CreateCompatibleDC(hdc);
	bmpDCPNO = CreateCompatibleDC(hdc);
	bmpDCPSE = CreateCompatibleDC(hdc);
	bmpDCPSO = CreateCompatibleDC(hdc);
	SelectObject(bmpDC, hBmp);
	SelectObject(bmpDCPN, hBmpPN);
	SelectObject(bmpDCPS, hBmpPS);
	SelectObject(bmpDCPE, hBmpPE);
	SelectObject(bmpDCPO, hBmpPO);
	SelectObject(bmpDCPNE, hBmpPNE);
	SelectObject(bmpDCPNO, hBmpPNO);
	SelectObject(bmpDCPSE, hBmpPSE);
	SelectObject(bmpDCPSO, hBmpPSO);
	ReleaseDC(hWnd, hdc);

	GetClientRect(hWnd, &rect);
	//xBitmap = (rect.right / 2) - (bmp.bmWidth / 2);
	//yBitmap = (rect.bottom / 2) - (bmp.bmHeight / 2);

	limDir = rect.right - bmp.bmWidth;
	data->hWndGlobal = hWnd;

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
	TCHAR infoAirplane[500];
	HDC hdc;
	RECT rect;
	PAINTSTRUCT ps;
	int x, y, primeiraVezAviao = 1, primeiraVezPassageiro = 1;
	HWND static hPop = NULL;
	static int planeID = -1;

	//data = GetWindowLongPtr(hWnd, GWLP_USERDATA);

	switch (messg)
	{

	case WM_CREATE:
		EnableMenuItem(GetMenu(hWnd), ID_ATIVAR, MF_DISABLED | MF_GRAYED);
		break;

	case WM_MOUSEMOVE:
		x = GET_X_LPARAM(lParam);
		y = GET_Y_LPARAM(lParam);

		for (int i = 0; i < getNumberOfAirplanes(data); i++) {
			if (_tcscmp(data->planes[i].actualAirport, _T("Fly"))) {
				if (hPop && planeID == data->planes[i].planeID) {
					ShowWindow(hPop, SW_HIDE);
					DestroyWindow(hPop);
					hPop = NULL;
					planeID = -1;
				}
				continue;
			}

			primeiraVezPassageiro = 1;
			int airplaneX = (data->planes[i].current.x / 2);
			int airplaneY = (data->planes[i].current.y / 2);
			if (x >= airplaneX && y >= airplaneY && x <= (airplaneX + bmpPN.bmWidth) && y <= (airplaneY + bmpPN.bmHeight))
			{
				_stprintf_s(infoAirplane, 500, _T("Avião %d"), data->planes[i].planeID);
				if (_tcscmp(data->planes[i].destinAirport, _T("NULL")))
					_stprintf_s(infoAirplane, 500, _T("%s com destino à '%s'\n"), infoAirplane, data->planes[i].destinAirport);
				else
					_stprintf_s(infoAirplane, 500, _T("%s\n"), infoAirplane);
				_stprintf_s(infoAirplane, 500, _T("%sCapacidade: %3d\n"), infoAirplane, data->planes[i].maxCapacity);
				_stprintf_s(infoAirplane, 500, _T("%sVelocidade : %3d\n"), infoAirplane, data->planes[i].velocity);
				_stprintf_s(infoAirplane, 500, _T("%sCoordenadas [%d,%d]\n"), infoAirplane, data->planes[i].current.x, data->planes[i].current.y);
				for (int k = 0; k < data->nrPassengers; k++)
					if (data->passengers[k].planeID == data->planes[i].planeID) {
						if (primeiraVezPassageiro) {
							_stprintf_s(infoAirplane, 500, _T("%s\nPassageiros neste avião:\n"), infoAirplane);
							primeiraVezPassageiro--;
						}
						_stprintf_s(infoAirplane, 500, TEXT("%s    - %s\n"), infoAirplane, data->passengers[k].name);
					}

				if (!hPop || (hPop && planeID != data->planes[i].planeID)) {
					if (hPop) {
						ShowWindow(hPop, SW_HIDE);
						DestroyWindow(hPop);
						hPop = NULL;
					}
					GetWindowRect(hWnd, &rect);
					hPop = CreateWindowEx(WS_EX_CLIENTEDGE, //WS_EX_CLIENTEDGE,
						TEXT("STATIC"),
						infoAirplane,
						WS_POPUP | WS_BORDER | WS_SYSMENU,
						rect.left + x, rect.top + y + 85, 150, 170,
						hWnd, (HMENU)0, hWnd, NULL);
					if (hPop) {
						planeID = data->planes[i].planeID;
						ShowWindow(hPop, SW_SHOW);
					}
				}
			}
			else if (hPop && planeID == data->planes[i].planeID) {
				ShowWindow(hPop, SW_HIDE);
				DestroyWindow(hPop);
				hPop = NULL; 
				planeID = -1;
			}
		}

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
				for (int i = 0; i < getNumberOfAirplanes(data); i++)
				{
					if (!_tcscmp(data->planes[i].actualAirport, _T("Fly"))) {
						xBitmap = (data->planes[i].current.x / 2);
						yBitmap = (data->planes[i].current.y / 2);
						//1N 2S 3E 4O 5NE 6NO 7SE 8SO
						switch (data->planes[i].orientation) 
						{
							case 1: BitBlt(memDC, xBitmap, yBitmap, bmpPN.bmWidth, bmpPN.bmHeight, bmpDCPN, 0, 0, SRCCOPY); break;
							case 2: BitBlt(memDC, xBitmap, yBitmap, bmpPN.bmWidth, bmpPN.bmHeight, bmpDCPS, 0, 0, SRCCOPY); break;
							case 3: BitBlt(memDC, xBitmap, yBitmap, bmpPN.bmWidth, bmpPN.bmHeight, bmpDCPE, 0, 0, SRCCOPY); break;
							case 4: BitBlt(memDC, xBitmap, yBitmap, bmpPN.bmWidth, bmpPN.bmHeight, bmpDCPO, 0, 0, SRCCOPY); break;
							case 5: BitBlt(memDC, xBitmap, yBitmap, bmpPN.bmWidth, bmpPN.bmHeight, bmpDCPNE, 0, 0, SRCCOPY); break;
							case 6: BitBlt(memDC, xBitmap, yBitmap, bmpPN.bmWidth, bmpPN.bmHeight, bmpDCPNO, 0, 0, SRCCOPY); break;
							case 7: BitBlt(memDC, xBitmap, yBitmap, bmpPN.bmWidth, bmpPN.bmHeight, bmpDCPSE, 0, 0, SRCCOPY); break;
							case 8: BitBlt(memDC, xBitmap, yBitmap, bmpPN.bmWidth, bmpPN.bmHeight, bmpDCPSO, 0, 0, SRCCOPY); break;
						}
					}
				}
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
			int airportX = data->airports[i].coordinates[0] /2;
			int airportY = data->airports[i].coordinates[1] /2;
			if (x >= airportX && y >= airportY && x <= (airportX + bmp.bmWidth) && y <= (airportY + bmp.bmHeight)) {
				_stprintf_s(nomeAeroporto,500, _T("Aeroporto '%s'\n"), data->airports[i].name);
				_stprintf_s(infoAeroporto,500, _T("Coordenadas [%d,%d]\n"), data->airports[i].coordinates[0], data->airports[i].coordinates[1]);
				for (int j = 0; j < getNumberOfAirplanes(data); j++) {
					if (!_tcscmp(data->planes[j].actualAirport, data->airports[i].name)) {
						if (primeiraVezAviao) {
							_stprintf_s(infoAeroporto, 500, _T("%s\n--------------------------------------------------\n"), infoAeroporto);
							_stprintf_s(infoAeroporto, 500, _T("%s\nAviões neste aeroporto:\n"), infoAeroporto);
							primeiraVezAviao--;
						}
						_stprintf_s(infoAeroporto, 500, _T("%sID '%d'"), infoAeroporto, data->planes[j].planeID);
						if (_tcscmp(data->planes[j].destinAirport, _T("NULL")))
							_stprintf_s(infoAeroporto, 500, _T("%s com destino à '%s'\n"), infoAeroporto, data->planes[j].destinAirport);
						else
							_stprintf_s(infoAeroporto, 500, _T("%s\n"), infoAeroporto);
						_stprintf_s(infoAeroporto, 500, _T("%sCapacidade: %3d\n"), infoAeroporto, data->planes[j].maxCapacity);
						_stprintf_s(infoAeroporto, 500, _T("%sVelocidade : %3d\n"), infoAeroporto, data->planes[j].velocity);
						for (int k = 0; k < data->nrPassengers; k++)
							if (data->passengers[k].planeID == data->planes[j].planeID) {
								if (primeiraVezPassageiro) {
									_stprintf_s(infoAeroporto, 500, _T("%s\nPassageiros neste avião:\n"), infoAeroporto);
									primeiraVezPassageiro--;
								}
								_stprintf_s(infoAeroporto, 500, TEXT("%s    - %s\n"), infoAeroporto, data->passengers[k].name);
							}

					}
					_stprintf_s(infoAeroporto, 500, TEXT("%s\n"), infoAeroporto);
					primeiraVezPassageiro = 1;
				}
				
				for (int k = 0; k < data->nrPassengers; k++)
					if (data->passengers[k].planeID == -1 && !_tcscmp(data->passengers[k].origin, data->airports[i].name)) {
						if (primeiraVezPassageiro) {
							_stprintf_s(infoAeroporto, 500, _T("%s--------------------------------------------------\n"), infoAeroporto);
							_stprintf_s(infoAeroporto, 500, _T("%s\nPassageiros sem avião neste aeroporto:\n"), infoAeroporto);
							primeiraVezPassageiro--;
						}
						_stprintf_s(infoAeroporto, 500, TEXT("%s    - %s com destino à '%s'\n"), infoAeroporto, data->passengers[k].name, data->passengers[k].destiny);
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
		case ID_ATIVAR:
			interpretaComandoControladorGUI(_T("ativar"), data, NULL);
			EnableMenuItem(GetMenu(hWnd), ID_ATIVAR, MF_DISABLED | MF_GRAYED);
			EnableMenuItem(GetMenu(hWnd), ID_SUSPENDER, MF_ENABLED);
			break;

		case ID_SUSPENDER:
			interpretaComandoControladorGUI(_T("suspender"), data, NULL);
			EnableMenuItem(GetMenu(hWnd), ID_ATIVAR, MF_ENABLED);
			EnableMenuItem(GetMenu(hWnd), ID_SUSPENDER, MF_DISABLED | MF_GRAYED);
			break;

		case ID_NOVOAEROPORTO:
			DialogBox(NULL, MAKEINTRESOURCE(IDD_NOVO), NULL, TrataEventosNovoAeroporto);
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
	TCHAR welcome[300] = _T("Comando 'help' \na qualquer momento para rever informações dos comandos.");
	TCHAR comando[300];

	switch (messg)
	{
	case WM_INITDIALOG:

		hwndList = GetDlgItem(hWnd, IDC_CONSOLA);
		data->hwndList = hwndList;

		SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)welcome);
		interpretaComandoControladorGUI(_T("help"), data, hwndList);

		break;

		// quando ha um evento sobre a listbox ha um evento WM_COMMAND
	case WM_COMMAND:

		if (LOWORD(wParam) == IDOK)
		{
			hwndList = GetDlgItem(hWnd, IDC_CONSOLA);
			GetDlgItemText(hWnd, IDC_COMANDO, comando, 300);
			SetDlgItemTextA(hWnd, IDC_COMANDO, (LPWSTR)_T(""));
			interpretaComandoControladorGUI(comando, data, hwndList);
			InvalidateRect(data->hWndGlobal, NULL, FALSE);
			SendMessage(hwndList, WM_VSCROLL, SB_BOTTOM, (LPARAM)NULL); //só demorei 30m a procura desta linha ! adoro win API!
		}

		break;

	case WM_CLOSE:

		EndDialog(hWnd, 0);
		return TRUE;
	}

	return FALSE;
}

LRESULT CALLBACK TrataEventosNovoAeroporto(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
	HWND hwndList;
	TCHAR nome[100] = _T("^");
	TCHAR comando[300];
	TCHAR x[5] = _T("a");
	TCHAR y[5] = _T("a");


	switch (messg)
	{

	case WM_COMMAND:

		if (LOWORD(wParam) == IDOK)
		{
			hwndList = GetDlgItem(hWnd, IDC_CONSOLA);
			GetDlgItemText(hWnd, IDC_NOME, nome, 100);
			GetDlgItemText(hWnd, IDC_X, x, 5);
			GetDlgItemText(hWnd, IDC_Y, y, 5);
			if (!_tcscmp(nome, _T("")) || !_tcscmp(x, _T("")) || !_tcscmp(y, _T(""))) {
				EndDialog(hWnd, 0);
				return TRUE;
			}

			_stprintf_s(comando, 300, _TEXT("criar %s %s %s"), nome, x, y);
			interpretaComandoControladorGUI(comando, data, hwndList);
			InvalidateRect(data->hWndGlobal, NULL, FALSE);
			EndDialog(hWnd, 0);
			return TRUE;
		}
		else if (LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hWnd, 0);
			return TRUE;
		}
		break;

	case WM_CLOSE:

		EndDialog(hWnd, 0);
		return TRUE;
	}

	return FALSE;
}