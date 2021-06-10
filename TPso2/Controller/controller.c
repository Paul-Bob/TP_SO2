#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>

#include "../HF/structs.h"
#include "prepareEnv.h"
#include "textUI.h"
#include "sharedMemory.h"
#include "controller.h"

#define SIZE 200
#define BUFSIZE 2048



void printConsumedInfo(Protocol message, pPlane plane, HWND hwndList) {
	TCHAR myMessage[500] = _T("");
	if(hwndList == NULL)
		switch (message.type) {
		case Arrive:
			_tprintf(L"O avi�o %d aterrou com sucesso no aeroporto '%s'\n\n", message.planeID, plane->actualAirport); 
			_tprintf(L"-> ");
			break;
		case Departure:
			_tprintf(L"O avi�o %d descolou com sucesso do aeroporto '%s' e tem como destino o aeroporto '%s'\n\n",
				message.planeID, plane->departureAirport, plane->destinAirport);
			_tprintf(L"-> ");
			break;
		case Register:
			_tprintf(_T("\n\nNovo registo de avi�o!\n"));
			_tprintf(_T("Avi�o ID   : %d\n"), message.planeID);
			_tprintf(_T("Aeroporto  : %s\n"), plane->actualAirport);
			_tprintf(_T("Velocidade : %d\n"), plane->velocity);
			_tprintf(_T("Capacidade : %d\n\n"), plane->maxCapacity);
			_tprintf(L"-> ");
			break;
		}
	else
		switch (message.type) {
		case Arrive:
			SendMessage(hwndList, LB_ADDSTRING, 0, _T(""));
			_stprintf_s(myMessage, 500, _TEXT("O avi�o %d aterrou com sucesso no aeroporto '%s'"), message.planeID, plane->actualAirport);
			SendMessage(hwndList, LB_ADDSTRING, 0, myMessage);
			break;
		case Departure:
			SendMessage(hwndList, LB_ADDSTRING, 0, _T(""));

			_stprintf_s(myMessage, 500, _TEXT("O avi�o %d descolou com sucesso do aeroporto '%s' e tem como destino o aeroporto '%s'"),
				message.planeID, plane->departureAirport, plane->destinAirport);
			SendMessage(hwndList, LB_ADDSTRING, 0, myMessage);
			break;
		case Register:
			SendMessage(hwndList, LB_ADDSTRING, 0, _T(""));

			SendMessage(hwndList, LB_ADDSTRING, 0, _T("Novo registo de avi�o!"));
			_stprintf_s(myMessage, 500, _T("Avi�o ID   : %d\n"), message.planeID);
			SendMessage(hwndList, LB_ADDSTRING, 0, myMessage);
			_stprintf_s(myMessage, 500, _T("Aeroporto  : %s\n"), plane->actualAirport);
			SendMessage(hwndList, LB_ADDSTRING, 0, myMessage);
			_stprintf_s(myMessage, 500, _T("Velocidade : %d\n"), plane->velocity);
			SendMessage(hwndList, LB_ADDSTRING, 0, myMessage);
			_stprintf_s(myMessage, 500, _T("Capacidade : %d\n\n"), plane->maxCapacity);
			SendMessage(hwndList, LB_ADDSTRING, 0, myMessage);
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
	removeData->plane->velocity = -1; // Torna o espa�o novamente vazio;
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
	_tprintf(L"Conex�o PERDIDA!\n");
	_tprintf(_T("Avi�o ID   : %d\n\n"), removeData->plane->planeID);
	removeData->plane->planeID = -1;
	ReleaseSemaphore(removeData->controll, 1, NULL);
	free(removeData);
}

void initPlaneTimerThread(pRemovePlane removePlaneData) {
	//_tprintf(L"[DEBUG] Vou criar a thread do timer \n");
	HANDLE thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)removePlane, (LPVOID)removePlaneData, 0, NULL);
	if (thread == NULL) {
		_ftprintf(stderr, L"N�o foi poss�vel criar a thread do timer.\n");
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
			printConsumedInfo(message,&data->planes[planePos],data->hwndList);
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
		_ftprintf(stderr, L"N�o foi poss�vel criar a thread do produtor consumidor.\n");
	}
}

int writeOnPipe(HANDLE pipe, PassengerProtocol message) {
	HANDLE writeReadyEvent = CreateEvent(
		NULL,
		TRUE,
		FALSE,
		NULL);

	if (writeReadyEvent == NULL) {
		_ftprintf(stderr, L"N�o foi poss�vel criar o evento\n");
		return -1;
	}

	OVERLAPPED overlappedWriteReady = { 0 };

	ZeroMemory(&overlappedWriteReady, sizeof(overlappedWriteReady));
	ResetEvent(writeReadyEvent);
	overlappedWriteReady.hEvent = writeReadyEvent;

	DWORD numberBytesWritten;

	BOOL success = WriteFile(
		pipe,
		&message,
		sizeof(PassengerProtocol),
		&numberBytesWritten,
		&overlappedWriteReady);

	WaitForSingleObject(writeReadyEvent, INFINITE);
	_tprintf(L"[DEBUG] Mensagem Enviada.\n");
	return 1;
}

