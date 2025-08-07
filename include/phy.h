#ifndef PHY_H
#define PHY_H   

#include "raylib.h"
typedef struct {
    Rectangle hitbox;
    Vector2 velocity;
    bool onGround;
    bool isJumping; // Flag to check if the player is jumping
    float jumpTime; // Time the jump has been held
    float coyoteTimer;
    float jumpBufferTimer;
    bool isDashing;
    float dashTime;
    float dashCooldown;
    int dashDirection;  
} Entity;

void applyGravity(Entity *entity, float gravity);
void updateEntity(Entity *player, Rectangle *floor, int platformCount);
void handleJump(Entity *entity);
void handleDash(Entity *entity);


#endif