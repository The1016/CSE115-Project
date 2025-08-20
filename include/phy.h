#ifndef PHY_H
#define PHY_H   

#include "raylib.h"



#define MAX_ENEMY_PROJECTILES 10
typedef enum { SLASH_FORWARD, SLASH_UP, SLASH_DOWN } SlashDirection;

typedef struct {
    int x;
    int y;
}pos;
typedef struct  {
    Rectangle hitbox;      // Entity's hitbox for collision detection
    Vector2 velocity;   
    bool onGround;  
    bool isAlive; // Flag to check if the entity is alive
    int health;
    float damageCooldown;
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
    float knkbackTime;
    float maxHealth;
    float deathTimer;
    bool isAlive;
    bool isSlashing;
    float slashTimer;
    Rectangle slashHitbox;
    float slashDuration;
    SlashDirection slashDirection; // Direction of the slash
    int facingDirection;
    bool hasDoubleJump; 
    float slashCooldown; 
    int slashAnchorFacing;
    bool slashFlippedMid;
    float recoilTime;
    float recoilDuration;
    float recoilStrength;
    float recoilVelocityX; 
} Player;
typedef struct {
    Rectangle hitbox;
    Vector2 velocity;
    bool active;
    bool isParried; // New flag to track if projectile has been parried
} EnemyProjectile;
typedef struct {
    Entity base;
    bool active;
    float speed;
    int direction;
    float aiTimer;
    bool attacking;  // 👈 ADD THIS
    float attackWindup;    // Time spent preparing attack
    float attackCooldown;  // Time between attacks
    bool isCharging;       // Visual indicator state
    float deathTimer; 
    EnemyProjectile projectiles[MAX_ENEMY_PROJECTILES];
} Enemy;





void handleJump(Player *entity);
void handleDash(Player *entity);
void handleSlash(Player *player, Entity *target);

// Player and general object logic
void applyGravity(Entity *entity, float gravity,float gravityscale);
void updateEntity(Entity *player, Rectangle *floor, int platformCount,float ignoreHorizontalCollision);
void updatePlayer(Player *player, Enemy *enemy, Rectangle *platforms);
void handlePlayerCollisionDamage(Player *player, Entity *enemyEntity, int damage);

// Enemy-specific logic
void updateEnemy(Enemy *enemy, Player *player, Rectangle *platforms, int platformCount, float chaseSpeed, float chaseThreshold);

// Add after other function declarations
void initializePlayer(Player *player, float screenWidth, float screenHeight);
Camera2D* getGameCamera(void);  // Function to access the camera


#endif
