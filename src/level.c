#include <stdio.h>
#include <SDL2/SDL.h>
#include "kds.h"
#include "level.h"
#include "entities.h"


Level *Level_Create(int width, int height, const char tileset[])
{
    Level *lvl = malloc(sizeof(Level));
	lvl->sizeX=width;
	lvl->sizeY=height;
	lvl->entityCount=0;
	strcpy(lvl->tileset, tileset);
	lvl->entities_spawns=malloc(sizeof(GameEntity_SpawnData)*lvl->entityCount);
	lvl->cells=malloc(sizeof(int)*lvl->sizeX*lvl->sizeY);
        
    for (int i = 0; i<lvl->sizeX*lvl->sizeY; i++)
    {
        *(lvl->cells + i)=-1;
    }
    return lvl;
}

void Level_Destroy(Level *lvl)
{
    if(lvl!=NULL)
    {
        free(lvl->cells);
        if(lvl->entities_spawns!=NULL)
            free(lvl->entities_spawns);
        free(lvl);
    }
}

void Level_Save(Level *level, const char path[])
{
    FILE *fp;

    if((fp=fopen(path, "wb"))==NULL) {
        printf("Level_save: Cannot open file.\n");
        return;
    }


    fwrite(level->tileset, sizeof(char), HASHPAIR_STR_KEYLENGHT, fp);
    fwrite(&level->sizeX, sizeof(int), 1, fp);
    fwrite(&level->sizeY, sizeof(int), 1, fp);
    fwrite(level->cells, sizeof(int), level->sizeX*level->sizeY, fp);
    fwrite(&level->entityCount, sizeof(int), 1, fp);
    fwrite(level->entities_spawns, sizeof(GameEntity_SpawnData), level->entityCount, fp);

    fclose(fp);
}

Level *Level_Load(const char path[])
{
    FILE *fp;
    if((fp=fopen(path, "rb"))==NULL) {
        printf("Level_load: Cannot open file.\n");
    }
    
    Level *level = malloc(sizeof(Level));
    fread(level->tileset, sizeof(char), HASHPAIR_STR_KEYLENGHT, fp);

    fread(&level->sizeX, sizeof(int), 1, fp);
    fread(&level->sizeY, sizeof(int), 1, fp);
    level->cells = malloc(sizeof(int)*level->sizeX*level->sizeY);
    fread(level->cells, sizeof(int), level->sizeX*level->sizeY, fp);

    fread(&level->entityCount, sizeof(int), 1, fp);
    level->entities_spawns = malloc(sizeof(GameEntity_SpawnData)*level->entityCount);
    fread(level->entities_spawns, sizeof(GameEntity_SpawnData), level->entityCount, fp);
    fclose(fp);

    return level;
}

int Level_Get_Cell(Level *lvl, int x, int y)
{
    return *(lvl->cells + x + y*lvl->sizeX);
}

//Renvoie le type de collision la cellule placée en (x,y) dans le niveau
int Level_Get_Cell_Type(Level *lvl, Tileset *tileset, int x, int y)
{
    int cellValue = Level_Get_Cell(lvl,x,y);
    if(cellValue!=-1)
        return*(tileset->tilesCollisions + cellValue);
    else
        return TILECOL_NONE;
}



Tileset *Tileset_Create(char textureName[], int cellSize_x, int cellSize_y)
{
    Tileset *tileset = malloc(sizeof(tileset));
    SDL_Texture *tex = Hashmap_str_get(graphicalResources, textureName);
    int w, h;

    if(tex==NULL)
    {
        printf("\n---WATCHOUT : ERROR CREATING TILESET, SPECIFIED RESOURCE '%s' DOESNT EXIST IN THE HASHMAP, THE TILESET WILL HAVE AN INCORRECT CELL COUNT",textureName);
    }
    else
    {
        SDL_QueryTexture(tex, NULL,NULL, &w, &h);
        tileset->tileCount=(w/cellSize_x)*(h/cellSize_y);
    }
    strcpy(tileset->textureName,textureName);
    tileset->cellSize_x=cellSize_x;
    tileset->cellSize_y=cellSize_y;
    //
    tileset->tilesCollisions=malloc(sizeof(TILE_COLLISION)*tileset->tileCount);
    for (int i = 0; i<tileset->tileCount; i++)
    {
        *(tileset->tilesCollisions + i) = TILECOL_NONE;
    }
    return tileset;
}

void Tileset_Save(Tileset *tileset, const char path[])
{
    FILE *fp;

    if((fp=fopen(path, "wb"))==NULL) {
        printf("Tileset save: cannot open file '%s'.\n", path);
        return;
    }

    fwrite(tileset->textureName, sizeof(char), HASHPAIR_STR_KEYLENGHT, fp);
    
    fwrite(&tileset->cellSize_x, sizeof(int), 1, fp);
    fwrite(&tileset->cellSize_y, sizeof(int), 1, fp);
    fwrite(&tileset->tileCount, sizeof(int), 1, fp);
    fwrite(tileset->tilesCollisions, sizeof(TILE_COLLISION), tileset->tileCount, fp);
    
    fclose(fp);
}

Tileset *Tileset_Load(const char path[])
{
    FILE *fp;
    if((fp=fopen(path, "rb"))==NULL) {
        printf("Tileset load: Cannot open file %s.\n",path);
        return NULL;
    }

    Tileset *tileset = malloc(sizeof(Tileset));

    fread(&tileset->textureName, sizeof(char), HASHPAIR_STR_KEYLENGHT, fp);
    fread(&tileset->cellSize_x, sizeof(int), 1, fp);
    fread(&tileset->cellSize_y, sizeof(int), 1, fp);
    fread(&tileset->tileCount, sizeof(int), 1, fp);


    tileset->tilesCollisions = malloc(sizeof(TILE_COLLISION)*tileset->tileCount);
    fread(tileset->tilesCollisions, sizeof(TILE_COLLISION), tileset->tileCount, fp);

    //printf("X=%d; Y=%d",level->sizeX, level->sizeY);
    fclose(fp);
    return tileset;
}

void Tileset_Destroy(Tileset *tileset)
{
    if(tileset!=NULL)
    {
        free(tileset->tilesCollisions);
        free(tileset);
    }
}

char TILE_COLLISION_GetRepresentationChar(TILE_COLLISION col)
{
    switch(col)
    {
        case TILECOL_NONE:
            return ' ';
        case TILECOL_WALL:
            return 'W';
    }
    return '?';
}