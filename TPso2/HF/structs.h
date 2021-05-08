#ifndef STRUCTS_H
#define STRUCTS_H

#include <windows.h>

#include "../Plane/plane.h"

#define NAMESIZE 500
#define MAPSIZE 1000
#define X 0
#define Y 1
#define KEY_PATH TEXT("SOFTWARE\\TP_SO2\\")
#define MAX_AIRPLANES TEXT("maxAirplanes")
#define MAX_AIRPORTS  TEXT("maxAirports")



typedef struct aeroporto airport, * pAirport;                    
struct aeroporto {                                                         
	TCHAR name[NAMESIZE];
	int coordinates[2]; //sendo coordinates[0] o x 
};

typedef struct mapa Map, * pMap;
struct mapa {
	int matrix[MAPSIZE][MAPSIZE];
};

typedef struct dados DATA, *pDATA;
struct dados {
	HANDLE objMap;
	HANDLE objAirports;
	HANDLE objPlanes;
	HANDLE airportsMutex;
	HANDLE mapMutex;
	pMap map;
	pAirport airports;
	pPlane planes;
	int maxAirports  , nrAirports;
	int maxAirplanes , nrAirplanes;
};

typedef struct passageiro passenger, * pPassenger;
struct passageiro {
	TCHAR name[NAMESIZE];
	pAirport origin;
	pAirport destiny;
	long int waitingTime;
};
#endif
