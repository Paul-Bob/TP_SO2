#ifndef PLANE_H
#define PLANE_H
#include "../HF/structs.h"

#define SIZE 200

typedef struct Dados Data, * pData;
struct Dados {
	pPlane plane;
	int ongoingTrip;
	HANDLE tripThread;
	HANDLE heartbeatThread;
	CRITICAL_SECTION criticalSection;
	HANDLE controlSemaphore[2];
	HANDLE producerMutex;
	HANDLE emptiesSemaphore;
	HANDLE itemsSemaphore;
	HANDLE objProducerConsumer;
	HANDLE objAirports;
	HANDLE objPlanes;
	pPlane planes;
	int maxAirports;
	pAirport airports;
	pProducerConsumer producerConsumer;
};

void notifyController(pData data, enum messageType type); 
int openCreateKey(HKEY* key, DWORD* result, TCHAR* path);
int getMax(TCHAR* attribute, TCHAR* path);
int setInitialAirport(TCHAR* name, pData data);
int setDestinationAirport(TCHAR* destination, pData data);
int getArguments(int argc, LPTSTR argv[], pData data);
void setPosition(pMap map, int x, int y, int value);
int getValueOnPosition(pMap map, int x, int y);
void initTrip(pData data);
void heartbeat(pData data);
void initTripThread(pData data);
void initHeartbeatThread(pData data);
int processCommand(TCHAR* command, pData data);
int registerPlaneInController(pData data);
void closePlane(pData data);
void undoRegister(pData data);

#endif