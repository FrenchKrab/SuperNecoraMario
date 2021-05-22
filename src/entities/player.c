#include "player.h"
#include "../entities.h"
#include "../kengine.h"

#define MARIO_MAXSPEED_WALK 80
#define MARIO_MAXSPEED_RUN 150
#define MARIO_ACCELERATION 150
#define MARIO_BRAKEMULTIPLIER 3
#define MARIO_JUMP_IMPULSE 190
#define MARIO_JUMP_FORCE 250
#define MARIO_MAXJUMPTIME 0.6
#define MARIO_MINJUMPTIME 0.05
#define MARIO_DEATH_TIME 2
#define MARIO_DEATH_ULTIMATEJUMPIMPULSE 300
#define MARIO_DEATH_GRAVITY 700
#define MARIO_TOUCHED_INVINCIBILITYTIME 1

#define MARIO_RUNANIMATIONSPEED 15

#define MARIO_ANIMATION_RUN 0
#define MARIO_ANIMATION_JUMP 1
#define MARIO_ANIMATION_TURN 2
#define MARIO_ANIMATION_DIE 3

SpriteSheet *sheetMarioLittle = NULL;
SpriteSheet *sheetMarioBig = NULL;



MarioData *MarioData_Create(SpriteSheetRenderer *ssr_marioLittle, SpriteSheetRenderer *ssr_marioBig)
{
    MarioData *data = malloc(sizeof(MarioData));
    data->power=MARIO_POWER_LITTLE;
    data->activeSpriteSheetRenderer=ssr_marioLittle;
    data->rendererOffset=(SDL_Point){0,0};
    
    data->ssr_marioLittle=ssr_marioLittle;
    data->ssr_marioBig=ssr_marioBig;

    data->jumping=0;
    data->jumpingTime=0;

    data->dieAnimationTime = -1;
    data->invincibilityTimeLeft = 0;
    return data;
}

void PLAYER_start(World *world, GameEntity *self)
{
    self->velocity = (Vect2) {16,0};

    //Init spritesheet if needed
    if(sheetMarioLittle==NULL)
    {
        SDL_Texture *marioLittleTex = Hashmap_str_get(graphicalResources, RESOURCE_SPRITE_MARIO_LITTLE);
        if(marioLittleTex!=NULL)
        {
            sheetMarioLittle=SpriteSheet_Create(marioLittleTex, 16, 16);
        }
    }
    if(sheetMarioBig==NULL)
    {
        SDL_Texture *marioBigTex = Hashmap_str_get(graphicalResources, RESOURCE_SPRITE_MARIO_BIG);
        if(marioBigTex!=NULL)
        {
            sheetMarioBig=SpriteSheet_Create(marioBigTex, 32, 32);
        }
    }

    //Init renderers
    //Mario Little
    SpriteSheetRenderer *sr_ml = SpriteSheetRenderer_Create(sheetMarioLittle, 6);
    sr_ml->animations[MARIO_ANIMATION_RUN]=SpriteSheetAnimation_Create(2,(int[]){0,1});
    sr_ml->animations[MARIO_ANIMATION_JUMP]=SpriteSheetAnimation_Create(1,(int[]){16});
    sr_ml->animations[MARIO_ANIMATION_TURN]=SpriteSheetAnimation_Create(1,(int[]){64});
    sr_ml->animations[MARIO_ANIMATION_DIE]=SpriteSheetAnimation_Create(1,(int[]){65});
    List_add(&self->spriteSheetRenderers, sr_ml);

    //Mario big
    SpriteSheetRenderer *sr_mb = SpriteSheetRenderer_Create(sheetMarioBig, 6);
    sr_mb->animations[MARIO_ANIMATION_RUN]=SpriteSheetAnimation_Create(4,(int[]){0,1,2,1});
    sr_mb->animations[MARIO_ANIMATION_JUMP]=SpriteSheetAnimation_Create(1,(int[]){8});
    sr_mb->animations[MARIO_ANIMATION_TURN]=SpriteSheetAnimation_Create(1,(int[]){32});
    sr_mb->animations[MARIO_ANIMATION_DIE]=SpriteSheetAnimation_Create(1,(int[]){32});
    List_add(&self->spriteSheetRenderers, sr_mb);

    self->runtimeData=MarioData_Create(sr_ml, sr_mb);

    SpriteSheetRenderer_Play(sr_ml, 0);
    sr_ml->position=Vect2_ToPoint(self->position);
    sr_ml->playSpeed=0;

    sr_mb->visible=0;

    self->worldCollisions=1;
    self->colliderSize = (SDL_Point) {16,16};
    self->bounce=(Vect2){0,0};
}


