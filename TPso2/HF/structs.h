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
#define PIPENAME TEXT("\\\\.\\pipe\\pipeRegister")

typedef struct Coordenada Coordinate, * pCoordinate;

struct Coordenada {
	int x, y;
};

typedef struct Aviao Plane, * pPlane;

struct Aviao {
	int maxCapacity, velocity, planeID, index;
	TCHAR actualAirport[NAMESIZE];
	TCHAR destinAirport[NAMESIZE];
	TCHAR departureAirport[NAMESIZE];
	Coordinate initial, current, final;
	HANDLE heartbeatTimer;
};

enum messageType { Register, Departure, Arrive, Heartbeat };

typedef struct protocolo Protocol, * pProtocol;
struct protocolo {
	enum messageType type;
	int planeID;
	int index;
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
	HANDLE controlPlanes;
	HANDLE controlPassengerRegisterThread;
	pProducerConsumer producerConsumer;
	pMap map;
	pAirport airports;
	pPlane planes;
	int maxAirports  , nrAirports;
	int maxAirplanes , nrAirplanes;
};

typedef struct passageiro Passenger, * pPassenger;
struct passageiro {
	int id;
	TCHAR name[NAMESIZE];
	TCHAR origin[NAMESIZE];
	TCHAR destiny[NAMESIZE];
};

enum messagePassengerType { RegisterPassenger };

typedef struct protocoloPassageiro PassengerProtocol, * pPassengerProtocol;
struct protocoloPassageiro {
	enum messagePassengerType type;
	Passenger passenger;
	BOOL success;
};

typedef struct removeAviao RemovePlane, * pRemovePlane;

struct removeAviao {
	pMap map;
	pPlane plane;
	HANDLE controll;
};

#endif
