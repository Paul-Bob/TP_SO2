#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>

#include "preparacao.h"
#include "comandosControlador.h"

#define MAX_AIRPLANES TEXT("maxAirplanes")
#define MAX_AIRPORTS  TEXT("maxAirports")
#define SIZE 200

int _tmain(int argc, TCHAR* argv[]) {
	TCHAR comando[SIZE];

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
	if (!preparaAmbiente()) {
		puts("Registry fail!");
		return -1;
	}

	//Obtém variáveis que supostamente estão no Registry
	int maxAirplanes = getMax(MAX_AIRPLANES);
	int maxAirports  = getMax(MAX_AIRPORTS);
	if (!maxAirplanes || !maxAirports) {
		_tprintf(L"Registry vars fail\n");
		return -1;
	}

	_tprintf(L"Max airports do registry :  %d\nMax airplanes do registry : %d\nComando 'help' para mais informações.\n\n",maxAirports,maxAirplanes);

	do {
		_tprintf(L"-> ");
		_fgetts(comando, SIZE, stdin);
		comando[_tcslen(comando) - 1] = '\0';
		interpretaComandoControlador(comando);
	} 	while (_tcscmp(comando, L"exit"));
	return 0;
}