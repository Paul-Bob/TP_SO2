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

void printConsumedInfo(Protocol message) {
	switch (message.type) {
	case Arrive:
		_tprintf(L"O avião %d chegou ao seu destino", message.planeID); 
		break;
	case Departure:
		_tprintf(L"O avião %d descolou", message.planeID);
		break;
	}
}

void producerConsumer(pDATA data) {
	_tprintf(L"[DEBUG] Estou na thread \n");
	while (1) {
		_tprintf(L"[DEBUG] Estou a espera de coisas \n");
		WaitForSingleObject(data->itemsSemaphore, INFINITE);
		Protocol message = data->producerConsumer->buffer[data->producerConsumer->out];
		data->producerConsumer->out = (data->producerConsumer->out + 1) % DIM_BUFFER;
		ReleaseSemaphore(data->emptiesSemaphore, 1, NULL);
		printConsumedInfo(message);
	}
}

void initProducerConsumerThread(pDATA data) {
	_tprintf(L"[DEBUG] Vou criar a thread \n");
	data->producerConsumerThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)producerConsumer, (LPVOID) data, 0, NULL);
	if (data->producerConsumerThread == NULL) {
		_ftprintf(stderr, L"Não foi possível criar a thread do produtor consumidor.\n");
	}
}

int _tmain(int argc, TCHAR* argv[]) {
	TCHAR command[SIZE];
	pDATA data;
	HANDLE controlPlanes;

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

	if (!createAirportSpace(data)) {
		_ftprintf(stderr, L"File mapping fail\n");
		return -1;
	}

	if (!createMap(data)) {
		_ftprintf(stderr, L"File mapping fail\n");
		return -1;
	}

	if (!createAirplaneSpace(data)) {
		_ftprintf(stderr, L"File mapping fail\n");
		return -1;
	}

	if (!createProducerConsumer(data)) {
		_ftprintf(stderr, L"File mapping producer/consumer fail\n");
		return -1;
	}

	controlPlanes = CreateSemaphore(NULL, data->maxAirplanes, data->maxAirplanes, _T("planeEntryControl")); //TODO: Acho que isto não devia estar aqui

	_tprintf(L"Max airports do registry :  %d\nMax airplanes do registry : %d\nComando 'help' para mais informações.\n\n",data->maxAirports,data->maxAirplanes);
	
	initProducerConsumerThread(data);

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