void PLAYER_update(World *world, GameEntity *self)
{
    MarioData *data = (MarioData*)(self->runtimeData);

    if(data->dieAnimationTime==-1)  //If alive
    {
        self->velocity.y += 512 * world->delta;

        SDL_Point screenSize;
        SDL_GetRendererOutputSize(world->renderer, &screenSize.x, &screenSize.y);
        world->camera->pos=Vect2_ToPoint(self->position);
        world->camera->pos.x-=screenSize.x/2;
        world->camera->pos.y-=screenSize.y/2;

        int movement = 0;
        if(Input_IsAnyDown(world->actions->right) && !self->collisionData.right)
        {
            movement = 1;
            data->activeSpriteSheetRenderer->flip=SDL_FLIP_NONE;
        }
        else if(Input_IsAnyDown(world->actions->left) && !self->collisionData.left)
        {
            movement = -1;
            data->activeSpriteSheetRenderer->flip=SDL_FLIP_HORIZONTAL;
        }



        if(movement!=0)
        {
            if(KMath_SignInt(movement)!=KMath_SignInt(self->velocity.x))
            {
                movement *= MARIO_BRAKEMULTIPLIER;
                SpriteSheetRenderer_ForcePlay(data->activeSpriteSheetRenderer, MARIO_ANIMATION_TURN);
            }
            else
            {
                SpriteSheetRenderer_Play(data->activeSpriteSheetRenderer, MARIO_ANIMATION_RUN);
            }
    
            self->velocity.x += world->delta*(float)movement*MARIO_ACCELERATION;

            float speedLimiter = MARIO_MAXSPEED_WALK;
            if(Input_IsAnyDown(world->actions->run))
            {
                speedLimiter = MARIO_MAXSPEED_RUN;
            }
            self->velocity.x = KMath_ClampFloat(self->velocity.x, -speedLimiter,speedLimiter);
        }
        else if(self->collisionData.down)
        {
            if(fabs(self->velocity.x)<1)
                self->velocity.x=0;
            else
                self->velocity.x = self->velocity.x - 256*world->delta*KMath_SignInt(self->velocity.x);
        }
        
        if(!self->collisionData.down)
        {
            SpriteSheetRenderer_Play(data->activeSpriteSheetRenderer, MARIO_ANIMATION_JUMP);
        }
        else if(data->activeSpriteSheetRenderer->activeAnimation==MARIO_ANIMATION_JUMP)
        {
            SpriteSheetRenderer_Play(data->activeSpriteSheetRenderer, MARIO_ANIMATION_RUN);
        }

        data->activeSpriteSheetRenderer->playSpeed = MARIO_RUNANIMATIONSPEED* fabs(self->velocity.x/MARIO_MAXSPEED_RUN);

        if(world->actions->jump==INPUT_JUSTDOWN && self->collisionData.down)
        {
            self->velocity.y = -MARIO_JUMP_IMPULSE;
            data->jumping=1;
            data->jumpingTime=0;
        }

        if(((Input_IsAnyDown(world->actions->jump) && data->jumping) || data->jumpingTime<MARIO_MINJUMPTIME) && data->jumpingTime <=MARIO_MAXJUMPTIME)
        {
            self->velocity.y += -MARIO_JUMP_FORCE * world->delta;
            data->jumpingTime += world->delta;
        }
        else if(data->jumping)
        {
            data->jumping=0;
            self->velocity.y /= 4;
        }

        //If collide with roof
        if(self->collisionData.up)
        {
            data->jumping=0;
        }

        //Check death
        if(self->position.y>world->level->sizeY*world->tileset->cellSize_y)
            PLAYER_Die(world,self);  
        //Check level end
        if(self->position.x>(world->level->sizeX-1)*world->tileset->cellSize_x)
            world->finished = 1;

        //Invicibility time
        if(data->invincibilityTimeLeft>0)
        {
            data->activeSpriteSheetRenderer->tint.a = 130;
            data->invincibilityTimeLeft-=world->delta;
        }
        else
        {
            data->activeSpriteSheetRenderer->tint.a = 255;
        }
    }
    else    //If dead
    {
        if(data->dieAnimationTime<MARIO_DEATH_TIME)
        {
            data->dieAnimationTime+=world->delta;
            
            self->velocity.y += MARIO_DEATH_GRAVITY * world->delta;
        }
        else
        {
            world->isDead = 1;

        }
    }
    data->activeSpriteSheetRenderer->position=Vect2_ToPoint(self->position);
    data->activeSpriteSheetRenderer->position.x += data->rendererOffset.x;
    data->activeSpriteSheetRenderer->position.y += data->rendererOffset.y;


    //Coins check
    if(world->coins>=100)
    {
        world->coins-=100;
        world->lives++;
    }

}

