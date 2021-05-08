#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#include "plane.h"
#include "../HF/structs.h"

#define SIZE 200

//depois alterar
int openCreateKey(HKEY* key, DWORD* result, TCHAR* path) {
	if (RegCreateKeyEx(HKEY_CURRENT_USER, path, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, key, result) != ERROR_SUCCESS) {
		_tprintf(TEXT("Chave não foi criada nem aberta, erro!\n"));
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
	plane->initial.x = -1;

	HANDLE objAirports = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, _T("airports"));

	if (objAirports == NULL) {
		_ftprintf(stderr, L"Impossível abrir o file mapping.\n");
		return 0;
	}

	int maxAirports = getMax(MAX_AIRPORTS, KEY_PATH);

	pAirport airports = (pAirport)MapViewOfFile(objAirports, FILE_MAP_READ, 0, 0, maxAirports * sizeof(airport));

	if (airports == NULL) {
		CloseHandle(objAirports);
		_ftprintf(stderr, L"Impossível criar o map view.\n");
		return 0;
	}
	
	for (int i = 0; i < maxAirports; i++) {
		if (!_tcscmp(name, airports[i].name)) {
			plane->initial.x = airports[i].coordinates[X];
			plane->initial.y = airports[i].coordinates[Y];
			plane->current.x = airports[i].coordinates[X];
			plane->current.y = airports[i].coordinates[Y];
			break;
		}
	}
	UnmapViewOfFile(airports);
	CloseHandle(objAirports);

	if (plane->initial.x == -1) {
		return 0;
	}

	return 1;
}

int setDestinationAirport(TCHAR* destination, pPlane plane) {
	plane->final.x = -1;

	HANDLE objAirports = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, _T("airports"));

	if (objAirports == NULL) {
		_ftprintf(stderr, L"Impossível abrir o file mapping.\n");
		return 0;
	}

	int maxAirports = getMax(MAX_AIRPORTS, KEY_PATH);

	pAirport airports = (pAirport)MapViewOfFile(objAirports, FILE_MAP_READ, 0, 0, maxAirports * sizeof(airport));

	if (airports == NULL) {
		_ftprintf(stderr, L"Impossível criar o map view.\n");
		CloseHandle(objAirports);
		return 0;
	}

	HANDLE mutex = OpenMutex(SYNCHRONIZE, FALSE, _T("airportsMutex"));
	
	if (mutex == NULL) {
		_ftprintf(stderr, L"Não foi possível aceder aos aeroportos.\n");
		UnmapViewOfFile(airports);
		CloseHandle(objAirports);
		return 0;
	}
	WaitForSingleObject(mutex, INFINITE);

	for (int i = 0; i < maxAirports; i++) {
		if (!_tcscmp(destination, airports[i].name)) {
			if(plane->current.x != airports[i].coordinates[X] && plane->current.y != airports[i].coordinates[Y]) {
				plane->final.x = airports[i].coordinates[X];
				plane->final.y = airports[i].coordinates[Y];
				break;
			}
		}
	}
	ReleaseMutex(mutex);
	UnmapViewOfFile(airports);
	CloseHandle(objAirports);

	if (plane->final.x == -1) {
		return 0;
	}

	return 1;
}

int getArguments(int argc, LPTSTR argv[], pPlane plane) {

	if (argc != 4) {
		_ftprintf(stderr, L"Argumentos para lançar o avião insuficientes\n");
		return -1;
	}
	

	if (_stscanf_s(argv[1], L"%d", &plane->maxCapacity) == 0) {
		_ftprintf(stderr, L"Capacidade do avião inválida\n");
		return -1;
	}
	if (_stscanf_s(argv[2], L"%d", &plane->velocity) == 0) {
		_ftprintf(stderr, L"Velocidade do avião inválida\n");
		return -1;
	}
	if (!setInitialAirport(argv[3], plane)) {
		_ftprintf(stderr, L"Aeroporto inicial inválido...\n");
		return -1;
	}


	return 1;
}

void notifyController() {
	_tprintf(L"[DEBUG] Amigo controlador, cheguei, tá? \n"); //TODO: Avisar o controlador que cheguei ou atualizar a minha posicao
}

