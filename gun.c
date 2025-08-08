// gun.c

#include "gun.h"

void InitBullets(Bullet bullets[], int max) {
    for (int i = 0; i < max; i++) {
        bullets[i].active = false;
    }
}

void ShootBullet(Bullet bullets[], Vector2 position, Vector2 direction) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) {
            bullets[i].hitbox = (Rectangle){ position.x, position.y, 10, 4 };
            bullets[i].velocity = direction;
            bullets[i].active = true;
            break;
        }
    }
}

void UpdateBullets(Bullet bullets[], int max) {
    for (int i = 0; i < max; i++) {
        if (bullets[i].active) {
            bullets[i].hitbox.x += bullets[i].velocity.x;
            bullets[i].hitbox.y += bullets[i].velocity.y;

            // Deactivate if off screen
            if (bullets[i].hitbox.x > GetScreenWidth() || bullets[i].hitbox.x < 0)
                bullets[i].active = false;
        }
    }
}

void DrawBullets(Bullet bullets[], int max) {
    for (int i = 0; i < max; i++) {
        if (bullets[i].active) {
            DrawRectangleRec(bullets[i].hitbox, RED);
        }
    }
}
