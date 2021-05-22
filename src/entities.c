#include <stdio.h>
#include <SDL2/SDL.h>
#include <math.h>
#include "entities.h"
#include "kds.h"
/*Include every entity type*/
#include "goomba.h"
#include "player.h"
#include "block.h"
#include "bonus.h"


GameEntity_Behaviour entityFunctions[ENTITY_TYPE_COUNT];


void SetupEntityFunctions()
{
    entityFunctions[ENTITY_PLAYER].start = PLAYER_start;
    entityFunctions[ENTITY_PLAYER].update = PLAYER_update;
    entityFunctions[ENTITY_PLAYER].on_collision_entity = PLAYER_onCollision;
    
    entityFunctions[ENTITY_GOOMBA].start = GOOMBA_start;
    entityFunctions[ENTITY_GOOMBA].update = GOOMBA_update;
    entityFunctions[ENTITY_GOOMBA].on_collision_entity = GOOMBA_onCollision;

    entityFunctions[ENTITY_BONUS].start = BONUS_start;
    entityFunctions[ENTITY_BONUS].update = BONUS_update;
    entityFunctions[ENTITY_BONUS].on_collision_entity = BONUS_onCollision;

    entityFunctions[ENTITY_BLOCK].start = BLOCK_start;
    entityFunctions[ENTITY_BLOCK].update = BLOCK_update;
    entityFunctions[ENTITY_BLOCK].on_collision_entity = BLOCK_onCollision;


}

GameEntity *GameEntity_Create(GameEntity_SpawnData spawn)
{
    GameEntity *entity = malloc(sizeof(GameEntity));
    entity->spawnData = spawn;
    entity->position = Vect2_FromPoint(entity->spawnData.position);
    entity->position_last = entity->position;
    entity->velocity = (Vect2){0,0};
    entity->behaviour=&entityFunctions[entity->spawnData.type];
    entity->spriteSheetRenderers=NULL;
    entity->colliderSize=(SDL_Point){1,1};
    entity->bounce=(Vect2){0,0};
    entity->worldCollisions=0;
    entity->entitySolid=0;
    //TODO load behaviour
    return entity;
}

void GameEntity_Delete(void *entity)
{
    GameEntity *e = ((GameEntity*)entity);
    free(e->runtimeData);
    List_free(e->spriteSheetRenderers, free);
    free(entity);
}

SDL_Rect GameEntity_GetCollisionRect(GameEntity *entity)
{
    SDL_Point pos = Vect2_ToPoint(entity->position);
    SDL_Rect rect = {pos.x,pos.y,entity->colliderSize.x,entity->colliderSize.y};
    return rect;
}

SpriteSheetRenderer *SpriteSheetRenderer_Create(SpriteSheet *sheet, int animationCount)
{
    SpriteSheetRenderer *sr = malloc(sizeof(SpriteSheetRenderer)
                                +sizeof(SpriteSheetAnimation*)*animationCount);
    sr->position=(SDL_Point){0,0};
    sr->tint=(SDL_Color){255,255,255,255};
    sr->angle=0;
    sr->center=(SDL_Point){0,0};
    sr->activeAnimation=0;
    sr->playSpeed=1;
    sr->animationCount=animationCount;
    sr->flip=SDL_FLIP_NONE;
    sr->sheet=sheet;
    sr->lastFrame=0;
    sr->lastFrameTick=0;
    sr->z=10;
    sr->visible=1;
    return sr;
}

void SpriteSheetRenderer_ForcePlay(SpriteSheetRenderer *sheetRenderer, int animationId)
{
    sheetRenderer->activeAnimation = animationId;
    sheetRenderer->lastFrame = 0;
    sheetRenderer->lastFrameTick = SDL_GetTicks();
}

void SpriteSheetRenderer_Play(SpriteSheetRenderer *sheetRenderer, int animationId)
{
    if(sheetRenderer->activeAnimation!=animationId)
    {
        SpriteSheetRenderer_ForcePlay(sheetRenderer, animationId);
    }
}

