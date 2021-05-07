#ifndef STRUCTS_H
#define STRUCTS_H

#include <windows.h>

#include "../Plane/plane.h"

#define NAMESIZE 500
#define MAPSIZE 1000
#define X 0
#define Y 1



typedef struct aeroporto airport, * pAirport;                    
struct aeroporto {                                                         
	TCHAR name[NAMESIZE];
	int coordinates[2]; //sendo coordinates[0] o x 
};

typedef struct ocupacao ocupation, * pOcupation;
struct ocupacao {
	pAirport airport;
	pPlane airplane;
};

typedef struct mapa Map, * pMap;
struct mapa {
	int matrix[MAPSIZE][MAPSIZE];
};

typedef struct dados DATA, *pDATA;
struct dados {
	HANDLE objMap;
	pMap map;
	int maxAirports  , nrAirports;
	int maxAirplanes , nrAirplanes;
	pAirport airports;
};

typedef struct passageiro passenger, * pPassenger;
struct passageiro {
	TCHAR name[NAMESIZE];
	pAirport origin;
	pAirport destiny;
	long int waitingTime;
};
#endif
