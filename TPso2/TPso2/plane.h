#ifndef PLANE_H
#define PLANE_H

#include <windows.h>


typedef struct Aviao Plane, * pPlane;
struct Aviao {
	int maxCapacity, velocity;
};

#endif