SDL_Rect SpriteSheetRenderer_GetRenderRect(SpriteSheetRenderer *sheetRenderer, Uint32 currentTick)
{
    Uint32 offset = currentTick - sheetRenderer->lastFrameTick;
    SpriteSheetAnimation *anim = sheetRenderer->animations[sheetRenderer->activeAnimation];
    if(sheetRenderer->playSpeed!=0)
    {
        if(offset>(1000/sheetRenderer->playSpeed))
        {
            sheetRenderer->lastFrame++;
            if(sheetRenderer->lastFrame >= sheetRenderer->animations[sheetRenderer->activeAnimation]->frameCount)
            {
                sheetRenderer->lastFrame = 0;
            }
            sheetRenderer->lastFrameTick=currentTick;
        }
    }
    return SpriteSheet_GetRectFromIndex(sheetRenderer->sheet, anim->frames[sheetRenderer->lastFrame]);
}

SDL_Rect SpriteSheetRenderer_GetDestRect(SpriteSheetRenderer *sheetRenderer)
{
    SDL_Rect dest;
    dest.x = sheetRenderer->position.x;
    dest.y = sheetRenderer->position.y;
    dest.w = sheetRenderer->sheet->cellSize_x;
    dest.h = sheetRenderer->sheet->cellSize_y;
    return dest;
}

void StartAllEntities(World *world)
{
    List *l = world->entities;
    while(l != NULL)
    {
        GameEntity *entity = (GameEntity*)l->value;
        if(entity->behaviour!=NULL && entity->behaviour->start != NULL)
        {
            entity->behaviour->start(world, entity);
        }
        l=l->next;
    }
}

