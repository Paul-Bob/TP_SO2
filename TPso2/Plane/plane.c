#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#include "plane.h"

#define SIZE 200

void notifyController(pData data, enum messageType type) {
	WaitForSingleObject(data->emptiesSemaphore, INFINITE);
	WaitForSingleObject(data->producerMutex, INFINITE);

	data->producerConsumer->buffer[data->producerConsumer->in].type = type;
	data->producerConsumer->buffer[data->producerConsumer->in].planeID = data->plane->planeID;
	data->producerConsumer->buffer[data->producerConsumer->in].index = data->plane->index;
	data->producerConsumer->in = (data->producerConsumer->in + 1) % DIM_BUFFER;

	ReleaseMutex(data->producerMutex);
	ReleaseSemaphore(data->itemsSemaphore, 1, NULL);
}

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
	data->plane->initial.x = -1;

	data->objAirports = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, _T("airports"));

	if (data->objAirports == NULL) {
		_ftprintf(stderr, L"Impossível abrir o file mapping.\n");
		return 0;
	}

	data->maxAirports = getMax(MAX_AIRPORTS, KEY_PATH);

	data->airports = (pAirport)MapViewOfFile(data->objAirports, FILE_MAP_READ, 0, 0, data->maxAirports * sizeof(airport));

	if (data->airports == NULL) {
		CloseHandle(data->objAirports);
		_ftprintf(stderr, L"Impossível criar o map view.\n");
		return 0;
	}
	
	for (int i = 0; i < data->maxAirports; i++) {
		if (!_tcscmp(name, data->airports[i].name)) {
			data->plane->initial.x = data->airports[i].coordinates[X];
			data->plane->initial.y = data->airports[i].coordinates[Y];
			data->plane->current.x = data->airports[i].coordinates[X];
			data->plane->current.y = data->airports[i].coordinates[Y];
			_tcscpy_s(data->plane->actualAirport, NAMESIZE, name);
			_tcscpy_s(data->plane->destinAirport, NAMESIZE, _T("NULL"));
			break;
		}
	}

	if (data->plane->initial.x == -1) {
		return 0;
	}

	return 1;
}

int setDestinationAirport(TCHAR* destination, pData data) {
	data->plane->final.x = -1;

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
			if(data->plane->current.x != airports[i].coordinates[X] && data->plane->current.y != airports[i].coordinates[Y]) {
				data->plane->final.x = airports[i].coordinates[X];
				data->plane->final.y = airports[i].coordinates[Y];
				_tcscpy_s(data->plane->destinAirport, NAMESIZE, destination);
				_tprintf(TEXT("Aeroporto destino alterado para '%s'\n\n"),destination);
				break;
			}
		}
	}
	ReleaseMutex(mutex);
	UnmapViewOfFile(airports);
	CloseHandle(objAirports);

	if (data->plane->final.x == -1) {
		_tprintf(TEXT("Aeroporto destino '%s' inválido\n\n"), destination);
		return 0;
	}

	return 1;
}

