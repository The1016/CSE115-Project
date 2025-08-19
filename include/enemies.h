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

typedef struct {
    Entity base; // Inherit from Entity
    pos spawnPos; // Spawn position of the boss
    float attackCooldown; // Time until next attack
    float attackWindup; // Time spent charging the attack
    bool isCharging;   
    float deathTimer; // Timer for death animation
    float health; // Boss health
    bool isAwake;
    bool showName; 
    float nameTimer;
    float damageCooldown;
} boss1;

void Chasers(chaser *enemy, Player *player, Rectangle *platforms, int platformCount);
void Boss1(boss1 *enemy, Player *player, Rectangle *platforms, int platformCount);
#endif