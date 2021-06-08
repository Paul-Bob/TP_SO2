#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>

#include "../HF/structs.h"
#include "prepareEnv.h"
#include "textUI.h"
#include "sharedMemory.h"

#define SIZE 200
#define BUFSIZE 2048

void printConsumedInfo(Protocol message, pPlane plane) {
	switch (message.type) {
	case Arrive:
		_tprintf(L"O avião %d aterrou com sucesso no aeroporto '%s'\n\n", message.planeID, plane->actualAirport); 
		_tprintf(L"-> ");
		break;
	case Departure:
		_tprintf(L"O avião %d descolou com sucesso do aeroporto '%s' e tem como destino o aeroporto '%s'\n\n",
			message.planeID, plane->departureAirport, plane->destinAirport);
		_tprintf(L"-> ");
		break;
	case Register:
		_tprintf(_T("\n\nNovo registo de avião!\n"));
		_tprintf(_T("Avião ID   : %d\n"), message.planeID);
		_tprintf(_T("Aeroporto  : %s\n"), plane->actualAirport);
		_tprintf(_T("Velocidade : %d\n"), plane->velocity);
		_tprintf(_T("Capacidade : %d\n\n"), plane->maxCapacity);
		_tprintf(L"-> ");
		break;
	}
}

void removePlane(pRemovePlane removeData) {
	//_tprintf(L"[DEBUG] Remove\n");
	if (removeData == NULL) {
		return;
	}

	WaitForSingleObject(removeData->plane->heartbeatTimer, INFINITE);

	int x = removeData->plane->current.x;
	int y = removeData->plane->current.y;

	if (x > 0 && x < MAPSIZE && y > 0 && y < MAPSIZE) {
		removeData->map->matrix[x][y] = 0; // Limpa o mapa
	}
	removeData->plane->velocity = -1; // Torna o espaço novamente vazio;
	removeData->plane->current.x = -1;
	removeData->plane->current.y = -1;
	removeData->plane->initial.x = -1;
	removeData->plane->initial.y = -1;
	removeData->plane->final.x = -1;
	removeData->plane->final.y = -1;
	removeData->plane->maxCapacity = -1;
	if (removeData->plane->heartbeatTimer != NULL) {
		CloseHandle(removeData->plane->heartbeatTimer);
	}

	//_tprintf(L"[DEBUG] Removi\n");
	_tprintf(L"ALERTA!!\n");
	_tprintf(L"Conexão PERDIDA!\n");
	_tprintf(_T("Avião ID   : %d\n\n"), removeData->plane->planeID);
	removeData->plane->planeID = -1;
	ReleaseSemaphore(removeData->controll, 1, NULL);
	free(removeData);
}

void initPlaneTimerThread(pRemovePlane removePlaneData) {
	//_tprintf(L"[DEBUG] Vou criar a thread do timer \n");
	HANDLE thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)removePlane, (LPVOID)removePlaneData, 0, NULL);
	if (thread == NULL) {
		_ftprintf(stderr, L"Não foi possível criar a thread do timer.\n");
	}
}

void producerConsumer(pDATA data) {
	//_tprintf(L"[DEBUG] Estou na thread \n");
	int planePos;
	while (1) {
		//_tprintf(L"[DEBUG] Estou a espera de coisas \n");
		WaitForSingleObject(data->itemsSemaphore, INFINITE);
		Protocol message = data->producerConsumer->buffer[data->producerConsumer->out];
		data->producerConsumer->out = (data->producerConsumer->out + 1) % DIM_BUFFER;
		ReleaseSemaphore(data->emptiesSemaphore, 1, NULL);

		planePos = message.index;

		if (message.type == Arrive || message.type == Departure || message.type == Register) {
			printConsumedInfo(message,&data->planes[planePos]);
		}
		else {
			//_tprintf(L"[DEBUG] Heartbeat %d \n, ", message.planeID);
			if(data->planes[planePos].heartbeatTimer == NULL) {
				data->planes[planePos].heartbeatTimer = CreateWaitableTimer(NULL, TRUE, NULL);
				pRemovePlane removePlaneData = malloc(sizeof(RemovePlane));
				if (removePlaneData == NULL) 
					continue;
				removePlaneData->map = data->map;
				removePlaneData->plane = &data->planes[planePos];
				removePlaneData->controll = data->controlPlanes;
				initPlaneTimerThread(removePlaneData);
			}

			if(data->planes[planePos].heartbeatTimer != 0) {
				LARGE_INTEGER time;
				time.QuadPart = -30000000LL;
				SetWaitableTimer(data->planes[planePos].heartbeatTimer, &time, 0, NULL, NULL, FALSE);
			}
		}
	}
}

