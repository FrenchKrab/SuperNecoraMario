#ifndef ENTITIES_H
#define ENTITIES_H


#include <SDL2/SDL.h>
#include "kmath.h"
#include "input.h"
#include "kds.h"
#include "resources.h"


#define ENTITY_TYPE_COUNT 4
#define SPAWNDATA_ARGS 16

typedef struct World World;

/*Entity type, defines an entity's behaviour*/
typedef enum
{
    ENTITY_PLAYER = 0,
    ENTITY_BONUS = 1,
    ENTITY_GOOMBA = 2,
    ENTITY_BLOCK = 3
} ENTITY_TYPE;

/*Contains spawn information about an entity*/
typedef struct GameEntity_SpawnData{
    int type;
    SDL_Point position;
    float args[SPAWNDATA_ARGS];
} GameEntity_SpawnData;

typedef struct s_e_behaviour GameEntity_Behaviour;

typedef struct CollisionData
{
    int up;
    int down;
    int left;
    int right;
} CollisionData;

/*An in-game entity's data*/
typedef struct {
    GameEntity_SpawnData spawnData;
    GameEntity_Behaviour *behaviour;
    void* runtimeData;

    Vect2 position;
    Vect2 position_last;
    Vect2 velocity;
    int worldCollisions;
    SDL_Point colliderSize;   /*Collision box*/
    Vect2 bounce;
    int entitySolid;
    CollisionData collisionData;
    List *spriteSheetRenderers;
} GameEntity;

typedef struct SpriteSheetRenderer{
    SpriteSheet *sheet;

    int visible;
    int activeAnimation;
    float playSpeed;
    int lastFrame;
    Uint32 lastFrameTick;
    SDL_Point position;

    SDL_Color tint;
    double angle;
    SDL_Point center;
    SDL_RendererFlip flip;
    int z;


    int animationCount;
    SpriteSheetAnimation *animations[];
} SpriteSheetRenderer;



/* Contains function pointers for a specific ENTITY_TYPE */
struct s_e_behaviour {
    void (*start)(World*, GameEntity*);
    void (*update)(World*, GameEntity*);
    void (*on_collision_entity)(World*,GameEntity*,GameEntity*,CollisionData,SDL_Rect);
};

void SetupEntityFunctions();

GameEntity *GameEntity_Create(GameEntity_SpawnData spawn);
void GameEntity_Delete(void*);
SDL_Rect GameEntity_GetCollisionRect(GameEntity *entity);

SpriteSheetRenderer *SpriteSheetRenderer_Create(SpriteSheet *sheet, int animationCount);
void SpriteSheetRenderer_ForcePlay(SpriteSheetRenderer *sheetRenderer, int animationId);
void SpriteSheetRenderer_Play(SpriteSheetRenderer *, int animationId);
SDL_Rect SpriteSheetRenderer_GetRenderRect(SpriteSheetRenderer *, Uint32 currentTick);
SDL_Rect SpriteSheetRenderer_GetDestRect(SpriteSheetRenderer *);

void StartAllEntities(World *world);
void UpdateAllEntities(World *world);
void DrawAllEntities(World *world);


SDL_Point Collision_GetOneRectOutOfCollision(SDL_Rect pos1_old, SDL_Rect pos1, SDL_Rect pos2);
void Collision_GetBoundaryCheckers(SDL_Rect r1, SDL_Rect r_other, int radius, CollisionData*);
void Collision_ApplyOffsetOnEntity(GameEntity *entity, SDL_Point outOffset);
void Collision_UpdateCollisionDataFromOffset(CollisionData*, SDL_Point);

CollisionData CollisionData_Reverse(CollisionData data);
CollisionData CollisionData_AddCollisions(CollisionData data1, CollisionData data2);
int CollisionData_HasCollision(CollisionData data);

SDL_Texture *GameEntity_SpawnData_GetIcon(GameEntity_SpawnData *data, SDL_Rect *zone);


#endif