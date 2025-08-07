#ifndef PHY_H
#define PHY_H   

#include "raylib.h"
typedef struct {
    Rectangle hitbox;
    Vector2 velocity;
    bool onGround;
} Entity;

void applyGravity(Entity *entity, float gravity);
void updateEntity(Entity *player, Rectangle *floor, int platformCount);


#endif