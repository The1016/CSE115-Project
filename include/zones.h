#ifndef ZONES_H
#define ZONES_H

#include "raylib.h"

#define MAX_SMALL_PLATFORMS 10
#define MAX_ZONES 3

typedef struct {
    char name[50];           // Name of the zone
    Rectangle floor;
    Rectangle smallPlatforms[MAX_SMALL_PLATFORMS];
    int platformCount;
    Color bgColor;           // Background color of the zone
    float leftBound;         // Left boundary of the zone
    float rightBound;        // Right boundary of the zone
    Rectangle entrance;      // Entrance position to next zone
} MapZone;


extern MapZone zones[MAX_ZONES];
extern int currentZone;

void initZones(int screenWidth, int screenHeight); // Call this at start

#endif
