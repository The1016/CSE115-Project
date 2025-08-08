// gun.h

#ifndef GUN_H
#define GUN_H

#include "raylib.h"

#define MAX_BULLETS 100

typedef struct {
    Rectangle hitbox;
    Vector2 velocity;
    bool active;
} Bullet;

void InitBullets(Bullet bullets[], int max);
void ShootBullet(Bullet bullets[], Vector2 position, Vector2 direction);
void UpdateBullets(Bullet bullets[], int max);
void DrawBullets(Bullet bullets[], int max);

#endif
