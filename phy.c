#include "phy.h"
#include <math.h>  // For fabs

void applyGravity(Entity *entity, float gravity){
    if (!entity->onGround) {
        entity->velocity.y += gravity;
    }
}

void updateEntity(Entity *entity, Rectangle *platforms, int platformCount) {
    // Move horizontally
    entity->hitbox.x += entity->velocity.x;
    for (int i = 0; i < platformCount; i++) {
        if (CheckCollisionRecs(entity->hitbox, platforms[i])) {
            if (entity->velocity.x > 0)
                entity->hitbox.x = platforms[i].x - entity->hitbox.width;
            else if (entity->velocity.x < 0)
                entity->hitbox.x = platforms[i].x + platforms[i].width;
            entity->velocity.x = 0;
        }
    }

    // Move vertically
    entity->hitbox.y += entity->velocity.y;
    entity->onGround = false;
    for (int i = 0; i < platformCount; i++) {
        if (CheckCollisionRecs(entity->hitbox, platforms[i])) {
            if (entity->velocity.y > 0) {
                entity->hitbox.y = platforms[i].y - entity->hitbox.height;
                entity->onGround = true;
            } else if (entity->velocity.y < 0) {
                entity->hitbox.y = platforms[i].y + platforms[i].height;
            }
            entity->velocity.y = 0;
        }
    }
}

void updateEnemy(Entity *enemy, Entity *player, Rectangle *platforms, int platformCount, float chaseSpeed, float chaseThreshold) {
    float distanceX = player->hitbox.x - enemy->hitbox.x;

    // Horizontal movement toward player
    if (fabs(distanceX) > chaseThreshold) {
        enemy->velocity.x = (distanceX > 0) ? chaseSpeed : -chaseSpeed;
    } else {
        enemy->velocity.x = 0;
    }

    applyGravity(enemy, 0.5f);
    updateEntity(enemy, platforms, platformCount);

    // Stop overlapping player horizontally
    if (CheckCollisionRecs(player->hitbox, enemy->hitbox)) {
        if (enemy->velocity.x > 0) {
            enemy->hitbox.x = player->hitbox.x - enemy->hitbox.width;
        } else if (enemy->velocity.x < 0) {
            enemy->hitbox.x = player->hitbox.x + player->hitbox.width;
        }
        enemy->velocity.x = 0;
    }
}



