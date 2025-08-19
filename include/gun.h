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
void ShootBullet(Bullet bullets[], Vector2 position, int direction); // 1=right, -1=left
void UpdateBullets(Bullet bullets[], int max);
void DrawBullets(Bullet bullets[], int max);

#endif
