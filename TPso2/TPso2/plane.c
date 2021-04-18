#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>

#include "plane.h"

int getArguments(int argc, TCHAR* argv[], pPlane plane) {

	//TODO: Falta o aeroporto que n�o fa�o ideia como vou saber qual o aeroporto onde estou ?????' -.-''' "#$"#$ fuck my life

	if (argc != 3) {
		_ftprintf(stderr, L"Argumentos para lan�ar o avi�o insuficientes\n");
		return -1;
	}
	

	if (sscanf_s(argv[1], "%d", &plane->maxCapacity) == 0) {
		_ftprintf(stderr, L"Capacidade do avi�o inv�lida\n");
		return -1;
	}
	if (sscanf_s(argv[2], "%d", &plane->velocity) == 0) {
		_ftprintf(stderr, L"Velocidade do avi�o inv�lida\n");
		return -1;
	}

	return 1;
}

int _tmain(int argc, TCHAR* argv[]) {

#ifdef UNICODE
	(void)_setmode(_fileno(stdin), _O_WTEXT);
	(void)_setmode(_fileno(stdout), _O_WTEXT);
	(void)_setmode(_fileno(stderr), _O_WTEXT);
#endif

	_tprintf(TEXT("Avi�o a voar #sqn\n"));

	Plane plane;

	if (getArguments(argc, argv, &plane) == -1) {
		return 1;
	}

	_tprintf(L"[DEBUG] Cap: %d - Vel: %d\n", plane.maxCapacity, plane.velocity);

}
