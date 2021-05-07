#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>

#include "sharedMemory.h"

int createMap(pDATA data) {
	data->objMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(Map), _T("map"));

	if (data->objMap == NULL) {
		_ftprintf(stderr, L"Impossível criar o file mapping.\n");
		return 0;
	}

	data->map = (pMap)MapViewOfFile(data->objMap, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Map));

	if (data->map == NULL) {
		_ftprintf(stderr, L"Impossível criar a vista do file mapping.\n");
		return 0;
	}

	for (int i = 0; i < MAPSIZE - 1; i++) {
		for (int j = 0; j < MAPSIZE - 1; j++) {
			data->map->matrix[i][j] = 0;
		}
	}

	//for (int i = 0; i < MAPSIZE - 1; i++) {
	//	for (int j = 0; j < MAPSIZE - 1; j++) {
	//		_tprintf(L"[DEBUG] X: %d, Y: %d Ocupado: %d \n", i, j, map->matrix[i][j]);
	//	}
	//}

	HANDLE mapMutex = CreateMutex(0, FALSE, _T("mapMutex"));

	return 1;
}