void UpdateAllEntities(World *world)
{
    List *l = world->entities;

    //Update entities
    while(l != NULL)
    {
        GameEntity *entity = (GameEntity*)l->value;
        //Call entities' update function
        if(entity->behaviour!=NULL && entity->behaviour->update != NULL)
        {
            entity->behaviour->update(world, entity);
        }
        //Update their position according to physics
        entity->position_last = entity->position;
        entity->position.x += entity->velocity.x * world->delta;
        entity->position.y += entity->velocity.y * world->delta;

        l=l->next;
    }

    //Get entity out of world collisions
    if(world->tileset!=NULL)
    {
        SDL_Point cellSize = (SDL_Point){world->tileset->cellSize_x, world->tileset->cellSize_y};
        //printf("\n---Start collision\n");
        l = world->entities;
        while(l != NULL)
        {
            GameEntity *entity = (GameEntity*)l->value;
            entity->collisionData=(CollisionData){0,0,0,0};
            if(!entity->worldCollisions)
            {
                l=l->next;
                continue;
            }
            SDL_Point entityPos = Vect2_ToPoint(entity->position);
            SDL_Point entityPos_old = Vect2_ToPoint(entity->position_last);
            SDL_Rect collisionRect_last = {entityPos_old.x,entityPos_old.y,entity->colliderSize.x,entity->colliderSize.y};
            SDL_Point posInCell = {entityPos.x/cellSize.x, entityPos.y/cellSize.y};
            SDL_Point checkRadius;
            checkRadius.x = 2+((entity->colliderSize.x/cellSize.x))/2;
            checkRadius.y = 2+((entity->colliderSize.y/cellSize.y))/2;
            for(int x = KMath_MaxInt(0,posInCell.x-checkRadius.x); x<=(KMath_MinInt(world->level->sizeX-1, posInCell.x+checkRadius.x));x++)
            {
                for(int y = KMath_MaxInt(0,posInCell.y-checkRadius.y); y<=(KMath_MinInt(world->level->sizeY-1, posInCell.y+checkRadius.y));y++)
                {
                    if(Level_Get_Cell_Type(world->level, world->tileset, x, y)==TILECOL_WALL)
                    {
                        SDL_Rect collisionRect = GameEntity_GetCollisionRect(entity);

                        SDL_Rect cellCollider = {x*cellSize.x, y*cellSize.y, cellSize.x, cellSize.y};
                        SDL_Point outOffset = Collision_GetOneRectOutOfCollision(collisionRect_last, collisionRect, cellCollider);
                        Collision_ApplyOffsetOnEntity(entity, outOffset);
                        Collision_UpdateCollisionDataFromOffset(&entity->collisionData, outOffset);

                        collisionRect = GameEntity_GetCollisionRect(entity);
                        Collision_GetBoundaryCheckers(collisionRect, cellCollider, 1, &entity->collisionData);
                    }
                }
            }

            
            //SDL_Rect collisionRect = {entityPos.x,entityPos.y,entity->colliderSize.x,entity->colliderSize.y};

            //if(SDL_IntersectRectAndLine())
            l=l->next;
        }
    }


    //Do entity collisions
    l=world->entities;
    while(l!=NULL)
    {
        GameEntity *entity1 = (GameEntity*)l->value;
        List *l_other = l->next;
        while(l_other!=NULL)
        {
            GameEntity *entity2 = (GameEntity*)l_other->value;
            CollisionData colData = {0,0,0,0};
            SDL_Rect intersection={0,0,0,0};
            int isSolidInteraction = 0;

            if((entity1->entitySolid!=0 || entity2->entitySolid!=0) && entity1->entitySolid!=entity2->entitySolid)
            {
                isSolidInteraction = 1;

                GameEntity *solidest = NULL;
                GameEntity *lightest = NULL;

                if(entity1->entitySolid>entity2->entitySolid)
                {
                    solidest = entity1;
                    lightest = entity2; 
                }
                else
                {
                    solidest = entity2;
                    lightest = entity1;
                }

                SDL_Rect solidestRect = GameEntity_GetCollisionRect(solidest);
                SDL_Rect lightestRect = GameEntity_GetCollisionRect(lightest);
                SDL_Point lightestPos_old = Vect2_ToPoint(lightest->position_last);
                SDL_Rect lightestRect_old = {lightestPos_old.x,lightestPos_old.y,lightest->colliderSize.x,lightest->colliderSize.y};

                SDL_IntersectRect(&solidestRect, &lightestRect, &intersection);
                SDL_Point outOffset = Collision_GetOneRectOutOfCollision(lightestRect_old, lightestRect, solidestRect);
                
                if(outOffset.x!=0 || outOffset.y!=0)
                {
                    Collision_ApplyOffsetOnEntity(lightest, outOffset);
                    Collision_UpdateCollisionDataFromOffset(&colData, outOffset);
                    if(solidest==entity1)
                      colData = CollisionData_Reverse(colData);
                }

            }

            SDL_Rect r1 = GameEntity_GetCollisionRect(entity1);
            SDL_Rect r2 = GameEntity_GetCollisionRect(entity2);
            Collision_GetBoundaryCheckers(r1, r2, 2, &colData);

            if(isSolidInteraction)
            {
                entity1->collisionData=CollisionData_AddCollisions(colData, entity1->collisionData);
                entity2->collisionData=CollisionData_AddCollisions(CollisionData_Reverse(colData), entity2->collisionData);
            }
            else
            {
                SDL_IntersectRect(&r1,&r2,&intersection);
            }
            
            if((intersection.w>0 && intersection.h>0) || CollisionData_HasCollision(colData))
            {
                if(entity1->spawnData.type==ENTITY_BLOCK || entity2->spawnData.type==ENTITY_BLOCK )
                {
                    //printf("\n%d: (%d,%d,%d,%d)  | %d: (%d,%d,%d,%d)",entity1->spawnData.type, r1.x, r1.y, r1.w, r1.h, entity2->spawnData.type, r2.x, r2.y, r2.w, r2.h);
                }

                if(entity1->behaviour != NULL && entity1->behaviour->on_collision_entity!=NULL)
                    entity1->behaviour->on_collision_entity(world, entity1, entity2, colData, intersection);
                if(entity2->behaviour != NULL && entity2->behaviour->on_collision_entity!=NULL)
                    entity2->behaviour->on_collision_entity(world, entity2, entity1, CollisionData_Reverse(colData), intersection);

            }


            l_other=l_other->next;
        }
        l=l->next;
    }
}

void DrawAllEntities(World *world)
{
    Uint32 tick = SDL_GetTicks();
    List *l = world->entities;

    while(l != NULL)
    {
        GameEntity *entity = (GameEntity*)l->value;
        List *l_sr = entity->spriteSheetRenderers;
        while(l_sr != NULL)
        {
            SpriteSheetRenderer *sr = (SpriteSheetRenderer*)(l_sr->value);
            if(sr->visible)
            {
                Camera_AddSpriteSheetRendererToCameraQueue(sr, world->camera, tick);   
            }
            l_sr=l_sr->next;
        }
        l=l->next;
    }
}


