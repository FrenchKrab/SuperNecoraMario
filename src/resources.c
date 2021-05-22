#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <libgen.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "resources.h"
#include "kds.h"
#include "sdl_helper.h"
#include "kmath.h"

#define RESOURCES_DEBUG 0


char RESOURCE_SPRITE_MARIO_LITTLE[HASHPAIR_STR_KEYLENGHT] = "mario_little.png";
char RESOURCE_SPRITE_MARIO_BIG[HASHPAIR_STR_KEYLENGHT] = "mario_big.png";
char RESOURCE_SPRITE_BONUS[HASHPAIR_STR_KEYLENGHT] = "bonus.png";
char RESOURCE_SPRITE_GOOMBA[HASHPAIR_STR_KEYLENGHT] = "goomba.png";
char RESOURCE_SPRITE_WING[HASHPAIR_STR_KEYLENGHT] = "wings.png";
char RESOURCE_SPRITE_BLOCK[HASHPAIR_STR_KEYLENGHT] = "blocks.png";

char graphicalResourcesFolders[GRAPHICALRESOURCES_FOLDERS_COUNT][HASHPAIR_STR_KEYLENGHT] =
    {"sprites/"};

char levelFolder[] = "levels/";
char tilesetFolder[] = "tilesets/";

Hashmap_str *graphicalResources = NULL;

void SetupResources(SDL_Renderer *renderer)
{
    graphicalResources = Hashmap_str_create(GRAPHICALRESOURCES_HASHMAP_SIZE);
    //First retrieve every png in every folders
    List *graphicalResourcesNames = NULL;
    for(int i = 0; i<GRAPHICALRESOURCES_FOLDERS_COUNT; i++)
    {
        List_link(&graphicalResourcesNames, ScanDir(graphicalResourcesFolders[i]));
    }

    //Then browse them all and try to create textures
    List *graphicalList = graphicalResourcesNames;
    while(graphicalList != NULL)
    {
        SDL_Texture *tex = SDLH_LoadPNG(renderer, graphicalList->value);
        if(tex != NULL)
        {
            char *name = malloc(sizeof(char)* HASHPAIR_STR_KEYLENGHT);
            strcpy(name, basename(graphicalList->value));
            #if RESOURCES_DEBUG!=0
            printf("\nGonna add %s", name);
            #endif
            Hashmap_str_add(graphicalResources, name, tex);
        }
        graphicalList = graphicalList->next;
    }
    List_free(graphicalResourcesNames, free);
    #if RESOURCES_DEBUG!=0
    printf("\nInitialized resources");
    #endif
}

/* source : https://stackoverflow.com/questions/26357792/return-a-list-of-files-in-a-folder-in-c */
List *ScanDir(char dirname[])
{
    List *dirList = NULL;
    DIR *d = NULL;
    struct dirent *dir = NULL;

    d = opendir(dirname);

    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            if(dir->d_type == DT_REG)   //If it is a file (not a folder/symbolic link/etc)
            {
                char *name = malloc(sizeof(char)*HASHPAIR_STR_KEYLENGHT);
                strcpy(name, dirname);
                strcat(name, dir->d_name);
                #if RESOURCES_DEBUG!=0
                printf("\nFound file : %s", dir->d_name);
                #endif
                List_append(&dirList, name);
            }
        }
        closedir(d);
    }
    return dirList;
}


SpriteSheet *SpriteSheet_Create(SDL_Texture *texture, int cellSize_x, int cellSize_y)
{
    SpriteSheet *sheet = malloc(sizeof(SpriteSheet));
    sheet->texture = texture;
    sheet->cellSize_x = cellSize_x;
    sheet->cellSize_y = cellSize_y;
    if(texture==NULL || SDL_QueryTexture(texture, NULL, NULL, &sheet->width, &sheet->height) < 0)
    {
        printf("\nWARNING : couldnt SDL_QueryTexture for sprite sheet :\n%s\n", SDL_GetError());
    }
    //printf("Loaded (%d,%d;%d,%d)", sheet->cellSize_x, sheet->cellSize_y, sheet->width, sheet->height);
    return sheet;
}

void SpriteSheet_Destroy(SpriteSheet *sheet)
{
    if(sheet!=NULL)
    {
        free(sheet);
    }
}

SDL_Rect SpriteSheet_GetRectFromPos(SpriteSheet *sheet, int posX, int posY)
{
    SDL_Rect rect;
    rect.x = KMath_ClampInt(posX*sheet->cellSize_x,0, sheet->width);  //Clamp x between 0 and width
    rect.y = KMath_ClampInt(posY*sheet->cellSize_y, 0, sheet->height);
    rect.w = KMath_ClampInt(sheet->cellSize_x, 0, sheet->width - rect.x); //Make sure 0<x+w<sheet.width
    rect.h = KMath_ClampInt(sheet->cellSize_y, 0, sheet->height - rect.y);
    return rect;
}

SDL_Rect SpriteSheet_GetRectFromIndex(SpriteSheet *sheet, int index)
{
    int cellCountX = sheet->width/sheet->cellSize_x;
    int x = index%cellCountX;
    int y = index/cellCountX;
    return SpriteSheet_GetRectFromPos(sheet, x, y);
}

void SpriteSheet_RenderCopy_FromPos(SDL_Renderer *renderer,
                                    SpriteSheet *sheet,
                                    int x, int y,
                                    SDL_Rect destination,
                                    SDL_RendererFlip flip)
{
    SDL_Rect source = SpriteSheet_GetRectFromPos(sheet, x, y);
    SDL_RenderCopyEx(renderer, sheet->texture, &source, &destination, 0, NULL, flip);
}

void SpriteSheet_RenderCopy_FromIndex(SDL_Renderer *renderer, SpriteSheet *sheet,
                                        int index, SDL_Rect destination, SDL_RendererFlip flip)
{
    int cellCountX = sheet->width/sheet->cellSize_x;
    int x = index%cellCountX;
    int y = index/cellCountX;
    SpriteSheet_RenderCopy_FromPos(renderer, sheet, x, y, destination, flip);
}


void SpriteSheet_ASCII_Write(SDL_Renderer *renderer, SpriteSheet *font, char text[] , int height, SDL_Point pos)
{
    int maxRows = font->width/font->cellSize_x;    //X
    int charWidth = (int)(((float)height/(float)font->cellSize_y)*font->cellSize_x);
    int currentLine = 0;
    int currentCharX = 0;
    for (int i = 0; i < strlen(text); i++)
    {
        if(text[i]=='\n')
        {
            currentCharX = 0;
            currentLine++;
        }
        else
        {
            int ascii = (int)text[i];
            SDL_Rect dest = {pos.x + charWidth*currentCharX, pos.y + height*currentLine, charWidth, height};
            SpriteSheet_RenderCopy_FromPos(renderer, font, ascii%maxRows, ascii/maxRows, dest, 0);
            currentCharX++;
        }
    }
}


SpriteSheetAnimation *SpriteSheetAnimation_Create(int frameCount, int frames[])
{
    SpriteSheetAnimation *anim = malloc(sizeof(SpriteSheetAnimation)+sizeof(int)*frameCount);
    anim->frameCount=frameCount;
    memcpy(anim->frames,frames, sizeof(int)*frameCount);
    return anim;
}


void String_AddPrefix(char str[], char prefix[])
{
    char *strTmp = strdup(str);    //Contains str
    strcpy(str, prefix);  //str contains prefix
    strcat(str, strTmp);   //str contains prefix+str
    free(strTmp);
}