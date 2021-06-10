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
	wcApp.hbrBackground = CreateSolidBrush(RGB(220, 220, 220));

	if (!RegisterClassEx(&wcApp))
		return(0);

	hWnd = CreateWindow(
		szProgName,
		TEXT("SO2 - Controlador Aéreo"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		500,
		500
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

	data->nrAirplanes = 0;

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

	//dadosPartilhados.numOperacoes = 5; // Apenas para testar...
	LONG_PTR x = SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)&data);

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
	TCHAR str1[512], str2[512];
	//dados* dadosPartilhados;
	pDATA data;

	data = GetWindowLongPtr(hWnd, GWLP_USERDATA);

	//dadosPartilhados = (dados*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

	switch (messg)
	{
		// evento de criação de janela
	case WM_CREATE:
		// começa com os menus desativadas
		//EnableMenuItem(GetMenu(hWnd), ID_CONSULTA, MF_DISABLED | MF_GRAYED);
		//EnableMenuItem(GetMenu(hWnd), ID_LEVANTAMENTO, MF_DISABLED | MF_GRAYED);
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
		//	break;

		//case ID_ACCELERATOR_L:
		//case ID_LEVANTAMENTO:

		//	DialogBox(NULL, MAKEINTRESOURCE(IDD_LEVANTAMENTO), NULL, TrataEventosLevantar);
		//	break;

		}

		break;

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
	TCHAR retorno[10][500];
	// quando uma dialogbox é inicializava, ela recebe um evento chamado de WM_INITDIALOG
	// no caso das janelas, é um evento chamado de WM_CREATE

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

			SendMessage(hwndList, WM_VSCROLL, SB_BOTTOM, NULL); //só demorei 30m a procura desta linha ! adoro win API!
		}

		break;

	case WM_CLOSE:

		EndDialog(hWnd, 0);
		return TRUE;
	}

	return FALSE;
}