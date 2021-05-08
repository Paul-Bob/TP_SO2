#ifndef PLANE_H
#define PLANE_H

#include <windows.h>

typedef struct Aviao Plane, * pPlane;
typedef struct Coordenada Coordinate, * pCoordinate;

struct Coordenada {
	int x, y;
};

struct Aviao {
	int maxCapacity, velocity, flagThreadTrip;
	Coordinate initial, current, final;
	HANDLE tripThread;
	CRITICAL_SECTION criticalSection;
};

#endif