#ifndef BLOCK_H
#define BLOCK_H

#include "../entities.h"
#include "../kengine.h"


typedef struct
{
    SpriteSheetRenderer *activeRenderer;
    double lastTimeHit;

    int contentLeft;
    GameEntity_SpawnData contentSpawndata;
} BlockData;

BlockData *BlockData_Create();


void BLOCK_start(World*, GameEntity*);
void BLOCK_update(World*, GameEntity*);
void BLOCK_onCollision(World*,GameEntity*,GameEntity*,CollisionData,SDL_Rect);

void BLOCK_bump(World *world, GameEntity *self);


#endif