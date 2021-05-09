#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#include "plane.h"

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

int setInitialAirport(TCHAR* name, pData data) {
	data->plane.initial.x = -1;

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
			data->plane.initial.x = airports[i].coordinates[X];
			data->plane.initial.y = airports[i].coordinates[Y];
			data->plane.current.x = airports[i].coordinates[X];
			data->plane.current.y = airports[i].coordinates[Y];
			break;
		}
	}
	UnmapViewOfFile(airports);
	CloseHandle(objAirports);

	if (data->plane.initial.x == -1) {
		return 0;
	}

	return 1;
}

int setDestinationAirport(TCHAR* destination, pData data) {
	data->plane.final.x = -1;

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
		if (!_tcscmp(destination, airports[i].name)) { //TODO: mensagem de erro
			if(data->plane.current.x != airports[i].coordinates[X] && data->plane.current.y != airports[i].coordinates[Y]) {
				data->plane.final.x = airports[i].coordinates[X];
				data->plane.final.y = airports[i].coordinates[Y];
				break;
			}
		}
	}
	ReleaseMutex(mutex);
	UnmapViewOfFile(airports);
	CloseHandle(objAirports);

	if (data->plane.final.x == -1) {
		return 0;
	}

	return 1;
}

int getArguments(int argc, LPTSTR argv[], pData data) {

	if (argc != 4) {
		_ftprintf(stderr, L"Argumentos para lançar o avião insuficientes\n");
		return -1;
	}
	

	if (_stscanf_s(argv[1], L"%d", &data->plane.maxCapacity) == 0) {
		_ftprintf(stderr, L"Capacidade do avião inválida\n");
		return -1;
	}
	if (_stscanf_s(argv[2], L"%d", &data->plane.velocity) == 0) {
		_ftprintf(stderr, L"Velocidade do avião inválida\n");
		return -1;
	}
	if (!setInitialAirport(argv[3], data)) {
		_ftprintf(stderr, L"Aeroporto inicial inválido...\n");
		return -1;
	}
	return 1;
}

void notifyController(pData data, enum messageType type) {
	WaitForSingleObject(data->emptiesSemaphore, INFINITE);
	WaitForSingleObject(data->producerMutex, INFINITE);

	data->producerConsumer->buffer[data->producerConsumer->in].type = type;
	data->producerConsumer->buffer[data->producerConsumer->in].planeID = data->plane.planeID;
	data->producerConsumer->in = (data->producerConsumer->in + 1) % DIM_BUFFER;

	ReleaseMutex(data->producerMutex);
	ReleaseSemaphore(data->itemsSemaphore, 1, NULL);
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

void initTrip(pData data) {
	EnterCriticalSection(&data->criticalSection);
	data->ongoingTrip = 1;
	int auxOngoingTrip = 1; // para impedir acesso de leitura à critical section
	LeaveCriticalSection(&data->criticalSection);
	
	_tprintf(L"[DEBUG] Trip começou \n");

	notifyController(data, Departure);
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
	
	setPosition(map, data->plane.current.x, data->plane.current.y, 1); // Ocupa a posição de sobrevoar o aeroporto.
	
	do {
		 Sleep(1000 / (DWORD)data->plane.velocity);

		int nextX = 0, nextY = 0;
		int result = move(data->plane.current.x, data->plane.current.y, data->plane.final.x, data->plane.final.y, &nextX, &nextY);

		if (result == 2) {
			continue;
		}
		else if (result == 0) {
			_tprintf(L"[DEBUG] Cheguei ao meu destino, tenho que avisar o controlador \n"); //TODO: Limpar o final
			notifyController(data, Arrive);
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
					map->matrix[data->plane.current.x][data->plane.current.y] = 0;
				}
				data->plane.current.x = nextX;
				data->plane.current.y = nextY;
				map->matrix[nextX][nextY] = 1;
			}
			else {
				//TODO: Podemos melhorar esta estrategia, para já estamos só a aguardar que os gordos desocupem a loja
			}
			ReleaseMutex(mutex);
		}
		EnterCriticalSection(&data->criticalSection);
		auxOngoingTrip = data->ongoingTrip;
		LeaveCriticalSection(&data->criticalSection);

	} while (auxOngoingTrip);
	_tprintf(L"[DEBUG] Trip terminou \n");
	EnterCriticalSection(&data->criticalSection);
	data->ongoingTrip = 0;
	LeaveCriticalSection(&data->criticalSection);
	FreeLibrary(dll);
	UnmapViewOfFile(map);
	CloseHandle(objMap);
}

void heartbeat(pData data) {
	while (1) {
		notifyController(data, Heartbeat);
		Sleep(2500); // TODO: verificar se este é o melhor
	}
}

void initTripThread(pData data) {
	data->tripThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)initTrip, (LPVOID)data, 0, NULL);
	if (data->tripThread == NULL) {
		_ftprintf(stderr, L"Não foi possível criar a thread.\n");
	}
}

