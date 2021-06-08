#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>

#include "../HF/structs.h"
#include "passenger.h"

#define SIZE 200

void processMessageFromController(PassengerProtocol message) {
	_tprintf(L"[DEBUG] Mensagem recebida do servidor: %d.\n", message.type);
}

void readerThread(HANDLE hPipe) {

	if (hPipe == NULL) {
		_ftprintf(stderr, L"O Handle recebido na thread do reader está a NULL\n");
		return;
	}

	HANDLE readReadyEvent = CreateEvent(
		NULL,
		TRUE,
		FALSE,
		NULL);

	if (readReadyEvent == NULL) {
		_ftprintf(stderr, L"A criação do evento na thread do reader deu erroL\n");
		return;
	}

	OVERLAPPED overlappedReadReady = { 0 };
	while(1) {
		ZeroMemory(&overlappedReadReady, sizeof(overlappedReadReady));
		ResetEvent(readReadyEvent);
		overlappedReadReady.hEvent = readReadyEvent;

		DWORD numberBytesRead;
		PassengerProtocol messageRead;

		ReadFile(
			hPipe,
			&messageRead,
			sizeof(PassengerProtocol),
			&numberBytesRead,
			&overlappedReadReady);

		WaitForSingleObject(readReadyEvent, INFINITE);
		_tprintf(L"[DEBUG] Mensagem lida.\n");

		GetOverlappedResult(
			hPipe,
			&overlappedReadReady,
			&numberBytesRead,
			FALSE);

		if (numberBytesRead < sizeof(PassengerProtocol)) {
			_ftprintf(stderr, L"Acho que a leitura falhou\n");
			continue;
		}
		
		processMessageFromController(messageRead);
	}
}

int connectToController(pData data) {
	while (1) {
		data->hPipe = CreateFile(
		PIPENAME,
			GENERIC_READ | GENERIC_WRITE,
			0 | FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			0 | FILE_FLAG_OVERLAPPED,
			NULL);

		if (data->hPipe != INVALID_HANDLE_VALUE) {
			break;
		}

		if (GetLastError() != ERROR_PIPE_BUSY) {
			_ftprintf(stderr, L"Erro ao criar o pipe (não estava ocupado)\n");
			return -1;
		}

		// Caso as instâncias todas estejam ocupadas (são 255)

		if(!WaitNamedPipe(PIPENAME, 60000)) {
			_ftprintf(stderr, L"Esperei 60 segundos, vou à minha vida\n");
			return -1;
		}
	}

	DWORD dwMode = PIPE_READMODE_MESSAGE;
	BOOL sucess = SetNamedPipeHandleState(
		data->hPipe,
		&dwMode,
		NULL,
		NULL
	);
	
	if (!sucess) {
		_ftprintf(stderr, L"O Set do Named Pipe Falhou\n");
		return -1;
	}

	data->namedPipeReaderThread = CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)readerThread,
		(LPVOID)data->hPipe,
		0,
		NULL);

	if (data->namedPipeReaderThread == NULL) {
		_ftprintf(stderr, L"Criação da Reader Thread falhou\n");
		return -1;
	}

	_tprintf(L"[DEBUG] Connected.\n");
	return 1;
}

int processCommand() {
	TCHAR command[SIZE];
	
	do {
		_tprintf(L"-> ");
		_fgetts(command, SIZE, stdin);
		command[_tcslen(command) - 1] = '\0';
		if (_tcscmp(command, TEXT("exit"))) {
			_ftprintf(stderr, L"Comando não suportado\n");
		}
		else {
			break;
		}
	} while (1);
}

void initCommandProcessThread(pData data) {
	_tprintf(_T("Criei a Thread"));
	data->processCommandThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)processCommand,NULL, 0, NULL);
	if (data->processCommandThread == NULL) {
		_ftprintf(stderr, L"Não foi possível criar a thread do heartbeat.\n");
	}
}

int getArguments(int argc, LPTSTR argv[], pData data) {

	if (argc < 4 || argc > 5) {
		_ftprintf(stderr, L"Lançamento passageiro: %s aeroportoOrigem aeroportoDestino nome tempoEspera(opcional)\n", argv[0]);
		return 0;
	}

	_tcscpy_s(data->passenger->name, NAMESIZE, argv[3]);
	_tcscpy_s(data->passenger->origin, NAMESIZE, argv[1]);
	_tcscpy_s(data->passenger->destiny, NAMESIZE, argv[2]);


	if (argc == 5) {
		TCHAR* endptr;
		data->waitingTime = _tcstol(argv[4], &endptr, 10);

		if (_tcslen(endptr)) {
			_ftprintf(stderr, L"%s depois do inteiro não aceite...\n", endptr);
			return 0;
		}

		if (data->waitingTime <= 0) {
			_ftprintf(stderr, L"Tempo de espera deve ser um maior do que 0\n");
			return 0;
		}
	}
	else {
		data->waitingTime = -1;
	}
	return 1;
}

int writeOnPipe(HANDLE pipe, pPassengerProtocol message) {
	HANDLE writeReadyEvent = CreateEvent(
		NULL,
		TRUE,
		FALSE,
		NULL);
	
	if (writeReadyEvent == NULL) {
		_ftprintf(stderr, L"Não foi possível criar o evento\n");
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

int registerOnController(pData data) {
	PassengerProtocol registerMessage;
	registerMessage.type = RegisterPassenger;
	registerMessage.passenger = *(data->passenger);

	return writeOnPipe(data->hPipe, &registerMessage);
}



int _tmain(int argc, LPTSTR argv[]) {

#ifdef UNICODE
	(void)_setmode(_fileno(stdin), _O_WTEXT);
	(void)_setmode(_fileno(stdout), _O_WTEXT);
	(void)_setmode(_fileno(stderr), _O_WTEXT);
#endif
	
	Data data;
	Passenger passenger;
	data.passenger = &passenger;

	if (!getArguments(argc, argv, &data)) {
		return 1;
	}

	if (connectToController(&data) == -1) {
		_ftprintf(stderr, L"Erro ao conectar ao controlador \n");
		return 1;
	}

	if (registerOnController(&data) == -1) {
		_ftprintf(stderr, L"Erro ao registar no controlador \n");
		return 1;
	}



	data.waitingTimeTimer = CreateWaitableTimer(NULL, TRUE, NULL);

	if(data.waitingTime != -1) {

		if (data.waitingTimeTimer != 0) {
			_tprintf(L"Vou definir o timer %d", data.waitingTime);
			LARGE_INTEGER time;
			time.QuadPart = -10000000LL * data.waitingTime;
			SetWaitableTimer(data.waitingTimeTimer, &time, 0, NULL, NULL, FALSE);
		}
	}
	
	initCommandProcessThread(&data);
	
	HANDLE handleArray[2];
	handleArray[0] = data.waitingTimeTimer;
	handleArray[1] = data.processCommandThread;

	WaitForMultipleObjects(2, handleArray, FALSE, INFINITE);
	_tprintf(_T("Cansei!"));
	
	return 0;
}

