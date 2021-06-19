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

	data->mapMutex = CreateMutex(0, FALSE, _T("mapMutex"));

	return 1;
}

int createAirportSpace(pDATA data) {
	data->objAirports = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, data->maxAirports * sizeof(airport), _T("airports"));

	if (data->objAirports == NULL) {
		_ftprintf(stderr, L"Impossível criar o file mapping.\n");
		return 0;
	}

	data->airports = (pAirport)MapViewOfFile(data->objAirports, FILE_MAP_ALL_ACCESS, 0, 0, data->maxAirports * sizeof(airport));

	if (data->airports == NULL) {
		_ftprintf(stderr, L"Impossível criar a vista do file mapping.\n");
		return 0;
	}

	for (int i = 0; i < data->maxAirports; i++) {
		_tcscpy_s(data->airports[i].name, NAMESIZE, _T("Empty"));
	}

	data->nrAirports = 0;
	data->airportsMutex = CreateMutex(0, FALSE, _T("airportsMutex"));

	return 1;
}

int createAirplaneSpace(pDATA data) {

	data->objPlanes = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, data->maxAirplanes * sizeof(Plane), _T("planes"));

	if (data->objPlanes == NULL) {
		_ftprintf(stderr, L"Impossível criar o file mapping.\n");
		return 0;
	}

	data->planes = (pPlane)MapViewOfFile(data->objPlanes, FILE_MAP_ALL_ACCESS, 0, 0, data->maxAirplanes * sizeof(Plane));

	if (data->planes == NULL) {
		_ftprintf(stderr, L"Impossível criar a vista do file mapping.\n");
		return 0;
	}

	for (int i = 0; i < data->maxAirplanes; i++) {
		data->planes[i].velocity = -1; //flag
		data->planes[i].heartbeatTimer = NULL;
	}
	data->planesMutex = CreateMutex(0, FALSE, _T("planesMutex"));
	data->controlPlanesMutex = CreateMutex(0, FALSE, _T("planeEntryControlMutex"));
	data->controlPlanes = CreateSemaphore(NULL, data->maxAirplanes, data->maxAirplanes, _T("planeEntryControl"));

	return 1;
}

int createProducerConsumer(pDATA data) {
	data->objProducerConsumer = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(ProducerConsumer), _T("producerConsumer"));

	if (data->objProducerConsumer == NULL) {
		_ftprintf(stderr, L"Impossível criar o file mapping do produtor/consumidor.\n");
		return 0;
	}

	data->producerConsumer = (pProducerConsumer)MapViewOfFile(data->objProducerConsumer, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(ProducerConsumer));

	if (data->producerConsumer == NULL) {
		_ftprintf(stderr, L"Impossível criar a vista do produtor/consumidor.\n");
		return 0;
	}

	data->producerConsumer->in = 0;
	data->producerConsumer->out = 0;

	data->itemsSemaphore = CreateSemaphore(NULL, 0, DIM_BUFFER, _T("itemsSemaphore"));
	data->emptiesSemaphore = CreateSemaphore(NULL, DIM_BUFFER, DIM_BUFFER, _T("emptiesSemaphore"));

	return 1;
}