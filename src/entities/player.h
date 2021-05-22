#ifndef PLAYER_H
#define PLAYER_H

#include "../entities.h"
#include "../kengine.h"
#include "../input.h"

typedef enum
{
    MARIO_POWER_LITTLE,
    MARIO_POWER_BIG,
    MARIO_POWER_FLOWER,
    MARIO_POWER_TANUKI
} MARIO_POWER;

typedef struct
{
    int jumping;
    double jumpingTime;
    SpriteSheetRenderer *activeSpriteSheetRenderer;
    SDL_Point rendererOffset;
    MARIO_POWER power;

    double dieAnimationTime;
    double invincibilityTimeLeft;

    SpriteSheetRenderer *ssr_marioLittle;
    SpriteSheetRenderer *ssr_marioBig;
} MarioData;

MarioData *MarioData_Create();


void PLAYER_start(World*, GameEntity*);
void PLAYER_update(World*, GameEntity*);
void PLAYER_onCollision(World*,GameEntity*,GameEntity*,CollisionData,SDL_Rect);

void PLAYER_Die(World *world, GameEntity *self);
void PLAYER_GetDamaged(World *world, GameEntity *self);
void PLAYER_ChangePower(World *world, GameEntity *self, MARIO_POWER power);
void PLAYER_PowerTransition(World *world, GameEntity *self);


#endif