void PLAYER_onCollision(World* world, GameEntity* self, GameEntity* other, CollisionData colData, SDL_Rect inter)
{
    MarioData *data = (MarioData*)(self->runtimeData);
    ENTITY_TYPE otherType = other->spawnData.type;
    

    SDL_Rect other_rect = GameEntity_GetCollisionRect(other);
    SDL_Point center_other = KMath_Rect_GetCenter(other_rect);

    if((otherType == ENTITY_GOOMBA))
    {
        if(colData.down || center_other.y-self->position.y-self->colliderSize.y>=0)
        {
            self->velocity.y = -MARIO_JUMP_FORCE;
            data->jumping=1;
            data->jumpingTime=0;
        }
        else
        {
            PLAYER_GetDamaged(world, self);
        }
    }
}


void PLAYER_Die(World *world, GameEntity *self)
{
    MarioData *data = (MarioData*)(self->runtimeData);


    data->dieAnimationTime = 0;
    self->velocity.y=-MARIO_DEATH_ULTIMATEJUMPIMPULSE;
    self->velocity.x=0;
    //self->colliderSize = (SDL_Point){0,0};
    self->worldCollisions=0;
    if(data->power!=MARIO_POWER_LITTLE)
        PLAYER_ChangePower(world, self, MARIO_POWER_LITTLE);

    SpriteSheetRenderer_Play(data->activeSpriteSheetRenderer, MARIO_ANIMATION_DIE);
}

void PLAYER_GetDamaged(World *world, GameEntity *self)
{
    MarioData *data = (MarioData*)(self->runtimeData);
    if(data->invincibilityTimeLeft<=0)  //If player isn't invincible
    {
        if(data->power==MARIO_POWER_LITTLE) //If he gets damaged while already little, he dies
        {
            PLAYER_Die(world, self);
        }
        else    //Else he goes back to being little
        {
            data->power=MARIO_POWER_LITTLE;
            PLAYER_PowerTransition(world, self);
            data->invincibilityTimeLeft = MARIO_TOUCHED_INVINCIBILITYTIME;
        }
    }

}

void PLAYER_ChangePower(World *world, GameEntity *self, MARIO_POWER power)
{
    MarioData *data = (MarioData*)(self->runtimeData);
    data->power = power;
    PLAYER_PowerTransition(world, self);
}

void PLAYER_PowerTransition(World *world, GameEntity *self)
{
    MarioData *data = (MarioData*)(self->runtimeData);
    SpriteSheet *newTargetSheet = NULL;
    SpriteSheetRenderer *oldSr = data->activeSpriteSheetRenderer;
    SpriteSheetRenderer *newSr = NULL;

    if(data->power==MARIO_POWER_LITTLE)
    {
        newTargetSheet = sheetMarioLittle; 
    }
    else
    {
        newTargetSheet = sheetMarioBig;
    }

    List *l = self->spriteSheetRenderers;
    while(l!=NULL)
    {
        if(((SpriteSheetRenderer*)l->value)->sheet==newTargetSheet)
        {
            newSr = l->value;
        }

        l = l->next;
    }

    if(newSr!=NULL)
    {
        oldSr->visible=0;
        newSr->visible=1;
        data->activeSpriteSheetRenderer=newSr;

        if(data->power==MARIO_POWER_LITTLE)
        {
            self->position.y+=8;
            self->colliderSize=(SDL_Point){16,16};
            data->rendererOffset=(SDL_Point){0,0};
        }
        else
        {
            self->colliderSize=(SDL_Point){16,24};
            data->rendererOffset=(SDL_Point){-8,-8};
        }
    }
}