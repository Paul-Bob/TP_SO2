#ifndef PASSENGER_H
#define PASSENGER_H

#include <windows.h>
#include "../HF/structs.h"

typedef struct Dados Data, * pData;
struct Dados {
	pPassenger passenger;
	HANDLE waitingTimeThread;
	HANDLE waitingTimeTimer;
	long int waitingTime;
	HANDLE processCommandThread;
	HANDLE namedPipeReaderThread;
	HANDLE hPipe;
};

#endif