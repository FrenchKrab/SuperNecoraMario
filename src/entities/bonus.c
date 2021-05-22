#include <math.h>
#include <stdio.h>
#include "bonus.h"
#include "../entities.h"
#include "../kengine.h"
#include "../kmath.h"
#include "player.h"

#define BONUS_ARG_TYPE 0
#define BONUS_ARG_AUTOCOLLECT 1

#define BONUS_SCORE_COIN 10
#define BONUS_SCORE_MUSHROOM 300

#define BONUS_MUSHROOM_VELOCITY_X 64
#define BONUS_STAR_VELOCITY_X 64
#define BONUS_STAR_VELOCITY_Y 64
#define BONUS_GRAVITY 512

#define BONUS_COLLECTANIMATION_LENGTH 0.5
#define BONUS_COLLECTANIMATION_HEIGHT 16

#define BONUS_ANIMATION_IDLE 0


SpriteSheet *sheetBonus = NULL;


BonusData *BonusData_Create()
{
    BonusData *data = malloc(sizeof(BonusData));
    data->collectedAnimationTimer=-1;
    return data;
}

void BONUS_start(World* world, GameEntity* self)
{
    self->runtimeData = BonusData_Create();

    //Init spritesheet if needed
    if(sheetBonus==NULL)
    {
        SDL_Texture *bonusTex = Hashmap_str_get(graphicalResources, RESOURCE_SPRITE_BONUS);
        if(bonusTex!=NULL)
        {
            sheetBonus=SpriteSheet_Create(bonusTex, 16, 16);
        }
    }

    SpriteSheetRenderer *sr = SpriteSheetRenderer_Create(sheetBonus, 1);
    if((int)self->spawnData.args[BONUS_ARG_TYPE]==BONUS_TYPE_COIN)
    {
        sr->animations[BONUS_ANIMATION_IDLE]=SpriteSheetAnimation_Create(4,(int[]){0,1,2,1});
        sr->playSpeed = 6;

        self->worldCollisions=0;
    }
    else if((int)self->spawnData.args[BONUS_ARG_TYPE]==BONUS_TYPE_MUSHROOM)
    {
        sr->animations[BONUS_ANIMATION_IDLE]=SpriteSheetAnimation_Create(1,(int[]){4});
        self->worldCollisions = 1;
        self->bounce=(Vect2){1,0};
        self->velocity=(Vect2){64,0};
    }
    else if((int)self->spawnData.args[BONUS_ARG_TYPE]==BONUS_TYPE_STAR)
    {
        sr->animations[BONUS_ANIMATION_IDLE]=SpriteSheetAnimation_Create(2,(int[]){6,7});
        sr->playSpeed = 10;
        self->worldCollisions = 1;
        self->bounce=(Vect2){1,1};
        self->velocity=(Vect2){64,-64};
    }
    else
    {
        sr->animations[BONUS_ANIMATION_IDLE]=SpriteSheetAnimation_Create(1,(int[]){5});
        sr->playSpeed = 10;
        self->worldCollisions = 1;
        self->bounce=(Vect2){1,1};
        self->velocity=(Vect2){64,-64};    
    }

    List_add(&self->spriteSheetRenderers, sr);

    self->colliderSize = (SDL_Point){16,16};
    sr->position=Vect2_ToPoint(self->position);

    if((int)self->spawnData.args[BONUS_ARG_AUTOCOLLECT]==1)
    {
        GameEntity *player = NULL;
        //We need to find the player in case it's a powerup
        if((int)self->spawnData.args[BONUS_ARG_TYPE]!=BONUS_TYPE_COIN)
        {
            List *L = world->entities;
            while(L!=NULL)
            {
                if(((GameEntity*)L->value)->spawnData.type==ENTITY_PLAYER)
                {
                    player = L->value;
                    break;
                }
                L=L->next;
            }
        }
        BONUS_Collect(world, self, player);
    }
}

void BONUS_update(World* world, GameEntity* self)
{
    int type = (int)self->spawnData.args[BONUS_ARG_TYPE];
    BonusData *data = (BonusData*)self->runtimeData;
    SpriteSheetRenderer *sr = (SpriteSheetRenderer*)self->spriteSheetRenderers->value;
    if(data->collectedAnimationTimer>=0)
    {
        sr->position = Vect2_ToPoint(self->position);
        sr->position.y += -BONUS_COLLECTANIMATION_HEIGHT * sin(M_PI/2 * (data->collectedAnimationTimer/BONUS_COLLECTANIMATION_LENGTH));

        data->collectedAnimationTimer+=world->delta;
        if(data->collectedAnimationTimer>BONUS_COLLECTANIMATION_LENGTH)
        {
            List_addUnique(&world->entities_toDelete, self);
        }
    }
    else if(type==BONUS_TYPE_MUSHROOM || type==BONUS_TYPE_STAR)
    {
        self->velocity.y += BONUS_GRAVITY * world->delta;
        sr->position=Vect2_ToPoint(self->position);
    }
}

void BONUS_onCollision(World* world, GameEntity *self, GameEntity *other, CollisionData colData, SDL_Rect inter)
{
    if(other->spawnData.type==ENTITY_PLAYER)
    {
        BONUS_Collect(world,self,other);
    }
}

void BONUS_Collect(World *world, GameEntity *self, GameEntity *player)
{ 
    BonusData *data = (BonusData*)self->runtimeData;
    if(data->collectedAnimationTimer == -1)
    {
        data->collectedAnimationTimer = 0;

        switch((int)self->spawnData.args[BONUS_ARG_TYPE])
        {
            case BONUS_TYPE_COIN:
                world->coins++;
                world->score += BONUS_SCORE_COIN;
                break;
            case BONUS_TYPE_MUSHROOM:
                PLAYER_ChangePower(world, player, MARIO_POWER_BIG);
                world->score += BONUS_SCORE_MUSHROOM;
                break;
        }
        
        self->colliderSize = (SDL_Point){0,0};
        self->worldCollisions = 0;
        self->velocity = (Vect2){0,0};
    }
}