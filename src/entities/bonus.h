#ifndef BONUS_H
#define BONUS_H

#include "../entities.h"
#include "../kengine.h"

typedef enum {
    BONUS_TYPE_COIN=0,
    BONUS_TYPE_MUSHROOM=1,
    BONUS_TYPE_LEAF=2,
    BONUS_TYPE_STAR=3
} BONUS_TYPE;

typedef struct {
    double collectedAnimationTimer;
} BonusData;

BonusData *BonusData_Create();

void BONUS_start(World*, GameEntity*);
void BONUS_update(World*, GameEntity*);
void BONUS_onCollision(World*,GameEntity*,GameEntity*,CollisionData,SDL_Rect);

void BONUS_Collect(World *world, GameEntity *self, GameEntity *player);

#endif