void processMessageFromPassenger(pPipeAndData pipeAndData, PassengerProtocol message) {
	switch (message.type) {
	case RegisterPassenger: {
		BOOL origin = FALSE;
		BOOL destiny = FALSE;
		for (int i = 0; i < pipeAndData->data->maxAirports; i++) { //TODO: FALTA MUTEX
			if (!_tcscmp(message.passenger.origin, pipeAndData->data->airports[i].name)) {
				origin = TRUE;
			} else if (!_tcscmp(message.passenger.destiny, pipeAndData->data->airports[i].name)) {
				destiny = TRUE;
			}
		}

		PassengerProtocol newMessage;
		newMessage.type = RegisterPassenger;
		newMessage.success = (origin && destiny);
		writeOnPipe(pipeAndData->pipe, newMessage);

		if (origin && destiny) {
			//TODO: Registar passageiro
		}

		break;
	}
	}
}

void passengerThread(pPipeAndData pipeAndData) {
	if (pipeAndData->pipe == NULL) {
		_ftprintf(stderr, L"O Handle recebido na thread do reader est� a NULL\n");
		return;
	}

	HANDLE readReadyEvent = CreateEvent(
		NULL,
		TRUE,
		FALSE,
		NULL);

	if (readReadyEvent == NULL) {
		_ftprintf(stderr, L"A cria��o do evento na thread do reader deu erroL\n");
		return;
	}

	OVERLAPPED overlappedReadReady = { 0 };
	while (1) {
		ZeroMemory(&overlappedReadReady, sizeof(overlappedReadReady));
		ResetEvent(readReadyEvent);
		overlappedReadReady.hEvent = readReadyEvent;

		DWORD numberBytesRead;
		PassengerProtocol messageRead;

		ReadFile(
			pipeAndData->pipe,
			&messageRead,
			sizeof(PassengerProtocol),
			&numberBytesRead,
			&overlappedReadReady);

		WaitForSingleObject(readReadyEvent, INFINITE);
		_tprintf(L"[DEBUG] Mensagem lida.\n");

		GetOverlappedResult(
			pipeAndData->pipe,
			&overlappedReadReady,
			&numberBytesRead,
			FALSE);

		if (numberBytesRead < sizeof(PassengerProtocol)) {
			_ftprintf(stderr, L"Acho que a leitura falhou\n");
			continue;
		}

		processMessageFromPassenger(pipeAndData, messageRead);
	}

}

void passengerRegister(pDATA data) {
	HANDLE hPipe;
	while (1) {
		hPipe = CreateNamedPipe(
			PIPENAME,
			PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
			PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
			PIPE_UNLIMITED_INSTANCES,
			BUFSIZE,
			BUFSIZE,
			100000,
			NULL);
		if(hPipe == INVALID_HANDLE_VALUE) {
			_ftprintf(stderr, L"N�o foi poss�vel criar o pipe de registo.\n");
			return;
		}

		_tprintf(L"[DEBUG] Controlador � espera que passageiros se registem...\n");

		BOOL connected = ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
		pPipeAndData passengerThreadParameters = malloc(sizeof(pipeAndData));
		passengerThreadParameters->data = data;
		passengerThreadParameters->pipe = hPipe;

		if (connected) {	
			_tprintf(L"[DEBUG] Passageiro connectado\n");
			
			HANDLE thread = CreateThread(
				NULL,
				0,
				(LPTHREAD_START_ROUTINE)passengerThread,
				(LPVOID)passengerThreadParameters,
				0,
				NULL);
			if (thread == NULL) {
				_ftprintf(stderr, L"N�o foi poss�vel criar a thread do passageiro.\n");
				return;
			}
			else {
				CloseHandle(thread);
			}
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
		_ftprintf(stderr, L"N�o foi poss�vel criar a thread do produtor consumidor.\n");
	}
}


int _tmainCMD(int argc, TCHAR* argv[]) {
	TCHAR command[SIZE];
	pDATA data;

	data = malloc(sizeof(DATA));
	if (data == NULL) {
		_ftprintf(stderr, L"Memorry alloc fail for data\n");
		return -1;
	}

	data->nrAirplanes = 0;

	//previne poder ter mais do que uma inst�ncia do mesmo programa a correr em simult�neo.
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

	//Coloca as variaveis no registry caso n�o estejam onde � esperado...
	if (!prepareEnvironment(KEY_PATH)) {
		puts("Registry fail!");
		return -1;
	}

	//Obt�m vari�veis que supostamente est�o no Registry
	data->maxAirplanes = getMax(MAX_AIRPLANES,KEY_PATH);
	data->maxAirports  = getMax(MAX_AIRPORTS,KEY_PATH);
	if (!data->maxAirplanes || !data->maxAirports) {
		_tprintf(L"Registry vars fail\n");
		return -1;
	}

	//estas fun��es internamente fornecem mais informa��es caso algo falhe
	if (!createAirportSpace(data) || !createMap(data) || !createAirplaneSpace(data) || !createProducerConsumer(data)) {
		_ftprintf(stderr, L"File mapping fail\n");
		return -1;
	}

	_tprintf(L"Max airports do registry :  %d\nMax airplanes do registry : %d\nComando 'help' para mais informa��es.\n\n",
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
