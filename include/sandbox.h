#ifndef SANDBOX_H
#define SANDBOX_H

#include "raylib.h" 



extern bool debugMode; // Global debug mode flag
extern int SMALL_PLATFORM_COUNT;
extern float CONST_GRAVITY;
extern bool gatesOpen; // Global flag for gate state

typedef struct {
    Rectangle hitbox;    // Area of the bench
    Vector2 spawnPoint;  // Where the player respawns
    bool isActive;       // Is this the current active checkpoint?
} Checkpoint;


void sandBox();



#endif


