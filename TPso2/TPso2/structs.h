#ifndef STRUCTS_H
#define STRUCTS_H

#include <windows.h>

#define NAMESIZE 500
#define MAPSIZE 1000

typedef struct aeroporto airport, * pAirport;                    
struct aeroporto {                                                         
	TCHAR name[NAMESIZE];
	int coordinates[2]; //sendo coordinates[0] o x 
};

typedef struct dados DATA, *pDATA;
struct dados {
	int map[MAPSIZE][MAPSIZE];      //0-empty      1-airport      2-airplane
	int maxAirports  , nrAirports;
	int maxAirplanes , nrAirplanes;
	pAirport airports;
};

#endif
