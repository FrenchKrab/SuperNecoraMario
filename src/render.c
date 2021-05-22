#include <SDL2/SDL.h>
#include "render.h"
#include "kmath.h"
#include "kds.h"
#include "level.h"
#include "entities.h"


Camera *Camera_Create()
{
    Camera *cam = malloc(sizeof(Camera));
    cam->pos=(SDL_Point){0,0};
    for(int i = 0; i<CAMERA_Z_LAYERS; i++)
    {
        cam->spriteRenderQueues[i]=NULL;
    }
    return cam;
}

void Camera_AddSpriteToQueue(Camera *camera, SDL_Texture *texture, SDL_Rect source,
                            SDL_Rect dest, int z)
{
    Camera_AddSpriteToQueueEx(camera, texture, source, dest,
                            (SDL_Color){255,255,255,255},0,(SDL_Point){0,0},SDL_FLIP_NONE,z);
}

void Camera_AddSpriteToQueueEx(Camera *camera,
                            SDL_Texture *texture,
                            SDL_Rect source,
                            SDL_Rect dest,
                            SDL_Color tint,
                            double angle,
                            SDL_Point center,
                            SDL_RendererFlip flip,
                            int z)
{
    Camera_SpriteQueueElement *element = malloc(sizeof(Camera_SpriteQueueElement));
    element->texture=texture;
    element->source=source;
    element->dest=dest;
    element->tint=tint;
    element->angle=angle;
    element->center=center;
    element->flip=flip;
    List_add(&(camera->spriteRenderQueues[KMath_ClampInt(z,0,CAMERA_Z_LAYERS-1)]), element);
}


void Camera_DrawSpriteQueue(Camera *camera, SDL_Renderer *renderer)
{
    for(int i = 0; i<CAMERA_Z_LAYERS; i++)
    {    
        List *l = camera->spriteRenderQueues[i];
        while(l!=NULL)
        {
            Camera_SpriteQueueElement *elt = (Camera_SpriteQueueElement*)l->value;
            SDL_SetTextureColorMod(elt->texture, elt->tint.r, elt->tint.g, elt->tint.b);
            SDL_SetTextureAlphaMod(elt->texture, elt->tint.a);
            elt->dest.x -= camera->pos.x;
            elt->dest.y -= camera->pos.y;
            SDL_RenderCopyEx(renderer, elt->texture, &elt->source,
                            &elt->dest, elt->angle, &elt->center, elt->flip);
            l = l->next;
        }
    }
}

void Camera_ClearSpriteQueue(Camera *camera)
{
    for(int i = 0; i<CAMERA_Z_LAYERS; i++)
    {
        List *l = camera->spriteRenderQueues[i];
        List_free(l, free);
        camera->spriteRenderQueues[i]=NULL;
    }
}

/*SpriteSheetRenderer functions*/

/*
SpriteSheetRenderer *SpriteSheetRenderer_Create(SpriteSheet *sheet)
{
    SpriteSheetRenderer *ssr = malloc(sizeof(SpriteSheetRenderer));
    ssr->sheet = sheet;
    ssr->spriteX = 0;
    ssr->spriteY = 0;
    ssr->tint = (SDL_Color){0,0,0,255};
    ssr->drawRect=(SDL_Rect){0,0,50,50};
    ssr->z = CAMERA_Z_LAYERS/2;
    return ssr;
}

void SpriteSheetRenderer_AddToCameraQueue(SpriteSheetRenderer *r, Camera *c)
{
    Camera_AddSpriteToQueueEx(c,
                            r->sheet->texture,
                            SpriteSheet_GetRectFromPos(r->sheet, r->spriteX, r->spriteY),
                            r->drawRect,
                            r->tint,
                            r->angle,
                            r->center,
                            r->flip,
                            r->z);
}
*/

void Camera_AddSpriteSheetRendererToCameraQueue(SpriteSheetRenderer* r, Camera* c, Uint32 tick)
{
    Camera_AddSpriteToQueueEx(c,
                            r->sheet->texture,
                            SpriteSheetRenderer_GetRenderRect(r, tick),
                            SpriteSheetRenderer_GetDestRect(r),
                            r->tint,
                            r->angle,
                            r->center,
                            r->flip,
                            r->z);

}

/* Level functions */

void Camera_AddLevelToCameraQueue(Level* level, SpriteSheet *tilesetSheet, Camera* camera)
{
    for(int x=0; x<level->sizeX; x++)
    {
        for(int y=0; y<level->sizeY; y++)
        {
            int cellType = Level_Get_Cell(level, x, y);
            if(cellType!=-1)
            {
                SDL_Rect source = SpriteSheet_GetRectFromIndex(tilesetSheet, cellType);
                SDL_Rect dest = {0,0,0,0};
                dest.x = x*tilesetSheet->cellSize_x;
                dest.y = y*tilesetSheet->cellSize_y;
                dest.w = tilesetSheet->cellSize_x;
                dest.h = tilesetSheet->cellSize_y;
                Camera_AddSpriteToQueue(camera, tilesetSheet->texture, source, dest, 2);
            }

        }
    }
}