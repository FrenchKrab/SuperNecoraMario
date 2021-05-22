#ifndef RESOURCES_H
#define RESOURCES_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "kds.h"

#define GRAPHICALRESOURCES_HASHMAP_SIZE 20
#define GRAPHICALRESOURCES_FOLDERS_COUNT 1 


extern char RESOURCE_SPRITE_MARIO_LITTLE[HASHPAIR_STR_KEYLENGHT];
extern char RESOURCE_SPRITE_MARIO_BIG[HASHPAIR_STR_KEYLENGHT];
extern char RESOURCE_SPRITE_BONUS[HASHPAIR_STR_KEYLENGHT];
extern char RESOURCE_SPRITE_GOOMBA[HASHPAIR_STR_KEYLENGHT];
extern char RESOURCE_SPRITE_WING[HASHPAIR_STR_KEYLENGHT];
extern char RESOURCE_SPRITE_BLOCK[HASHPAIR_STR_KEYLENGHT];

extern char graphicalResourcesFolders[GRAPHICALRESOURCES_FOLDERS_COUNT][HASHPAIR_STR_KEYLENGHT];

extern char levelFolder[];
extern char tilesetFolder[];

extern Hashmap_str *graphicalResources;

typedef struct
{
    int cellSize_x, cellSize_y;
    int width, height;
    SDL_Texture *texture;
} SpriteSheet;

typedef struct
{
    int frameCount;
    int frames[];
}
SpriteSheetAnimation;

void SetupResources(SDL_Renderer *renderer);
List *ScanDir(char dirname[]);

SpriteSheet *SpriteSheet_Create(SDL_Texture *texture, int cellSize_x, int cellSize_y);
void SpriteSheet_Destroy(SpriteSheet *sheet);

SDL_Rect SpriteSheet_GetRectFromPos(SpriteSheet *sheet, int posX, int posY);
SDL_Rect SpriteSheet_GetRectFromIndex(SpriteSheet *sheet, int index);

void SpriteSheet_RenderCopy_FromPos(SDL_Renderer *renderer,
                                    SpriteSheet *sheet,
                                    int x, int y,
                                    SDL_Rect destination,
                                    SDL_RendererFlip flip);
void SpriteSheet_RenderCopy_FromIndex(SDL_Renderer *renderer, SpriteSheet *sheet,
                                        int index, SDL_Rect destination, SDL_RendererFlip flip);
void SpriteSheet_ASCII_Write(SDL_Renderer *renderer, SpriteSheet *font, char text[] , int height, SDL_Point pos);

SpriteSheetAnimation *SpriteSheetAnimation_Create(int, int[]);

//Add prefix at the start of str
void String_AddPrefix(char str[], char prefix[]);

#endif