#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>

#include "../HF/structs.h"
#include "prepareEnv.h"
#include "textUI.h"

#define MAX_AIRPLANES TEXT("maxAirplanes")
#define MAX_AIRPORTS  TEXT("maxAirports")
#define SIZE 200
#define KEY_PATH TEXT("SOFTWARE\\TP_SO2\\")

int _tmain(int argc, TCHAR* argv[]) {
	TCHAR command[SIZE];
	pDATA data;

	data = malloc(sizeof(DATA));
	if (data == NULL) {
		_ftprintf(stderr, L"Memorry alloc fail for data\n");
		return -1;
	}

	data->nrAirplanes = 0;
	data->nrAirports  = 0;


	//previne poder ter mais do que uma inst�ncia do mesmo programa a correr em simult�neo.
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

	//Coloca as variaveis no registry caso n�o estejam onde � esperado...
	if (!prepareEnvironment(KEY_PATH)) {
		puts("Registry fail!");
		return -1;
	}

	//Obt�m vari�veis que supostamente est�o no Registry
	data->maxAirplanes = getMax(MAX_AIRPLANES,KEY_PATH);
	data->maxAirports  = getMax(MAX_AIRPORTS,KEY_PATH);
	if (!data->maxAirplanes || !data->maxAirports) {
		_tprintf(L"Registry vars fail\n");
		return -1;
	}

	//vetores avi�es/aeroportos
	data->airports = malloc(data->maxAirports * sizeof(airport));
	if (data->airports == NULL) {
		_ftprintf(stderr, L"Memorry alloc fail for airports\n");
		return -1;
	}

	//map
	for (int i = 0; i < MAPSIZE-1; i++)
		for (int j = 0; j < MAPSIZE-1; j++)
		{
			data->map[i][j].airplane = NULL;
			data->map[i][j].airport = NULL;
		}

	// Init Altera��es Liliana
	// Decidi n�o apagar o que tinhas feito para n�o estragar nada. Basicamente estou a criar um mapa de inteiros
	HANDLE objMap;
	pMap map;
	objMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(Map), _T("map"));

	if (objMap == NULL) {
		_ftprintf(stderr, L"Imposs�vel criar o file mapping.\n");
		return;
	}

	map = (pMap) MapViewOfFile(objMap, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Map));

	if (map == NULL) {
		_ftprintf(stderr, L"Imposs�vel criar a vista do file mapping.\n");
		return;
	}

	for (int i = 0; i < MAPSIZE - 1; i++) {
		for (int j = 0; j < MAPSIZE - 1; j++) {
			map->matrix[i][j] = 0;
		}
	}

	for (int i = 0; i < MAPSIZE - 1; i++) {
		for (int j = 0; j < MAPSIZE - 1; j++) {
			_tprintf(L"[DEBUG] X: %d, Y: %d Ocupado: %d \n", i, j, map->matrix[i][j]);
		}
	}

	HANDLE mapMutex = CreateMutex(0, FALSE, _T("mapMutex"));

	// End Altera��es Liliana


	_tprintf(L"Max airports do registry :  %d\nMax airplanes do registry : %d\nComando 'help' para mais informa��es.\n\n",data->maxAirports,data->maxAirplanes);

	do {
		_tprintf(L"-> ");
		_fgetts(command, SIZE, stdin);
		command[_tcslen(command) - 1] = '\0';
		interpretaComandoControlador(command,data);
	} 	while (_tcscmp(command, L"exit"));

	free(data->airports);
	free(data);
	// Init Altera��es Liliana

	UnmapViewOfFile(map);
	CloseHandle(objMap);

	// END Altera��es Liliana

	return 0;
}