void initHeartbeatThread(pData data) {
	data->heartbeatThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)heartbeat, (LPVOID)data, 0, NULL);
	if (data->heartbeatThread == NULL) {
		_ftprintf(stderr, L"Não foi possível criar a thread do heartbeat.\n");
	}
}

int processCommand(TCHAR* command, pData data) {

	if (!_tcscmp(command, TEXT("exit"))) {
		EnterCriticalSection(&data->criticalSection);
		data->ongoingTrip = 0;
		LeaveCriticalSection(&data->criticalSection);
		WaitForSingleObject(data->tripThread, INFINITE);
		return 0;
	}

	EnterCriticalSection(&data->criticalSection);
	int auxOngoingTrip = data->ongoingTrip;
	LeaveCriticalSection(&data->criticalSection);

	if (auxOngoingTrip == 1) {
		_ftprintf(stderr, L"Viagem a decorrer. Aguarde.\n");
		return 1;
	}

	if (!_tcscmp(command, TEXT("iniciar"))) {
		initTripThread(data);
	}

	TCHAR aux[SIZE];
	wcscpy_s(aux, SIZE, command);

	TCHAR destination[SIZE];
	int parameters = swscanf_s(aux, L"%s %s", command, SIZE, destination, SIZE);
	if (parameters != 2) {
		return 1;
	}

	if (!_tcscmp(command, TEXT("destino"))) {
		setDestinationAirport(destination, data);
	}
	
	return 1;
}

int registerPlaneInController(pData data) {
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
			data->plane.heartbeatTimer = NULL;
			planes[i] = data->plane;
			found = 1;
			break;
		}
	}

	if (!found)
		return 0;

	return 1;
}

void closePlane(pData data) {
	ReleaseSemaphore(data->controlSemaphore, 1, NULL);
	CloseHandle(data->controlSemaphore);
	DeleteCriticalSection(&data->criticalSection);
	CloseHandle(data->producerMutex);
	UnmapViewOfFile(data->producerConsumer);
	CloseHandle(data->objProducerConsumer);
	CloseHandle(data->emptiesSemaphore);
	CloseHandle(data->itemsSemaphore);
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
		return 1;								 
	}

	_tprintf(L"Pedido de registo à torre de controlo...");
	
	Data data;

	data.controlSemaphore = OpenSemaphore(SEMAPHORE_MODIFY_STATE | SYNCHRONIZE, FALSE, _T("planeEntryControl"));

	if (data.controlSemaphore == NULL) {
		_ftprintf(stderr, _T("Impossível abrir semáforo."));
		closePlane(&data);
		return 1;
	}

	WaitForSingleObject(data.controlSemaphore, INFINITE);

	if (getArguments(argc, argv, &data) == -1) {
		closePlane(&data);
		return 1;
	}

	data.plane.planeID = GetCurrentProcessId();

	if (!registerPlaneInController(&data)) {
		_tprintf(TEXT("Problemas a registar o avião no controlador.\n"));
		closePlane(&data);
		return 1;
	}

	InitializeCriticalSection(&data.criticalSection);

	data.producerMutex = CreateMutex(0, FALSE, _T("producerMutex"));
	if (data.producerMutex == NULL) {
		_ftprintf(stderr, L"Não foi possível abrir o mutex do produtor.\n");
		closePlane(&data);
		return 1;
	} 


	data.objProducerConsumer = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, _T("producerConsumer"));

	if (data.objProducerConsumer == NULL) {
		_ftprintf(stderr, L"Impossível abrir o file mapping do produtor/consumidor.\n");
		closePlane(&data);
		return 1;
	}

	data.producerConsumer = (pProducerConsumer)MapViewOfFile(data.objProducerConsumer, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(ProducerConsumer));

	if (data.producerConsumer == NULL) {
		_ftprintf(stderr, L"Impossível criar o map view do produtor/consumidor.\n");
		closePlane(&data);
		return 1;
	}

	data.emptiesSemaphore = OpenSemaphore(SEMAPHORE_MODIFY_STATE | SYNCHRONIZE, FALSE, _T("emptiesSemaphore"));

	if (data.emptiesSemaphore == NULL) {
		_ftprintf(stderr, _T("Impossível abrir semáforo dos items vazios."));
		closePlane(&data);
		return 1;
	}

	data.itemsSemaphore = OpenSemaphore(SEMAPHORE_MODIFY_STATE | SYNCHRONIZE, FALSE, _T("itemsSemaphore"));

	if (data.itemsSemaphore == NULL) {
		_ftprintf(stderr, _T("Impossível abrir semáforo dos items."));
		closePlane(&data);
		return 1;
	}
	
	initHeartbeatThread(&data);

	TCHAR command[SIZE];
	do {
		_tprintf(L"-> ");
		_fgetts(command, SIZE, stdin);
		command[_tcslen(command) - 1] = '\0';
	} while (processCommand(command, &data) != 0);
	
	closePlane(&data);
	return 0;
}
