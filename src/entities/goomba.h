#ifndef GOOMBA_H
#define GOOMBA_H

#include "../entities.h"
#include "../kengine.h"

typedef struct
{
    SpriteSheetRenderer *sr_goomba;
    SpriteSheetRenderer *sr_wing_left, *sr_wing_right;
    

    double timeSinceJumpedOn;
} GoombaData;

GoombaData *GoombaData_Create();

void GOOMBA_start(World*, GameEntity*);
void GOOMBA_update(World*, GameEntity*);
void GOOMBA_onCollision(World*, GameEntity*, GameEntity*, CollisionData, SDL_Rect);

#endif