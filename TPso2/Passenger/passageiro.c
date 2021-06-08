#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>

#include "../HF/structs.h"
#include "passenger.h"

#define SIZE 200


int registerPassenger() {
	// Tentar connectar ao Pipe de registo
	// enviar os dados do passageiro
	// esperar resposta
	// se resposta == -1 abortar
	// se não ficar à escuta de coisas
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

	if (registerPassenger() == 0) {
		_ftprintf(stderr, L"Erro ao registar o passageiro no controlador\n");
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

