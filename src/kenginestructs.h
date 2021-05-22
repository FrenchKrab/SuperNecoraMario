#ifndef WORLD_H
#define WORLD_H

#include <SDL2/SDL.h>
#include "kds.h"
#include "input.h"
#include "kmath.h"

#define ENTITY_TYPE_COUNT 3
#define SPAWNDATA_ARGS 16


/* ---------Entity structs---------*/

/*Entity type, defines an entity's behaviour*/
typedef enum 
{
    ENTITY_PLAYER = 0,
    ENTITY_BONUS = 1,
    ENTITY_GOOMBA = 2
}ENTITY_TYPE;


/*Contains spawn information about an entity*/
typedef struct {
    int type;
    SDL_Point position;
    float args[SPAWNDATA_ARGS];
} GameEntity_SpawnData;

typedef struct s_e_behaviour GameEntity_Behaviour;

/*An in-game entity's data*/
typedef struct {
    GameEntity_SpawnData *spawnData;
    GameEntity_Behaviour *behaviour;

    Vect2 position;
    Vect2 velocity;
    SDL_Rect col;   /*Collision box*/
    List spriteSheetRenderers;
} GameEntity;

/* Contains function pointers for a specific ENTITY_TYPE */
struct s_e_behaviour {
    void (*start)(GameEntity*);
    void (*update)(World*, GameEntity*);
    void (*on_collision_entity)(GameEntity*,GameEntity*);
    void (*input)(GameEntity*,InputActions*);
};

/* ---------Entity structs---------*/

typedef struct
{
    Uint64 time_last;
    Uint64 time_current;
    double delta;
    
    InputActions *actions;
    List *entities;
    Level *level;
    SDL_Renderer *renderer;
    Camera *camera;
} World;






World *World_Create(SDL_Renderer*, char level[]);
void World_Update(World *world);


#endif