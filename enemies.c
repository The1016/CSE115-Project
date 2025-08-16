#include "phy.h"
#include "sandbox.h"
#include "enemies.h"
#include <math.h>  



#define chaser_walk_speed 3.0f
#define chaser_run_speed 6.5f   
#define vision_range 400.0f


void Chasers(chaser *enemy, Player *player, Rectangle *platforms, int platformCount){
    float dt = GetFrameTime();
    // Draw enemy 
    DrawRectangleRec(enemy->base.hitbox, ORANGE);
    // --- Patrol or chase ---
    if (enemy->isChasing) {
        enemy->base.velocity.x = (player->base.hitbox.x > enemy->base.hitbox.x) ? chaser_run_speed : -chaser_run_speed;
    } else {
        enemy->base.velocity.x = chaser_walk_speed * enemy->patrolDirection;
        enemy->base.hitbox.x += enemy->base.velocity.x * dt;

        // Reverse direction if exceeding patrol distance
        if (enemy->base.hitbox.x > enemy->spawnPos.x + enemy->patrolDistance)
            enemy->patrolDirection = -1;
        if (enemy->base.hitbox.x < enemy->spawnPos.x - enemy->patrolDistance)
            enemy->patrolDirection = 1;
    }

    // --- Gravity ---
    applyGravity(&enemy->base, CONST_GRAVITY, 1.0f);
    updateEntity(&enemy->base, platforms, platformCount, 0.0f);


    // --- Check for player in vision ---
    Rectangle vision = {
        (enemy->patrolDirection == 1) ? enemy->base.hitbox.x + enemy->base.hitbox.width : enemy->base.hitbox.x - vision_range,
        enemy->base.hitbox.y,
        vision_range,
        enemy->base.hitbox.height
    };
    if (CheckCollisionRecs(player->base.hitbox, vision)) {
        enemy->isChasing = true;
    } else if (fabs(player->base.hitbox.x - enemy->base.hitbox.x) > vision_range) {
        enemy->isChasing = false;
    }
}



