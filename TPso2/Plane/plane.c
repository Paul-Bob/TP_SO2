#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>

#include "plane.h"
#include "../HF/structs.h"

#define SIZE 200

//depois alterar
int openCreateKey(HKEY* key, DWORD* result, TCHAR* path) {
	if (RegCreateKeyEx(HKEY_CURRENT_USER, path, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, key, result) != ERROR_SUCCESS) {
		_tprintf(TEXT("Chave n�o foi criada nem aberta, erro!\n"));
		RegCloseKey(*key);
		return 0;
	}
	return 1;
}
int getMax(TCHAR* attribute, TCHAR* path) {
	HKEY key;
	DWORD result;
	DWORD valor = 0;
	DWORD type, size = sizeof(valor);

	if (!openCreateKey(&key, &result, path))
		return 0;

	if (result == REG_CREATED_NEW_KEY) {
		RegCloseKey(key);
		return 0;
	}
	else if (result == REG_OPENED_EXISTING_KEY) {
		if (RegQueryValueEx(key, attribute, NULL, &type, (LPBYTE)&valor, &size) != ERROR_SUCCESS) {
			_tprintf(L"Can't acces registry '%s'...\n", attribute);
			RegCloseKey(key);
			return 0;
		}
	}
	RegCloseKey(key);
	return valor;
}
//nao sei como agora

int setInitialAirport(TCHAR* name, pPlane plane) {
	plane->current.x = -1;

	HANDLE objAirports = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, _T("airports"));

	if (objAirports == NULL) {
		_ftprintf(stderr, L"Imposs�vel abrir o file mapping.\n");
		return 0;
	}

	int maxAirports = getMax(MAX_AIRPORTS, KEY_PATH);

	pAirport airports = (pAirport)MapViewOfFile(objAirports, FILE_MAP_ALL_ACCESS, 0, 0, maxAirports * sizeof(airport));

	if (airports == NULL) {
		_ftprintf(stderr, L"Imposs�vel criar o map view.\n");
		return 0;
	}
	
	for (int i = 0; i < maxAirports - 1; i++) {
		if (!_tcscmp(name, airports[i].name)) {
			plane->current.x = airports[i].coordinates[X];
			plane->current.y = airports[i].coordinates[Y];
			break;
		}
	}

	if (plane->current.x == -1)
		return 0;

	return 1;
}

int getArguments(int argc, LPTSTR argv[], pPlane plane) {

	//TODO: Falta o aeroporto que n�o fa�o ideia como vou saber qual o aeroporto onde estou ?????' -.-''' "#$"#$ fuck my life

	if (argc != 4) {
		_ftprintf(stderr, L"Argumentos para lan�ar o avi�o insuficientes\n");
		return -1;
	}
	

	if (_stscanf_s(argv[1], L"%d", &plane->maxCapacity) == 0) {
		_ftprintf(stderr, L"Capacidade do avi�o inv�lida\n");
		return -1;
	}
	if (_stscanf_s(argv[2], L"%d", &plane->velocity) == 0) {
		_ftprintf(stderr, L"Velocidade do avi�o inv�lida\n");
		return -1;
	}
	if (!setInitialAirport(argv[3], plane)) {
		_ftprintf(stderr, L"Aeroporto inicial inv�lido...\n");
		return -1;
	}


	return 1;
}

void notifyController() {
	_tprintf(L"[DEBUG] Amigo controlador, cheguei, t�? \n"); //TODO: Avisar o controlador que cheguei ou atualizar a minha posicao
}

void setPosition(pMap map, int x, int y, int value) {
	HANDLE mutex = OpenMutex(SYNCHRONIZE, FALSE, _T("mapMutex"));
	if (mutex == NULL) {
		_ftprintf(stderr, L"N�o foi poss�vel atualizar o mapa.\n");
		return;
	}

	map->matrix[x][y] = value;
	ReleaseMutex(mutex);
}

int getValueOnPosition(pMap map, int x, int y) {
	HANDLE mutex = OpenMutex(SYNCHRONIZE, FALSE, _T("mapMutex"));
	if (mutex == NULL) {
		_ftprintf(stderr, L"N�o foi poss�vel aceder ao mapa.\n");
		return -1;
	}
	int value = map->matrix[x][y];
	ReleaseMutex(mutex);
	return value;
}

