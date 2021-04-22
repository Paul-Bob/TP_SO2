#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>

#include "structs.h"

int getArguments(int argc, LPTSTR argv[], pPlane plane);

int _tmain(int argc, LPTSTR argv[]) {

#ifdef UNICODE
	(void)_setmode(_fileno(stdin), _O_WTEXT);
	(void)_setmode(_fileno(stdout), _O_WTEXT);
	(void)_setmode(_fileno(stderr), _O_WTEXT);
#endif
	
	passenger passenger;

	if (!getArguments(argc, argv, &passenger)) {
		return 1;
	}
	

	return 0;
}

int getArguments(int argc, LPTSTR argv[], pPassenger passenger) {

	if (argc < 3 || argc > 4) {
		_ftprintf(stderr, L"Lançamento passageiro: %s aeroportoOrigem aeroportoDestino nome tempoEspera(opcional)\n",argv[0]);
		return 0;
	}

	if (argc == 4) {
		errno = 0;
		TCHAR* endptr;
		long int x = strtol(argv[4], &endptr, 10);
		if (!_tcscpy_s(endptr,NAMESIZE,argv[4])) {
			_ftprintf(stderr, L"Lançamento passageiro: %s aeroportoOrigem aeroportoDestino nome tempoEsperaEmSegundos(int opcional)\n", argv[0]);
			return 0;
		}
		else if (*endptr) {
			_ftprintf(stderr, L"%s depois do inteiro não aceites...\n", endptr);
			return 0;
		}
		else if (errno == ERANGE) {
			_ftprintf(stderr, L"Number out of range: %s\n", argv[4]);
			return 0;
		}
	}

	_ftprintf(stderr, L"TODO: verificar existencia do aeroporto origem '%s', destino '%s' e associar os ponteiros ao passageiro no controlador.\n",argv[1],argv[2]);

	_tcscpy_s(passenger->name,NAMESIZE,argv[3]);
	

	return 1;
}
