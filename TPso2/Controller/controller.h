#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "../HF/structs.h"
typedef struct pipeEDados pipeAndData, * pPipeAndData;

struct pipeEDados {
	HANDLE pipe;
	pDATA data;
};

void printConsumedInfo(Protocol message, pPlane plane, HWND gui);
void removePlane(pRemovePlane removeData);
void initPlaneTimerThread(pRemovePlane removePlaneData);
void producerConsumer(pDATA data);
void initProducerConsumerThread(pDATA data);
int writeOnPipe(HANDLE pipe, PassengerProtocol message);
void processMessageFromPassenger(pPipeAndData pipeAndData, PassengerProtocol message);
void passengerThread(pPipeAndData pipeAndData);
void passengerRegister(pDATA data);
void initControlPassengerRegisterThread(pDATA data);

#endif