int getArguments(int argc, LPTSTR argv[], pData data) {

	if (argc != 4) {
		_ftprintf(stderr, L"Lançamento avião: %s lotação velocidade aeroportoInicial\n", argv[0]);
		return -1;
	}
	

	if (_stscanf_s(argv[1], L"%d", &data->plane->maxCapacity) == 0) {
		_ftprintf(stderr, L"Capacidade do avião inválida\n");
		return -1;
	}
	if (_stscanf_s(argv[2], L"%d", &data->plane->velocity) == 0) {
		_ftprintf(stderr, L"Velocidade do avião inválida\n");
		return -1;
	}
	if (!setInitialAirport(argv[3], data)) {
		_ftprintf(stderr, L"Aeroporto inicial inválido...\n");
		return -1;
	}
	return 1;
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

	int valid = 0;
	for (int i = 0; i < data->maxAirports; i++) {
		//se o destino existe e é diferente do atual entao prosegue
		if (!_tcscmp(data->plane->destinAirport, data->airports[i].name) && _tcscmp(data->plane->destinAirport, data->plane->actualAirport)) {
			valid = 1;
			break;
		}
	}

	if (!valid) {
		_tprintf(L"Aeroporto destino fail. \n");
		return;
	}

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

	EnterCriticalSection(&data->criticalSection);
	data->ongoingTrip = 1;
	int auxOngoingTrip = 1; // para impedir acesso de leitura à critical section
	LeaveCriticalSection(&data->criticalSection);

	_tprintf(L"Descolagem efetuada, atualmente a voar em direção ao aeroporto '%s'\n\n",data->plane->destinAirport);
	_tprintf(L"-> ");

	_tcscpy_s(data->plane->departureAirport, NAMESIZE, data->plane->actualAirport);
	_tcscpy_s(data->plane->actualAirport, NAMESIZE, _T("Fly"));

	notifyController(data, Departure);

	//->>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!<<<<<<<<<<<<<<<<<<<<<<<<<<<<<-------------------------------------------------
	//setPosition(map, data->plane->current.x, data->plane->current.y, 1); // Ocupa a posição de sobrevoar o aeroporto. Isto nao tira a indicação de aeroporto no mapa?!

	do {
		 Sleep(1000 / (DWORD)data->plane->velocity);

		int nextX = 0, nextY = 0;
		int result = move(data->plane->current.x, data->plane->current.y, data->plane->final.x, data->plane->final.y, &nextX, &nextY);

		if (result == 2) {
			continue;
		}
		else if (result == 0) {
			_tprintf(L"Aterragem efetuada com sucesso no aeroporto '%s'\n\n",data->plane->destinAirport); //TODO: Limpar o final
			_tprintf(L"-> ");
			_tcscpy_s(data->plane->actualAirport, NAMESIZE, data->plane->destinAirport);
			_tcscpy_s(data->plane->destinAirport, NAMESIZE, _T("NULL"));
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
				if (map->matrix[data->plane->current.x][data->plane->current.y] != 2) {
					map->matrix[data->plane->current.x][data->plane->current.y] = 0;
				}
				data->plane->current.x = nextX;
				data->plane->current.y = nextY;
				map->matrix[nextX][nextY] = 1;
			}
			else {
				if (map->matrix[nextX][nextY - 1] == 0) {
					data->plane->current.x = nextX;
					data->plane->current.y = nextY - 1;
					map->matrix[nextX][nextY - 1] = 1;
				}
				else
					_tprintf(_T("Não há espaço no ceu!"));
			}
			ReleaseMutex(mutex);
		}
		EnterCriticalSection(&data->criticalSection);
		auxOngoingTrip = data->ongoingTrip;
		LeaveCriticalSection(&data->criticalSection);

	} while (auxOngoingTrip);
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
	notifyController(data, Register);
	_tprintf(L"Conexão com a torre de controlo efetuada com sucesso...\n\n");
	data->heartbeatThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)heartbeat, (LPVOID)data, 0, NULL);
	if (data->heartbeatThread == NULL) {
		_ftprintf(stderr, L"Não foi possível criar a thread do heartbeat.\n");
	}
}

