#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>

#include "plane.h"

int _tmain(int argc, TCHAR* argv[]) {

#ifdef UNICODE
	(void)_setmode(_fileno(stdin), _O_WTEXT);
	(void)_setmode(_fileno(stdout), _O_WTEXT);
	(void)_setmode(_fileno(stderr), _O_WTEXT);
#endif


	_tprintf(TEXT("Avião a voar #sqn\n"));

	//TODO: Falta o aeroporto que não faço ideia como vou saber qual o aeroporto onde estou ?????' -.-''' "#$"#$ fuck my life

	if (argc != 3) {
		_ftprintf(stderr, L"Argumentos para lançar o avião insuficientes\n");
		return -1;
	}
	int maxCapacity, velocity; 
	
	if (sscanf_s(argv[1], "%d", &maxCapacity) == 0) {
		_ftprintf(stderr, L"Capacidade do avião inválida\n");
		return -1;
	}
	if (sscanf_s(argv[2], "%d", &velocity) == 0) {
		_ftprintf(stderr, L"Velocidade do avião inválida\n");
		return -1;
	}

	_tprintf(L"Cap: %d \nVel: %d\n", maxCapacity, velocity);



}
