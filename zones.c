#include "zones.h"
#include <string.h> // for strcpy

MapZone zones[MAX_ZONES];
int currentZone;

void initZones(int screenWidth, int screenHeight) {
    // ----- Zone 0 -----
    zones[0].floor = (Rectangle){ 0, screenHeight - 100, screenWidth, 100 };
    zones[0].platformCount = 4;
    zones[0].bgColor = GREEN;
    zones[0].leftBound = 0;
    zones[0].rightBound = screenWidth;
    zones[0].entrance = (Rectangle){ screenWidth - 50, screenHeight - 150, 50, 100 };

    zones[0].smallPlatforms[0] = (Rectangle){ 200, screenHeight - 200, 100, 20 };
    zones[0].smallPlatforms[1] = (Rectangle){ 400, screenHeight - 250, 100, 20 };
    zones[0].smallPlatforms[2] = (Rectangle){ 600, screenHeight - 300, 100, 20 };
    zones[0].smallPlatforms[3] = (Rectangle){ 800, screenHeight - 220, 100, 20 };

    strcpy(zones[0].name, "Green Meadows");

    // ----- Zone 1 -----
    zones[1].floor = (Rectangle){ 0, screenHeight - 120, screenWidth, 120 };
    zones[1].platformCount = 3;
    zones[1].bgColor = DARKGREEN;
    zones[1].leftBound = 0;
    zones[1].rightBound = screenWidth;
    zones[1].entrance = (Rectangle){ screenWidth - 50, screenHeight - 180, 50, 100 };

    zones[1].smallPlatforms[0] = (Rectangle){ 150, screenHeight - 220, 100, 20 };
    zones[1].smallPlatforms[1] = (Rectangle){ 350, screenHeight - 270, 100, 20 };
    zones[1].smallPlatforms[2] = (Rectangle){ 550, screenHeight - 300, 100, 20 };

    strcpy(zones[1].name, "Dark Forest");

    // ----- Zone 2 -----
    zones[2].floor = (Rectangle){ 0, screenHeight - 150, screenWidth, 150 };
    zones[2].platformCount = 5;
    zones[2].bgColor = RED;
    zones[2].leftBound = 0;
    zones[2].rightBound = screenWidth;
    zones[2].entrance = (Rectangle){ screenWidth - 50, screenHeight - 200, 50, 100 };

    zones[2].smallPlatforms[0] = (Rectangle){ 100, screenHeight - 250, 100, 20 };
    zones[2].smallPlatforms[1] = (Rectangle){ 300, screenHeight - 300, 100, 20 };
    zones[2].smallPlatforms[2] = (Rectangle){ 500, screenHeight - 350, 100, 20 };
    zones[2].smallPlatforms[3] = (Rectangle){ 700, screenHeight - 280, 100, 20 };
    zones[2].smallPlatforms[4] = (Rectangle){ 900, screenHeight - 320, 100, 20 };

    strcpy(zones[2].name, "Crimson Ruins");
}
