#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>

#include "plane.h"

#define SIZE 200

int getArguments(int argc, LPTSTR argv[], pPlane plane) {

	//TODO: Falta o aeroporto que não faço ideia como vou saber qual o aeroporto onde estou ?????' -.-''' "#$"#$ fuck my life

	if (argc != 3) {
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

	return 1;
}

int move(int curX, int curY, int finalX, int finalY, int* nextX, int* nextY) {
	return 1;
}

int verifyNewPosition(int next_x, int next_y) {
	return 1; //TODO: Meter isto a avaliar se a posicao está livre
}

void notifyController() {
	_tprintf(L"[DEBUG] Amigo controlador, cheguei, tá? \n"); //TODO: Avisar o controlador que cheguei ou atualizar a minha posicao
}

void initTrip(pPlane plane) {
	
	while (1) {
		Sleep(1000 / (DWORD)plane->velocity);

		int nextX = 0, nextY = 0;
		int result = move(plane->current.x, plane->current.y, plane->final.x, plane->final.y, &nextX, &nextY);

		if (result == 2) {
			continue;
		}
		else if (result == 0) {
			_tprintf(L"[DEBUG] Cheguei ao meu destino, tenho que avisar o controlador \n"); 
			notifyController();
			return;
		}
		else {
			if (verifyNewPosition(nextX, nextY) == 1) {
				plane->current.x = nextX;
				plane->current.y = nextY;
				
				notifyController();
				
			}
			else {
				continue; //TODO: Podemos melhorar esta estrategia, para já estamos só a aguardar que os gordos desocupem a loja
			}
		}
	}

	return;
}

int processCommand(TCHAR* command, pPlane plane) {
	if (!_tcscmp(command, TEXT("exit"))) {
		return 0;
	}

	if (!_tcscmp(command, TEXT("destino"))) {
		_tprintf(L"[DEBUG] Vou definir o destino %s\n", command);
	} else if (!_tcscmp(command, TEXT("iniciar"))) {
		initTrip(plane);
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

	_tprintf(TEXT("Avião a voar #sqn\n"));

	Plane plane;

	if (getArguments(argc, argv, &plane) == -1) {
		return 1;
	}

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