void setPosition(pMap map, int x, int y, int value) {
	HANDLE mutex = OpenMutex(SYNCHRONIZE, FALSE, _T("mapMutex"));
	if (mutex == NULL) {
		_ftprintf(stderr, L"Não foi possível atualizar o mapa.\n");
		return;
	}
	WaitForSingleObject(mutex, INFINITE);
	map->matrix[x][y] = value;
	ReleaseMutex(mutex);
}

int getValueOnPosition(pMap map, int x, int y) {
	HANDLE mutex = OpenMutex(SYNCHRONIZE, FALSE, _T("mapMutex"));
	if (mutex == NULL) {
		_ftprintf(stderr, L"Não foi possível aceder ao mapa.\n");
		return -1;
	}
	WaitForSingleObject(mutex, INFINITE);
	int value = map->matrix[x][y];
	ReleaseMutex(mutex);
	return value;
}

void initTrip(pPlane plane) {
	EnterCriticalSection(&plane->criticalSection);
	plane->ongoingTrip = 1;
	int auxOngoingTrip = 1; // para impedir acesso de leitura à critical section
	LeaveCriticalSection(&plane->criticalSection);
	
	_tprintf(L"[DEBUG] Trip começou \n");
	HMODULE dll = LoadLibraryEx(_T("SO2_TP_DLL_2021"), NULL, 0);

	if (dll == NULL) {
		_ftprintf(stderr, L"Impossível encontrar a DLL.\n");
		return;
	}

	FARPROC genericPointer = GetProcAddress(dll, "move");

	if (genericPointer == NULL) {
		_ftprintf(stderr, L"Impossível encontrar o método move.\n");
		return;
	}

	int(*move)(int, int, int, int, int*, int*) = (int(*)(int, int, int, int, int*, int*))genericPointer;

	HANDLE objMap = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, _T("map"));

	if (objMap == NULL) {
		_ftprintf(stderr, L"Impossível abrir o file mapping.\n");
		return; 
	}

	pMap map = (pMap)MapViewOfFile(objMap, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Map));
	 
	if (map == NULL) {
		_ftprintf(stderr, _T("Impossível criar o map view.\n"));
		return;
	}
	
	setPosition(map, plane->current.x, plane->current.y, 1); // Ocupa a posição de sobrevoar o aeroporto.
	
	do {
		 Sleep(1000 / (DWORD)plane->velocity);

		int nextX = 0, nextY = 0;
		int result = move(plane->current.x, plane->current.y, plane->final.x, plane->final.y, &nextX, &nextY);

		if (result == 2) {
			continue;
		}
		else if (result == 0) {
			_tprintf(L"[DEBUG] Cheguei ao meu destino, tenho que avisar o controlador \n"); 
			notifyController();
			break;
		}
		else {
			HANDLE mutex = OpenMutex(SYNCHRONIZE, FALSE, _T("mapMutex"));
			if (mutex == NULL) {
				_ftprintf(stderr, L"Não foi possível aceder ao mapa.\n");
				continue;
			}
			WaitForSingleObject(mutex, INFINITE);
			if (map->matrix[nextX][nextY] == 0 || map->matrix[nextX][nextY] == 2) {
				if (map->matrix[nextX][nextY] != 2) {
					map->matrix[plane->current.x][plane->current.y] = 0;
				}
				plane->current.x = nextX;
				plane->current.y = nextY;
				map->matrix[nextX][nextY] = 1;
			}
			else {
				//TODO: Podemos melhorar esta estrategia, para já estamos só a aguardar que os gordos desocupem a loja
			}
			ReleaseMutex(mutex);
		}
		EnterCriticalSection(&plane->criticalSection);
		auxOngoingTrip = plane->ongoingTrip;
		LeaveCriticalSection(&plane->criticalSection);

	} while (auxOngoingTrip);
	_tprintf(L"[DEBUG] Trip terminou \n");
	EnterCriticalSection(&plane->criticalSection);
	plane->ongoingTrip = 0;
	LeaveCriticalSection(&plane->criticalSection);
	FreeLibrary(dll);
	UnmapViewOfFile(map);
	CloseHandle(objMap);
}


void initTripThread(pPlane plane) {
	plane->tripThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)initTrip, (LPVOID) plane, 0, NULL);
	if (plane->tripThread == NULL) {
		_ftprintf(stderr, L"Não foi possível criar a thread.\n");
	}
}

