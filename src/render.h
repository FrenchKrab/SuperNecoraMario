#ifndef RENDER_H
#define RENDER_H

#include <SDL2/SDL.h>
#include "kds.h"
#include "resources.h"
#include "level.h"

#define CAMERA_Z_LAYERS 20

typedef struct SpriteSheetRenderer SpriteSheetRenderer;


/*TODO	Could be useful ?	SDL_RenderSetScale(renderer, 4,2);*/
typedef struct
{   
    SDL_Renderer *renderer;
    List *spriteRenderQueues[CAMERA_Z_LAYERS];
    SDL_Point pos;
} Camera;

typedef struct
{
    SDL_Texture *texture;
    SDL_Rect source;
    SDL_Rect dest;
    SDL_Color tint;
    double angle;
    SDL_Point center;
    SDL_RendererFlip flip;
} Camera_SpriteQueueElement;

/*
typedef struct
{
    SpriteSheet *sheet;
    int spriteX, spriteY; //Current sheet value

    SDL_Rect drawRect;

    SDL_Color tint;
    double angle;
    SDL_Point center;
    SDL_RendererFlip flip;

    int z;
} SpriteSheetRenderer;
*/

/*Camera functions */
Camera *Camera_Create();

void Camera_AddSpriteToQueue(Camera *camera,
                            SDL_Texture *texture,
                            SDL_Rect source,
                            SDL_Rect dest,
                            int z);

/*Append a sprite to the spriteRenderQueues list (at the right z level)*/
void Camera_AddSpriteToQueueEx(Camera *camera,
                            SDL_Texture *texture,
                            SDL_Rect source,
                            SDL_Rect dest,
                            SDL_Color tint,
                            double angle,
                            SDL_Point center,
                            SDL_RendererFlip flip,
                            int z);

/*Draw everything in the spriteRenderQueues*/
void Camera_DrawSpriteQueue(Camera *camera, SDL_Renderer *renderer);

void Camera_ClearSpriteQueue(Camera *camera);

/*SpriteSheetRenderer functions*/
/*SpriteSheetRenderer *SpriteSheetRenderer_Create(SpriteSheet*);
void SpriteSheetRenderer_AddToCameraQueue(SpriteSheetRenderer*, Camera*);*/
void Camera_AddSpriteSheetRendererToCameraQueue(SpriteSheetRenderer* r, Camera* c, Uint32 tick);

void Camera_AddLevelToCameraQueue(Level*, SpriteSheet*, Camera*);


#endif