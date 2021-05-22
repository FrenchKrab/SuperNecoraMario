#include <math.h>
#include <stdio.h>
#include "block.h"
#include "../entities.h"
#include "../kengine.h"
#include "../kmath.h"

SpriteSheet *sheetBlock = NULL;

#define BLOCK_ARG_BLOCKSKIN 0
#define BLOCK_ARG_CONTENT_COUNT 1
#define BLOCK_ARG_CONTENT_ENTITYTYPE 4
#define BLOCK_ARG_CONTENT_ARGS 5

#define BLOCK_HIT_ANIMATION_LENGTH 0.3
#define BLOCK_HIT_ANIMATION_HEIGHT 8

#define BLOCK_ANIMATION_IDLE 0
#define BLOCK_ANIMATION_UNUSABLE 1

BlockData *BlockData_Create()
{
    BlockData *data = malloc(sizeof(BlockData));
    data->activeRenderer = NULL;
    data->contentLeft = 0;
    data->lastTimeHit = BLOCK_HIT_ANIMATION_LENGTH;
    return data;
}

void BLOCK_start(World* world, GameEntity* self)
{
    BlockData *data = BlockData_Create();
    self->runtimeData = data;
    //Init spritesheet if needed
    if(sheetBlock==NULL)
    {
        SDL_Texture *blockTex = Hashmap_str_get(graphicalResources, RESOURCE_SPRITE_BLOCK);
        if(blockTex!=NULL)
        {
            sheetBlock=SpriteSheet_Create(blockTex, 16, 16);
        }
    }
    //Goomba
    SpriteSheetRenderer *sr = SpriteSheetRenderer_Create(sheetBlock, 2);
    if(self->spawnData.args[BLOCK_ARG_BLOCKSKIN]==0)
    {
        sr->animations[BLOCK_ANIMATION_IDLE]=SpriteSheetAnimation_Create(4,(int[]){0,1,2,3});
        sr->animations[BLOCK_ANIMATION_UNUSABLE]=SpriteSheetAnimation_Create(1,(int[]){8});
    }
    else
    {
        sr->animations[BLOCK_ANIMATION_IDLE]=SpriteSheetAnimation_Create(4,(int[]){4,5,6,7});
        sr->animations[BLOCK_ANIMATION_UNUSABLE]=SpriteSheetAnimation_Create(1,(int[]){8});
    }

    List_add(&self->spriteSheetRenderers, sr);
    data->activeRenderer=sr;

    self->colliderSize = (SDL_Point){16,16};
    self->entitySolid = 10;
    data->activeRenderer->position=Vect2_ToPoint(self->position);
    data->activeRenderer->playSpeed = 4;

    SDL_Point contentSpawnPos = Vect2_ToPoint(self->position);
    contentSpawnPos.y -= self->colliderSize.y;
    data->contentSpawndata.position=contentSpawnPos;
    data->contentSpawndata.type= self->spawnData.args[BLOCK_ARG_CONTENT_ENTITYTYPE];
    for(int i=0; i<SPAWNDATA_ARGS - BLOCK_ARG_CONTENT_ARGS; i++)
    {
        data->contentSpawndata.args[i] = self->spawnData.args[i+BLOCK_ARG_CONTENT_ARGS];
    }
    data->contentLeft = self->spawnData.args[BLOCK_ARG_CONTENT_COUNT];
    self->worldCollisions = 0;
}

void BLOCK_update(World* world, GameEntity* self)
{
    BlockData *data = (BlockData*)self->runtimeData;
    SDL_Point pos = Vect2_ToPoint(self->position);
    if(data->lastTimeHit<BLOCK_HIT_ANIMATION_LENGTH)
    {
        data->activeRenderer->position.y = pos.y - BLOCK_HIT_ANIMATION_HEIGHT * sin(M_PI*(data->lastTimeHit/BLOCK_HIT_ANIMATION_LENGTH));
        data->lastTimeHit += world->delta;
    }
    else
    {
        data->activeRenderer->position.y = pos.y;
    }
}

void BLOCK_onCollision(World* world, GameEntity* self,GameEntity* other, CollisionData colData, SDL_Rect inter)
{
    if(other->spawnData.type==ENTITY_PLAYER && colData.down)
    {
        BLOCK_bump(world, self);
    }
}

void BLOCK_bump(World *world, GameEntity *self)
{
    BlockData *data = (BlockData*)self->runtimeData;

    if(data->lastTimeHit>=BLOCK_HIT_ANIMATION_LENGTH)
    {
        if(data->contentLeft>0)
        {
            GameEntity *spawned = GameEntity_Create(data->contentSpawndata);
            List_add(&world->entities_toAdd, spawned);
            data->contentLeft--;
            data->lastTimeHit=0;
        }

        //If block doesnt contains anything anymore
        if(data->contentLeft<=0)
        {
            //If block contained something, set it as an unsuable block
            if(self->spawnData.args[BLOCK_ARG_CONTENT_COUNT]!=0)
            {
                SpriteSheetRenderer_Play(data->activeRenderer, BLOCK_ANIMATION_UNUSABLE);
            }
            //Else, destroy it
            else
            {
                List_addUnique(&world->entities_toDelete, self);
            }
            data->contentLeft=-1;
        }
    }
}