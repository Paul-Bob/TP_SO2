#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>

#include "../HF/structs.h"

int getArguments(int argc, LPTSTR argv[], pPassenger passenger);

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

	if (argc < 4 || argc > 5) {
		_ftprintf(stderr, L"Lançamento passageiro: %s aeroportoOrigem aeroportoDestino nome tempoEspera(opcional)\n",argv[0]);
		return 0;
	}

	_tcscpy_s(passenger->name, NAMESIZE, argv[3]);


	if (argc == 5) {
		TCHAR* endptr;
		passenger->waitingTime = _tcstol(argv[4], &endptr, 10);

		if (strlen(endptr)) {
			_ftprintf(stderr, L"%s depois do inteiro não aceite...\n", endptr);
			return 0;
		}

		if (passenger->waitingTime <= 0) {
			_ftprintf(stderr, L"Tempo de espera deve ser um maior do que 0\n");
			return 0;
		}

		//ñ me lembro porque fiz isto, vou deixar se me lembrar entretanto
		/*
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
		*/
	}
	else {
		passenger->waitingTime = 999999999;
	}

	_ftprintf(stderr, L"TODO: verificar existencia dos aeroportos\norigem '%s'\ndestino '%s'\ne associar os ponteiros ao passageiro no controlador.\n",argv[1],argv[2]);
	_ftprintf(stderr, L"Nome: %s\nTempo de espera maximo: %d\n",passenger->name,passenger->waitingTime);


	return 1;
}
