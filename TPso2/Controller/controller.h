#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "../HF/structs.h"

#define SIZE 200
#define BUFSIZE 2048

typedef struct pipeEDados pipeAndData, * pPipeAndData;

struct pipeEDados {
	HANDLE pipe;
	pDATA data;
};

void printConsumedInfo(Protocol message, pPlane plane);
void assignPassengersToPlane(pDATA data);
int getNumberOfAirplanes(pDATA data);
void removePlane(pRemovePlane removeData);
void initPlaneTimerThread(pRemovePlane removePlaneData);
void sendMessageToPassengersOfPlane(pDATA data, int planeID, enum messagePassengerType type);
void producerConsumer(pDATA data);
void initProducerConsumerThread(pDATA data);
int writeOnPipe(HANDLE pipe, PassengerProtocol message);
void registerPassenger(pDATA data, Passenger passenger);
void processMessageFromPassenger(pPipeAndData pipeAndData, PassengerProtocol message);
void removePassenger(pDATA data, int idPassenger);
void passengerThread(pPipeAndData pipeAndData);
void passengerRegister(pDATA data);
void initControlPassengerRegisterThread(pDATA data);

#endif