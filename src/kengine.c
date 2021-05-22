#include <SDL2/SDL.h>
#include "kengine.h"
#include "entities.h"
#include "sdl_helper.h"


const SDL_Color clearColor = {0,127,255,255};


World *World_Create(SDL_Renderer *renderer, char levelName[], int coins, int score, int lives)
{
    World *world = malloc(sizeof(World));
    world->time_current = SDL_GetPerformanceCounter();
    world->time_last = world->time_current;

    world->coins = coins;
    world->lives = lives;
    world->score = score;
    world->isDead = 0;
    world->finished = 0;

    world->actions=InputActions_Create();
    world->entities=NULL;
    world->entities_toDelete=NULL;
    world->entities_toAdd=NULL;
    world->camera=Camera_Create();
    world->renderer=renderer;

    //Load level
    char levelPath[HASHPAIR_STR_KEYLENGHT];
    strcpy(levelPath, levelName);
    String_AddPrefix(levelPath, levelFolder);
    world->level = Level_Load(levelPath);

    if(world->level != NULL)
    {
        world->tileset=Tileset_Load(world->level->tileset);
        SDL_Texture *tilesetTex = Hashmap_str_get(graphicalResources, world->tileset->textureName);
        if(tilesetTex != NULL)
        {
            world->tilesetSheet=SpriteSheet_Create(tilesetTex,
                                                    world->tileset->cellSize_x,
                                                    world->tileset->cellSize_y);
        }
        for(int i = 0; i<world->level->entityCount; i++)
        {
            GameEntity_SpawnData data = world->level->entities_spawns[i];
            GameEntity *newEntity = GameEntity_Create(data);
            List_add(&world->entities, newEntity);
        }
    }

    StartAllEntities(world);

    return world;
}


void World_Update(World *world)
{
    /* STEP 1: get inputs */
    world->actions = InputActions_PollFromSDL(world->actions);

    /* STEP 2 : get & update time data */
    world->time_last = world->time_current;
    world->time_current = SDL_GetPerformanceCounter();
    world->delta = ((world->time_current - world->time_last)*1000
                    / (double)SDL_GetPerformanceFrequency())/1000;
    /*world->time_last = world->time_current;
    world->time_current = SDL_GetTicks();
    world->delta = KMath_MaxInt(world->time_current - world->time_last,1)/(float)1000;*/

    /* STEP 3 : Update entities */
    UpdateAllEntities(world);


    /* STEP 4 : Render everything */
    //clear renderer
    SDLH_ClearRenderer(world->renderer, clearColor);

    Camera_AddLevelToCameraQueue(world->level, world->tilesetSheet, world->camera);
    DrawAllEntities(world);

    Camera_DrawSpriteQueue(world->camera, world->renderer);
    Camera_ClearSpriteQueue(world->camera);


    /*STEP 5: Delete entities that need to be deleted*/
    while(world->entities_toDelete != NULL)
    {
        List_remove(&world->entities, world->entities_toDelete->value, NULL);
        List_remove(&world->entities_toDelete, world->entities_toDelete->value, GameEntity_Delete);
    }

    /*STEP 6: Add entities that need to be added*/
    while(world->entities_toAdd != NULL)
    {
        GameEntity *entity = world->entities_toAdd->value;
        List_add(&world->entities, entity);
        List_remove(&world->entities_toAdd, entity, NULL);
        if(entity->behaviour != NULL && entity->behaviour->start != NULL)
            entity->behaviour->start(world, entity);
    }
    

    //SDL_RenderPresent(world->renderer);
}