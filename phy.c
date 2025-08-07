
#include "phy.h"


void applyGravity(Entity *entity, float gravity){
    if(!entity->onGround) { 
        entity->velocity.y += gravity;

    } 
}
void updateEntity(Entity *player, Rectangle *floor, int platformCount){
    // Apply movement from velocity
    player->hitbox.y += player->velocity.y;
    player->onGround = false;

    for (int i = 0; i < platformCount; i++) {
        if (CheckCollisionRecs(player->hitbox, floor[i])) {
            // Snap to top of platform
            player->hitbox.y = floor[i].y - player->hitbox.height;
            player->velocity.y = 0;
            player->onGround = true;
        }
    }
}