SDL_Point Collision_GetOneRectOutOfCollision(SDL_Rect pos1_old, SDL_Rect pos1, SDL_Rect pos2)
{
    SDL_Rect inter;

    if(SDL_IntersectRect(&pos1, &pos2, &inter)==SDL_TRUE)
    {
        //printf("\nThey DO intersect: (%d,%d,%d,%d) and (%d,%d,%d,%d)  OLD=(%d,%d,%d,%d)",pos1.x,pos1.y,pos1.w,pos1.h,pos2.x,pos2.y,pos2.w,pos2.h,pos1_old.x,pos1_old.y,pos1_old.w,pos1_old.h);
        SDL_Point getOut = {inter.w, inter.h};
        //printf("\n--- %d, %d", getOut.x, getOut.y);

        //Get centers
        SDL_Point c1_old = KMath_Rect_GetCenter(pos1_old);
        //SDL_Point c1 = KMath_Rect_GetCenter(pos1);
        SDL_Point c2 = KMath_Rect_GetCenter(pos2);

        //SDL_Point c1_offset = {c1_old.x-c1.x, c1_old.y-c1.y};

        int value = 0;
        int centerOffsets = 0;
        int dir = 0;
        //if(abs(getOut.x)<abs(getOut.y))
        if(abs(getOut.x)<abs(getOut.y))
        {
            value=getOut.x;
            centerOffsets=c2.x-c1_old.x;
            dir = 0;
        }
        else
        {
            value=getOut.y;
            centerOffsets=c2.y-c1_old.y;
            dir = 1;
        }
/*
        if(KMath_SignInt(centerOffsets)!=KMath_SignInt(old_centerOffsets))
        {
            value = obstacleSize-value;
            value *= -1;
            printf("Reversed!");
        }
        */
        
        value*= -KMath_SignInt(centerOffsets);
        if(dir==0)
            return (SDL_Point){value, 0};
        else
            return (SDL_Point){0, value};
    }
    return (SDL_Point){0,0};
}

void Collision_GetBoundaryCheckers(SDL_Rect checkedRect, SDL_Rect otherRect,
                                            int outerRadius, CollisionData *colData)
{
    SDL_Rect entityOuterRect_d = {checkedRect.x, checkedRect.y+checkedRect.h,
                                    checkedRect.w, outerRadius};
    SDL_Rect entityOuterRect_r = {checkedRect.x+checkedRect.w, checkedRect.y,
                                    outerRadius, checkedRect.w};
    SDL_Rect entityOuterRect_u = {checkedRect.x, checkedRect.y-outerRadius,
                                    checkedRect.w, outerRadius};
    SDL_Rect entityOuterRect_l = {checkedRect.x-outerRadius, checkedRect.y,
                                    outerRadius, checkedRect.h};
    if(SDL_HasIntersection(&otherRect,&entityOuterRect_d)==SDL_TRUE)
        colData->down = 1;
    if(SDL_HasIntersection(&otherRect,&entityOuterRect_r)==SDL_TRUE)
        colData->right = 1;
    if(SDL_HasIntersection(&otherRect,&entityOuterRect_u)==SDL_TRUE)
        colData->up = 1;
    if(SDL_HasIntersection(&otherRect,&entityOuterRect_l)==SDL_TRUE)
        colData->left = 1;
}

void Collision_ApplyOffsetOnEntity(GameEntity *entity, SDL_Point outOffset)
{
    entity->position.x+=outOffset.x;
    entity->position.y+=outOffset.y;

    //Make entity bounce
    if(outOffset.x!=0)
    {
        entity->velocity.x = -entity->bounce.x*entity->velocity.x;
    }
    if(outOffset.y!=0)
    {
        entity->velocity.y = -entity->bounce.y*entity->velocity.y;
    }
}

void Collision_UpdateCollisionDataFromOffset(CollisionData *data, SDL_Point outOffset)
{
    if(outOffset.x<0)
        data->right=1;
    else if(outOffset.x>0)
        data->left=1;
    else if(outOffset.y>0)
        data->up=1;
    else if(outOffset.y<0)
        data->down=1;
}

CollisionData CollisionData_Reverse(CollisionData data)
{
    CollisionData reversed;
    reversed.right=data.left;
    reversed.left=data.right;
    reversed.up=data.down;
    reversed.down=data.up;
    return reversed;
}

CollisionData CollisionData_AddCollisions(CollisionData data1, CollisionData data2)
{
    CollisionData data = {0,0,0,0};
    if(data1.right || data2.right)
        data.right=1;
    if(data1.left || data2.left)
        data.left=1;
    if(data1.up || data2.up)
        data.up=1;
    if(data1.down || data2.down)
        data.down=1;
    return data;
}

