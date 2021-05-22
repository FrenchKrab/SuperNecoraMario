#include "goomba.h"
#include "../entities.h"
#include "../kengine.h"

SpriteSheet *sheetGoomba = NULL;
SpriteSheet *sheetWing = NULL;

#define GOOMBA_WALKSPEED 16
#define GOOMBA_GRAVITY 512
#define GOOMBA_DISAPPEARTIME 0.8

#define GOOMBA_ANIMATION_WALK 0
#define GOOMBA_ANIMATION_DIE 1

GoombaData *GoombaData_Create()
{
    GoombaData *data = malloc(sizeof(GoombaData));
    data->timeSinceJumpedOn = -1;
    data->sr_wing_left = NULL;
    data->sr_wing_right = NULL;
    data->sr_goomba = NULL;
    return data;
}


void GOOMBA_start(World *world, GameEntity *self)
{

    GoombaData *data = GoombaData_Create();
    self->runtimeData = data;

    int isParagoomba = self->spawnData.args[0];

    //Init spritesheet if needed
    if(sheetGoomba==NULL)
    {
        SDL_Texture *goombaTex = Hashmap_str_get(graphicalResources, RESOURCE_SPRITE_GOOMBA);
        if(goombaTex!=NULL)
        {
            sheetGoomba=SpriteSheet_Create(goombaTex, 16, 16);
        }
    }
    if(sheetWing == NULL)
    {
        SDL_Texture *wingTex = Hashmap_str_get(graphicalResources, RESOURCE_SPRITE_WING);
        if(wingTex!=NULL)
        {
            sheetWing=SpriteSheet_Create(wingTex, 8, 16);
        }
    }

    //Init renderers
    //Goomba
    SpriteSheetRenderer *sr_g = SpriteSheetRenderer_Create(sheetGoomba, 2);
    sr_g->animations[GOOMBA_ANIMATION_WALK]=SpriteSheetAnimation_Create(2,(int[]){0,1});
    sr_g->animations[GOOMBA_ANIMATION_DIE]=SpriteSheetAnimation_Create(1,(int[]){2});
    List_add(&self->spriteSheetRenderers, sr_g);
    data->sr_goomba=sr_g;

    if(isParagoomba)
    {
        SpriteSheetRenderer *sr_w_l = SpriteSheetRenderer_Create(sheetWing, 1);
        sr_w_l->animations[0]=SpriteSheetAnimation_Create(2,(int[]){0,1});
        List_add(&self->spriteSheetRenderers, sr_w_l);
        data->sr_wing_left=sr_w_l;

        SpriteSheetRenderer *sr_w_r = SpriteSheetRenderer_Create(sheetWing, 1);
        sr_w_r->animations[0]=SpriteSheetAnimation_Create(2,(int[]){0,1});
        List_add(&self->spriteSheetRenderers, sr_w_r);
        data->sr_wing_right=sr_w_r;

        SpriteSheetRenderer_Play(sr_w_l,0);
        SpriteSheetRenderer_Play(sr_w_r,0);
        sr_w_l->flip=SDL_FLIP_HORIZONTAL;
        
        sr_w_r->playSpeed = 7;
        sr_w_l->playSpeed = 7;
    }

    SpriteSheetRenderer_Play(sr_g, 0);
    sr_g->position=Vect2_ToPoint(self->position);
    sr_g->playSpeed=8;

    sr_g->visible=1;

    self->worldCollisions = 1;
    self->colliderSize = (SDL_Point) {16,16};

    if(isParagoomba)
    {
        self->velocity = (Vect2) {-GOOMBA_WALKSPEED,0};
        self->bounce = (Vect2){1,1};
    }
    else
    {
        self->velocity = (Vect2) {-GOOMBA_WALKSPEED,0};
        self->bounce = (Vect2){1,0};
    }

    self->entitySolid=0;
}

void GOOMBA_update(World *world, GameEntity *self)
{
    GoombaData *data = (GoombaData*)self->runtimeData;
    SDL_Point pos = Vect2_ToPoint(self->position);
    data->sr_goomba->position=pos;
    if(self->spawnData.args[0] == 1)
    {
        data->sr_wing_left->position=(SDL_Point){pos.x-4,pos.y-8};
        data->sr_wing_right->position=(SDL_Point){pos.x+12,pos.y-8};
    }



    if(data->timeSinceJumpedOn == -1)
    {
        self->velocity.y += GOOMBA_GRAVITY * world->delta;
    }
    else
    {
        data->timeSinceJumpedOn += world->delta;
        if(data->timeSinceJumpedOn >= GOOMBA_DISAPPEARTIME)
        {
            List_addUnique(&world->entities_toDelete, self);
        }
    }
}


void GOOMBA_onCollision(World* world, GameEntity* self, GameEntity* other, CollisionData colData, SDL_Rect inter)
{
    GoombaData *data = (GoombaData*)self->runtimeData;
   
    if(other->spawnData.type == ENTITY_PLAYER && colData.up)
    {
        SpriteSheetRenderer_Play(data->sr_goomba, GOOMBA_ANIMATION_DIE);
        data->timeSinceJumpedOn = 0;
        self->velocity=(Vect2){0,0};
        self->colliderSize=(SDL_Point){0,0};
        world->score+=100;
    }
}