void initTrip(pPlane plane) {

	HMODULE dll = LoadLibraryEx(_T("SO2_TP_DLL_2021"), NULL, 0);

	if (dll == NULL) {
		_ftprintf(stderr, L"Imposs�vel encontrar a DLL.\n");
		return;
	}

	FARPROC genericPointer = GetProcAddress(dll, "move");

	if (genericPointer == NULL) {
		_ftprintf(stderr, L"Imposs�vel encontrar o m�todo move.\n");
		return;
	}

	int(*move)(int, int, int, int, int*, int*) = (int(*)(int, int, int, int, int*, int*))genericPointer;

	HANDLE objMap = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, _T("map"));

	if (objMap == NULL) {
		_ftprintf(stderr, L"Imposs�vel abrir o file mapping.\n");
		return; 
	}

	pMap map = (pMap)MapViewOfFile(objMap, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Map));
	 
	if (map == NULL) {
		_ftprintf(stderr, _T("Imposs�vel criar o map view.\n"));
		return;
	}
	
	setPosition(map, plane->current.x, plane->current.y, 1); // Ocupa a posi��o de sobrevoar o aeroporto.
	
	while (1) {

		 Sleep(1000 / (DWORD)plane->velocity);

		int nextX = 0, nextY = 0;
		int result = move(plane->current.x, plane->current.y, plane->final.x, plane->final.y, &nextX, &nextY);
		if (result == 2) {
			continue;
		}
		else if (result == 0) {
			_tprintf(L"[DEBUG] Cheguei ao meu destino, tenho que avisar o controlador \n"); 
			setPosition(map, plane->current.x, plane->current.y, 0);
			notifyController();
			break;
		}
		else {
			HANDLE mutex = OpenMutex(SYNCHRONIZE, FALSE, _T("mapMutex"));
			if (mutex == NULL) {
				_ftprintf(stderr, L"N�o foi poss�vel aceder ao mapa.\n");
				continue;
			}

			if (map->matrix[nextX][nextY] == 0) {
				map->matrix[plane->current.x][plane->current.y] = 0;
				plane->current.x = nextX;
				plane->current.y = nextY;
				map->matrix[nextX][nextY] = 1;
			}
			else {
				//TODO: Podemos melhorar esta estrategia, para j� estamos s� a aguardar que os gordos desocupem a loja
			}
			ReleaseMutex(mutex);
		}
	}
	FreeLibrary(dll);
	UnmapViewOfFile(map);
	CloseHandle(objMap);
}


void initTripThread(pPlane plane) {
	HANDLE tripThread;
	int threadPlaneID;

	tripThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)initTrip, (LPVOID) plane, 0, &threadPlaneID);
	if (tripThread == NULL) {
		_ftprintf(stderr, L"N�o foi poss�vel criar a thread.\n");
	}
}

int processCommand(TCHAR* command, pPlane plane) {
	if (!_tcscmp(command, TEXT("exit"))) {
		return 0;
	}

	if (!_tcscmp(command, TEXT("destino"))) {
		_tprintf(L"[DEBUG] Vou definir o destino %s\n", command);
	} else if (!_tcscmp(command, TEXT("iniciar"))) {
		initTripThread(plane);
	} 

	return 1;
}

void registerPlaneInController(pPlane plane) {
	//TODO: Comunicar ao controlador que existo
}

int _tmain(int argc, LPTSTR argv[]) {

#ifdef UNICODE
	(void)_setmode(_fileno(stdin), _O_WTEXT);
	(void)_setmode(_fileno(stdout), _O_WTEXT);
	(void)_setmode(_fileno(stderr), _O_WTEXT);
#endif

	Plane plane;

	if (getArguments(argc, argv, &plane) == -1) {
		return 1;
	}

	plane.current.x = 0;
	plane.current.y = 1;
	plane.final.x = 200;
	plane.final.y = 200;
	_tprintf(L"[DEBUG] Cap: %d - Vel: %d\n", plane.maxCapacity, plane.velocity);

	registerPlaneInController(&plane);
	
	TCHAR command[SIZE];
	do {
		_tprintf(L"-> ");
		_fgetts(command, SIZE, stdin);
		command[_tcslen(command) - 1] = '\0';
	} while (processCommand(command, &plane) != 0);

	return 0;
}
