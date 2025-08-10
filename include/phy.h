#ifndef PHY_H
#define PHY_H   

#include "raylib.h"



typedef struct  {
    Rectangle hitbox;      // Entity's hitbox for collision detection
    Vector2 velocity;   
    bool onGround;       // Flag to check if the entity is on the ground
} Entity;
typedef struct {
    Entity base; // Inherit from Entity
    bool isJumping; // Flag to check if the player is jumping
    float jumpTime; // Time the jump has been held
    float coyoteTimer;
    float jumpBufferTimer;
    bool isDashing;
    float dashTime;
    float dashCooldown;
    int dashDirection;  
    float iFrames;
    float knkbackTime;
    float maxHealth;
    float health;
    float deathTimer;
    bool isAlive;
} Player;
typedef struct {
    Entity base;
    bool active;
    float speed;
    int health;
    int direction;
    float aiTimer;

    bool attacking;  // 👈 ADD THIS
} Enemy;
void handleJump(Player *entity);
void handleDash(Player *entity);


// Player and general object logic
void applyGravity(Entity *entity, float gravity,float gravityscale);
void updateEntity(Entity *player, Rectangle *floor, int platformCount,float ignoreHorizontalCollision);
void updatePlayer(Player *player, Enemy *enemy, Rectangle *platforms);

// Enemy-specific logic
void updateEnemy(Enemy *enemy, Player *player, Rectangle *platforms, int platformCount, float chaseSpeed, float chaseThreshold);
void resolveEntityCollision(Player *player, Enemy *enemy);

#endif
