#ifndef PASSENGER_H
#define PASSENGER_H

#include "../HF/structs.h"

#define SIZE 200

typedef struct Dados Data, * pData;
struct Dados {
	pPassenger passenger;
	HANDLE waitingTimeThread;
	HANDLE waitingTimeTimer;
	long int waitingTime;
	HANDLE processCommandThread;
	HANDLE namedPipeReaderThread;
	HANDLE hPipe;
	HANDLE readReadyEvent;
};

void closeHandlers(pData data);
int processMessageFromController(PassengerProtocol message);
void readerThread(pData data);
int connectToController(pData data);
void processCommand();
void initCommandProcessThread(pData data);
int getArguments(int argc, LPTSTR argv[], pData data);
int writeOnPipe(HANDLE pipe, PassengerProtocol message);
int registerOnController(pData data);

#endif