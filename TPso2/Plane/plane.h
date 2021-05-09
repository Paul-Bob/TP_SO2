#ifndef PLANE_H
#define PLANE_H

#include <windows.h>
#include "../HF/structs.h"

typedef struct Dados Data, * pData;
struct Dados {
	Plane plane;
	int ongoingTrip;
	HANDLE tripThread;
	HANDLE heartbeatThread;
	CRITICAL_SECTION criticalSection;
	HANDLE controlSemaphore;
	HANDLE producerMutex;
	HANDLE emptiesSemaphore;
	HANDLE itemsSemaphore;
	HANDLE objProducerConsumer;
	pProducerConsumer producerConsumer;
};

#endif