#ifndef STRUCTS_H
#define STRUCTS_H

#include <windows.h>

#define NAMESIZE 500

typedef struct aeroporto airport, * pAirport;                    
struct aeroporto {                                                         
	TCHAR name[NAMESIZE];
	int coordinates[2]; //sendo coordinates[0] o x 
};

typedef struct dados DATA, *pDATA;
struct dados {
	int maxAirports  , nrAirports;
	int maxAirplanes , nrAirplanes;
	pAirport airports;
};

#endif
