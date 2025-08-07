#ifndef PHY_H
#define PHY_H   

#include "raylib.h"

typedef struct {
    Rectangle hitbox;
    Vector2 velocity;
    bool onGround;
} Entity;

typedef struct {
    Rectangle hitbox;
    Vector2 velocity;
    bool active;
    float speed;
    int health;
    int direction;
    float aiTimer;

    bool attacking;  // 👈 ADD THIS
} Enemy;

// Player and general object logic
void applyGravity(Entity *entity, float gravity);
void updateEntity(Entity *player, Rectangle *floor, int platformCount);

// Enemy-specific logic
void updateEnemy(Entity *enemy, Entity *player, Rectangle *platforms, int platformCount, float chaseSpeed, float chaseThreshold);


#endif
