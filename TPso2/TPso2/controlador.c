#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>

#include "preparacao.h"

int _tmain(int argc, TCHAR* argv[]) {
	//previne poder ter mais do que uma inst�ncia do mesmo programa a correr em simult�neo.
	CreateMutexA(0, FALSE, "Local\\$myprogram$"); // try to create a named mutex
	if (GetLastError() == ERROR_ALREADY_EXISTS) {   // did the mutex already exist?
		puts("Already running!");
		return -1;								  // quit; mutex is released automatically
	}

	//Coloca as variaveis no registry caso n�o estejam onde � esperado...
	if (!preparaAmbiente()) {
		puts("Registry fail!");
		return -1;
	}

	//Obt�m vari�veis que supostamente est�o no Registry
	int maxAirplanes = getMax(TEXT("maxAirplanes"));
	int maxAirports  = getMax(TEXT("maxAirports" ));
	if (!maxAirplanes || !maxAirports) {
		_tprintf(L"Registry vars fail\n");
		return -1;
	}


#ifdef UNICODE
	(void)_setmode(_fileno(stdin), _O_WTEXT);
	(void)_setmode(_fileno(stdout), _O_WTEXT);
	(void)_setmode(_fileno(stderr), _O_WTEXT);
#endif

	_tprintf(L"Ol� mundo!\nMax airports do registry :  %d\nMax airplanes do registry : %d\n",maxAirports,maxAirplanes);
	Sleep(5000); //testar linha 11 a 15
	return 0;
}