int CollisionData_HasCollision(CollisionData data)
{
    return data.right || data.left || data.up || data.down;
}

/*
SDL_Point Collision_GetOneRectOutOfCollision(SDL_Rect pos1_old, SDL_Rect pos1, SDL_Rect pos2)
{
    SDL_Rect inter;

    if(SDL_IntersectRect(&pos1, &pos2, &inter)==SDL_TRUE)
    {
        printf("\nThey DO intersect: (%d,%d,%d,%d) and (%d,%d,%d,%d)  OLD=(%d,%d,%d,%d)",pos1.x,pos1.y,pos1.w,pos1.h,pos2.x,pos2.y,pos2.w,pos2.h,pos1_old.x,pos1_old.y,pos1_old.w,pos1_old.h);
        SDL_Point getOut = {inter.w, inter.h};

        //Get centers
        SDL_Point c1_old = KMath_Rect_GetCenter(pos1_old);
        SDL_Point c1 = KMath_Rect_GetCenter(pos1);
        SDL_Point c2 = KMath_Rect_GetCenter(pos2);

        SDL_Point c1_offset = {c1.x-c1_old.x, c1.y-c1_old.y};

        int value = 0;
        int centerOffsets = 0;
        int old_centerOffsets = 0;
        int obstacleSize = 0;
        int dir = 0;
        if(abs(c1_offset.x)>abs(c1_offset.y))
        {
            value=getOut.x;
            centerOffsets=c2.x-c1.x;
            old_centerOffsets=c1_offset.x;
            obstacleSize=pos2.w;
            dir = 0;
        }
        else
        {
            value=getOut.y;
            centerOffsets=c2.y-c1.y;
            old_centerOffsets=c1_offset.y;
            obstacleSize=pos2.h;
            dir = 1;
        }

        if(KMath_SignInt(centerOffsets)!=KMath_SignInt(old_centerOffsets))
        {
            value = obstacleSize-value;
            printf("Reversed!");
        }
        value*= -KMath_SignInt(old_centerOffsets);
        //SDL_Delay(2000);
        if(dir==0)
            return (SDL_Point){value, 0};
        else
            return (SDL_Point){0, value};
    }
    return (SDL_Point){0,0};
}*/


SDL_Texture *GameEntity_SpawnData_GetIcon(GameEntity_SpawnData *data, SDL_Rect *zone)
{
    char resource[256];
    SDL_Texture *tex = NULL;
    SpriteSheet *sheet = NULL;
    SDL_Point cellSize = {16,16};
    SDL_Point spritePos = {0,0};


    if(data->type==ENTITY_PLAYER)
    {
        strcpy(resource, RESOURCE_SPRITE_MARIO_LITTLE);
        cellSize = (SDL_Point) {16,16};
    }
    else if(data->type==ENTITY_GOOMBA)
    {
        strcpy(resource, RESOURCE_SPRITE_GOOMBA);
        cellSize = (SDL_Point) {16,16};
    }
    else if(data->type==ENTITY_BONUS)
    {
        strcpy(resource, RESOURCE_SPRITE_BONUS);
        cellSize = (SDL_Point) {16,16};
        if(data->args[0]==0)
            spritePos = (SDL_Point){0,0};
        else if(data->args[0]==1)
            spritePos = (SDL_Point){0,1};
        else if(data->args[0]==2)
            spritePos = (SDL_Point){1,1};
        else
            spritePos = (SDL_Point){3,1};
    }
    else if(data->type==ENTITY_BLOCK)
    {
        strcpy(resource, RESOURCE_SPRITE_BLOCK);
        cellSize = (SDL_Point) {16,16};
        if(data->args[0]==0)
            spritePos = (SDL_Point){0,0};
        else if(data->args[0]==1)
            spritePos = (SDL_Point){0,1};
        else
            spritePos = (SDL_Point){0,2};
    }
    else
    {
        return NULL;
    }

    tex = Hashmap_str_get(graphicalResources, resource);
    sheet = SpriteSheet_Create(tex, cellSize.x, cellSize.y);
    SDL_Rect rect = SpriteSheet_GetRectFromPos(sheet, spritePos.x, spritePos.y);
    memcpy(zone, &rect, sizeof(SDL_Rect));
    SpriteSheet_Destroy(sheet);
    return tex;
}