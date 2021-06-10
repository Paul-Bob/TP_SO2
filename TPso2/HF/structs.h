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
#define MAX_PASSENGERS 50

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

typedef struct passageiro Passenger, * pPassenger;
struct passageiro {
	int id;
	TCHAR name[NAMESIZE];
	TCHAR origin[NAMESIZE];
	TCHAR destiny[NAMESIZE];
	int planeID; // se o valor for -1 quer dizer que está na origin
	HANDLE pipe;
};

enum messageType { Register, Departure, Arrive, Heartbeat, PlaneUpdatedDestiny, PlaneUpdatePosition };

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
	CRITICAL_SECTION passengersCriticalSection;
	HANDLE planesMutex;
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
	pPassenger passengers;
	int maxAirports  , nrAirports;
	int maxAirplanes;
	int nrPassengers;
};

enum messagePassengerType { RegisterPassenger, DeparturePassenger, ArrivePassenger, FoundPlane, UpdatePositionPassenger };

typedef struct protocoloPassageiro PassengerProtocol, * pPassengerProtocol;
struct protocoloPassageiro {
	enum messagePassengerType type;
	Passenger passenger;
	BOOL success;
	Coordinate coordinates;
};

typedef struct removeAviao RemovePlane, * pRemovePlane;

struct removeAviao {
	pMap map;
	pPlane plane;
	HANDLE controll;
};

#endif
