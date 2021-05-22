#ifndef KENGINE_H
#define KENGINE_H

#include <SDL2/SDL.h>
#include "kds.h"
#include "input.h"
#include "render.h"
#include "level.h"

typedef struct World
{
    Uint64 time_last;
    Uint64 time_current;
    double delta;
    
    int coins;
    int score;
    int lives;
    int isDead;
    int finished;

    InputActions *actions;
    List *entities;
    List *entities_toDelete;
    List *entities_toAdd;
    Level *level;
    Tileset *tileset;
    SpriteSheet *tilesetSheet;
    SDL_Renderer *renderer;
    Camera *camera;
} World;

World *World_Create(SDL_Renderer *renderer, char levelName[], int coins, int score, int lives);
void World_Update(World *world);


#endif