void initProducerConsumerThread(pDATA data) {
	//_tprintf(L"[DEBUG] Vou criar a thread \n");
	data->producerConsumerThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)producerConsumer, (LPVOID) data, 0, NULL);
	if (data->producerConsumerThread == NULL) {
		_ftprintf(stderr, L"Não foi possível criar a thread do produtor consumidor.\n");
	}
}

void passengerRegister(pDATA data) {
	while (1) {
		HANDLE hPipe = CreateNamedPipe(
			PIPENAME,
			PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
			PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
			PIPE_UNLIMITED_INSTANCES,
			BUFSIZE,
			BUFSIZE,
			100000,
			NULL);
		if(hPipe == INVALID_HANDLE_VALUE) {
			_ftprintf(stderr, L"Não foi possível criar o pipe de registo.\n");
			return;
		}

		_tprintf(L"[DEBUG] Controlador à espera que passageiros se registem...\n");

		BOOL connected = ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);

		if (connected) {
			// criar thread passageiro e guardar pipe		
			_tprintf(L"[DEBUG] Passageiro connectado\n");
		}
		else {
			CloseHandle(hPipe);
		}

	}
}

void initControlPassengerRegisterThread(pDATA data) {
	_tprintf(L"[DEBUG] Vou criar a thread do registo\n");

	data->controlPassengerRegisterThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)passengerRegister, (LPVOID)data, 0, NULL);
	if (data->controlPassengerRegisterThread == NULL) {
		_ftprintf(stderr, L"Não foi possível criar a thread do produtor consumidor.\n");
	}
}


int _tmain(int argc, TCHAR* argv[]) {
	TCHAR command[SIZE];
	pDATA data;

	data = malloc(sizeof(DATA));
	if (data == NULL) {
		_ftprintf(stderr, L"Memorry alloc fail for data\n");
		return -1;
	}

	data->nrAirplanes = 0;

	//previne poder ter mais do que uma instância do mesmo programa a correr em simultâneo.
	CreateMutexA(0, FALSE, "Local\\$controlador$"); // try to create a named mutex
	if (GetLastError() == ERROR_ALREADY_EXISTS) {   // did the mutex already exist?
		puts("Already running!");
		return -1;								  // quit; mutex is released automatically
	}

#ifdef UNICODE
	(void)_setmode(_fileno(stdin), _O_WTEXT);
	(void)_setmode(_fileno(stdout), _O_WTEXT);
	(void)_setmode(_fileno(stderr), _O_WTEXT);
#endif

	//Coloca as variaveis no registry caso não estejam onde é esperado...
	if (!prepareEnvironment(KEY_PATH)) {
		puts("Registry fail!");
		return -1;
	}

	//Obtém variáveis que supostamente estão no Registry
	data->maxAirplanes = getMax(MAX_AIRPLANES,KEY_PATH);
	data->maxAirports  = getMax(MAX_AIRPORTS,KEY_PATH);
	if (!data->maxAirplanes || !data->maxAirports) {
		_tprintf(L"Registry vars fail\n");
		return -1;
	}

	//estas funções internamente fornecem mais informações caso algo falhe
	if (!createAirportSpace(data) || !createMap(data) || !createAirplaneSpace(data) || !createProducerConsumer(data)) {
		_ftprintf(stderr, L"File mapping fail\n");
		return -1;
	}

	_tprintf(L"Max airports do registry :  %d\nMax airplanes do registry : %d\nComando 'help' para mais informações.\n\n",
		data->maxAirports,data->maxAirplanes);
	
	initProducerConsumerThread(data);
	initControlPassengerRegisterThread(data);

	do {
		_tprintf(L"-> ");
		_fgetts(command, SIZE, stdin);
		command[_tcslen(command) - 1] = '\0';
		interpretaComandoControlador(command,data);
	} 	while (_tcscmp(command, L"exit"));

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

	return 0;
}
