#ifndef STRUCTS_H
#define STRUCTS_H

#include <windows.h>

#define NAMESIZE 500
#define MAPSIZE 1000
#define X 0
#define Y 1
#define KEY_PATH TEXT("SOFTWARE\\TP_SO2\\")
#define MAX_AIRPLANES TEXT("maxAirplanes")
#define MAX_AIRPORTS  TEXT("maxAirports")
#define DIM_BUFFER 50 

typedef struct Coordenada Coordinate, * pCoordinate;

struct Coordenada {
	int x, y;
};

typedef struct Aviao Plane, * pPlane;

struct Aviao {
	int maxCapacity, velocity;
	Coordinate initial, current, final;
};

enum messageType { Departure, Arrive, Heartbeat };

typedef struct protocolo Protocol, * pProtocol;
struct protocolo {
	enum messageType type;
	int planeID;
};

typedef struct produtorConsumidor ProducerConsumer, * pProducerConsumer;
struct produtorConsumidor {
	int in, out;
	Protocol buffer[DIM_BUFFER];
};

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
	HANDLE objProducerConsumer;
	HANDLE airportsMutex;
	HANDLE mapMutex;
	HANDLE producerConsumerThread;
	HANDLE itemsSemaphore;
	HANDLE emptiesSemaphore;
	pProducerConsumer producerConsumer;
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
