#ifndef ENEMIES_H
#define ENEMIES_H

#include "raylib.h" 
#include "phy.h"

typedef struct{
    Entity base; // Inherit from Entity
    bool isJumping; // Flag to check if the player is jumping
    pos spawnPos;
    bool isChasing;
    int patrolDirection; // 1 for right, -1 for left
    int patrolDistance; // Distance to patrol before changing direction
}chaser;


void Chasers(chaser *enemy, Player *player, Rectangle *platforms, int platformCount);

#endif