int processCommand(TCHAR* command, pPlane plane) {

	if (!_tcscmp(command, TEXT("exit"))) {
		EnterCriticalSection(&plane->criticalSection);
		plane->ongoingTrip = 0;
		LeaveCriticalSection(&plane->criticalSection);
		WaitForSingleObject(plane->tripThread, INFINITE);
		return 0;
	}

	EnterCriticalSection(&plane->criticalSection);
	int auxOngoingTrip = plane->ongoingTrip;
	LeaveCriticalSection(&plane->criticalSection);

	if (auxOngoingTrip == 1) {
		_ftprintf(stderr, L"Viagem a decorrer. Aguarde.\n");
		return 1;
	}

	if (!_tcscmp(command, TEXT("iniciar"))) {
		initTripThread(plane);
	}

	TCHAR aux[SIZE];
	wcscpy_s(aux, SIZE, command);

	TCHAR destination[SIZE];
	int parameters = swscanf_s(aux, L"%s %s", command, SIZE, destination, SIZE);
	if (parameters != 2) {
		return 1;
	}

	if (!_tcscmp(command, TEXT("destino"))) {
		setDestinationAirport(destination, plane);
	}
	
	return 1;
}

int registerPlaneInController(pPlane plane) {
	//TODO: Comunicar ao controlador que existo
	int found = 0;

	HANDLE objPlanes = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, _T("planes"));

	if (objPlanes == NULL) {
		_ftprintf(stderr, L"Impossível abrir o file mapping.\n");
		return 0;
	}

	int maxPlanes = getMax(MAX_AIRPLANES, KEY_PATH);

	pPlane planes = (pPlane)MapViewOfFile(objPlanes, FILE_MAP_ALL_ACCESS, 0, 0, maxPlanes * sizeof(Plane));

	if (planes == NULL) {
		_ftprintf(stderr, L"Impossível criar o map view.\n");
		return 0;
	}

	for (int i = 0; i < maxPlanes; i++) {
		_ftprintf(stderr, L"%d: %d\n", i, planes[i].velocity);
		if (planes[i].velocity == -1) {
			planes[i] = *plane;
			found = 1;
			break;
		}
	}

	if (!found)
		return 0;

	return 1;
}

int _tmain(int argc, LPTSTR argv[]) {

#ifdef UNICODE
	(void)_setmode(_fileno(stdin), _O_WTEXT);
	(void)_setmode(_fileno(stdout), _O_WTEXT);
	(void)_setmode(_fileno(stderr), _O_WTEXT);
#endif
	CreateMutexA(0, FALSE, "Local\\$controlador$"); 
	if (GetLastError() != ERROR_ALREADY_EXISTS) {  
		_ftprintf(stderr, _T("O controlador não está a correr."));
		return -1;								 
	}

	_tprintf(L"Pedido de registo à torre de controlo...");

	HANDLE semaphore = OpenSemaphore(SEMAPHORE_MODIFY_STATE | SYNCHRONIZE, FALSE, _T("planeEntryControl"));

	if (semaphore == NULL) {
		_ftprintf(stderr, _T("Impossível abrir semáforo."));
		return -1;
	}

	WaitForSingleObject(semaphore, INFINITE);

	Plane plane;

	if (getArguments(argc, argv, &plane) == -1) {
		return 1;
	}

	if (!registerPlaneInController(&plane)) {
		_tprintf(TEXT("Problemas a registar o avião no controlador.\n"));
		return 1;
	}

	InitializeCriticalSection(&plane.criticalSection);

	_tprintf(L"[DEBUG] Cap: %d - Vel: %d\n", plane.maxCapacity, plane.velocity);
	
	TCHAR command[SIZE];
	do {
		_tprintf(L"Estou nas coordenadas: %d, %d \n ", plane.initial.x, plane.initial.y);
		_tprintf(L"-> ");
		_fgetts(command, SIZE, stdin);
		command[_tcslen(command) - 1] = '\0';
	} while (processCommand(command, &plane) != 0);

	DeleteCriticalSection(&plane.criticalSection);
	if (ReleaseSemaphore(semaphore, 1, NULL) == 0) {
		_ftprintf(stderr, _T("Impossível libertar o semáforo."));
	}
	CloseHandle(semaphore);
	return 0;
}
