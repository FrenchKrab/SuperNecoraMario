#ifndef LEVEL_H
#define LEVEL_H

#include "resources.h"


typedef struct GameEntity_SpawnData GameEntity_SpawnData;

/*Level files structure:
-int sizeX
-int sizeY
-int entityCount;
-char[HASHPAIR_STR_KEYLENGHT] tileset_path
-int[sizex*sizey] cells
-GameEntity_SpawnData[entityCount] entityspawns
*/

typedef enum 
{
    TILECOL_NONE=0,
    TILECOL_WALL=1
} TILE_COLLISION;


typedef struct {
    char textureName[HASHPAIR_STR_KEYLENGHT];
    int cellSize_x;
    int cellSize_y;
    int tileCount;
    TILE_COLLISION *tilesCollisions;
} Tileset;


typedef struct {
    int sizeX;
    int sizeY;
    int entityCount;
    char tileset[HASHPAIR_STR_KEYLENGHT];    //Index du tileset
    int *cells; //Tableau contenant les types des cellules
    GameEntity_SpawnData *entities_spawns;
} Level;


/*Fonctions pour Level*/
Level *Level_Create(int width, int height, const char tileset[]);
void Level_Destroy(Level*);
void Level_Save(Level *level, const char path[]);
Level *Level_Load(const char path[]);

//Renvoie l'adresse de la cellule en (x,y)
int *Level_Get_Cell_Adress(Level *lvl, int x, int y);

//Renvoie le type de cellule en (x,y)
int Level_Get_Cell(Level *lvl, int x, int y);

//Renvoie le type de collision la cellule placée en (x,y) dans le niveau
int Level_Get_Cell_Type(Level *lvl, Tileset *tileset, int x, int y);

//Définie la nouvelle valeur de la cellule en (x,y) du niveau
int Level_Set_Cell(Level *lvl, int x, int y, int newType);

/*Fonctions pour tileset*/
Tileset *Tileset_Create(char path[], int cellSize_x, int cellSize_y);
void Tileset_Save(Tileset *tileset, const char path[]);
Tileset *Tileset_Load(const char path[]);
void Tileset_Destroy(Tileset *tileset);

char TILE_COLLISION_GetRepresentationChar(TILE_COLLISION);

#endif