int processCommand(TCHAR* command, pData data) {
	_puttchar(L'\n');

	if (!_tcscmp(command, TEXT("exit"))) {
		EnterCriticalSection(&data->criticalSection);
		data->ongoingTrip = 0;
		LeaveCriticalSection(&data->criticalSection);
		WaitForSingleObject(data->tripThread, INFINITE);
		return 0;
	}

	if (!_tcscmp(command, TEXT("help"))) {
		_tprintf(TEXT("Comandos disponiveis:\n"));
		_tprintf(TEXT("- info                 --> Info avião\n"));
		_tprintf(TEXT("- iniciar              --> Inicia viagem.\n"));
		_tprintf(TEXT("- destino [aeroporto]  --> Define aeroporto destino\n"));
		_tprintf(TEXT("- exit                 --> Encerra o avião\n\n"));
		return 1;
	}

	if (!_tcscmp(command, TEXT("info"))) {
		_tprintf(L"ID Avião:  %d\nCoordenadas: [%d,%d]\nCapacidade: %d\nVelocidade: %d\nPassageiros embarcados: 0\n",
			data->plane->planeID, data->plane->current.x, data->plane->current.y, data->plane->maxCapacity, data->plane->velocity);
		if(!_tcscmp(data->plane->actualAirport,_T("Fly")))
			_tprintf(TEXT("Atualmente em voo com destino ao aeroporto '%s'\n"), data->plane->destinAirport);
		else
			_tprintf(TEXT("Em repouso no aeroporto '%s'\n"), data->plane->actualAirport);
		if (!_tcscmp(data->plane->destinAirport, _T("NULL")))
			_tprintf(TEXT("Aeroporto destino não definido.\n\n"));
		else
			_tprintf(TEXT("Aeroporto destino: '%s'.\n\n"), data->plane->destinAirport);
		return 1;
	}

	EnterCriticalSection(&data->criticalSection);
	int auxOngoingTrip = data->ongoingTrip;
	LeaveCriticalSection(&data->criticalSection);

	if (auxOngoingTrip == 1) {
		_ftprintf(stderr, L"Viagem a decorrer. Aguarde.\n\n");
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

	data->objPlanes = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, _T("planes"));

	if (data->objPlanes == NULL) {
		_ftprintf(stderr, L"Impossível abrir o file mapping.\n");
		return 0;
	}

	int maxPlanes = getMax(MAX_AIRPLANES, KEY_PATH);

	data->planes = (pPlane)MapViewOfFile(data->objPlanes, FILE_MAP_ALL_ACCESS, 0, 0, maxPlanes * sizeof(Plane));

	if (data->planes == NULL) {
		_ftprintf(stderr, L"Impossível criar o map view.\n");
		return 0;
	}

	for (int i = 0; i < maxPlanes; i++) {
		if (data->planes[i].velocity == -1) {
			data->plane = &data->planes[i];
			data->plane->heartbeatTimer = NULL;
			data->plane->index = i;
			data->plane->planeID = GetCurrentProcessId();
			found = 1;
			break;
		}
	}

	if (!found)
		return 0;

	return 1;
}

void closePlane(pData data) {
	//ReleaseSemaphore(data->controlSemaphore, 1, NULL);
	CloseHandle(data->controlSemaphore);
	DeleteCriticalSection(&data->criticalSection);
	CloseHandle(data->producerMutex);
	UnmapViewOfFile(data->producerConsumer);
	CloseHandle(data->objProducerConsumer);
	CloseHandle(data->emptiesSemaphore);
	CloseHandle(data->itemsSemaphore);
	UnmapViewOfFile(data->airports);
	CloseHandle(data->objAirports);
	UnmapViewOfFile(data->planes);
	CloseHandle(data->objPlanes);
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

	Data data;

	data.controlSemaphore = OpenSemaphore(SEMAPHORE_MODIFY_STATE | SYNCHRONIZE, FALSE, _T("planeEntryControl"));

	if (data.controlSemaphore == NULL) {
		_ftprintf(stderr, _T("Impossível abrir semáforo."));
		closePlane(&data);
		return 1;
	}

	WaitForSingleObject(data.controlSemaphore, INFINITE);

	if (!registerPlaneInController(&data)) {
		_tprintf(TEXT("Problemas a registar o avião no controlador.\n"));
		closePlane(&data);
		return 1;
	}

	if (getArguments(argc, argv, &data) == -1) {
		closePlane(&data);
		return 1;
	}

	_tprintf(L"Pedido de registo à torre de controlo...\n");

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

	_tprintf(L"ID Avião:  %d\nCapacidade: %d\nVelocidade: %d\nAeroporto: %s\nComando 'help' para mais informações.\n\n",
		data.plane->planeID, data.plane->maxCapacity, data.plane->velocity, data.plane->actualAirport);



	TCHAR command[SIZE];
	do {
		_tprintf(L"-> ");
		_fgetts(command, SIZE, stdin);
		command[_tcslen(command) - 1] = '\0';
	} while (processCommand(command, &data) != 0);
	
	closePlane(&data);
	return 0;
}
