#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <string.h>
#include "sdl_helper.h"
#include "level.h"
#include "entities.h"
#include "resources.h"
#include "kds.h"
#include "unistd.h"

#define CELL_SIZE 16

SDL_Window *mainWindow;
SDL_Renderer *renderer;
SpriteSheet *font;



Level *level = NULL;
char levelPath[HASHPAIR_STR_KEYLENGHT] = "levels/a.lvl";
List *entities = NULL;
Tileset *level_tileset = NULL;
SpriteSheet *tilesetSheet = NULL;


int editMode = 0;   //0=level; 1=entities

int selectedTile = 0;
SDL_Point selectedCell = {0,0};

SDL_Point entitiesCursor = {0,0};
int entitiesCursorAction = 0;
int entityPlacingType = 1;
ENTITY_TYPE placeableEntities[ENTITY_TYPE_COUNT] = {ENTITY_PLAYER,ENTITY_BONUS, ENTITY_GOOMBA, ENTITY_BLOCK};
char placeableEntitiesNames[ENTITY_TYPE_COUNT][32] = {"Mario", "Bonus", "Goomba", "Block"};
GameEntity_SpawnData *selectedEntity;
int selectedEntityArg = 0;

SDL_Point cameraPos = {0,0};
float zoom = 1;
SDL_Point tilesetCellcount;

char lastMessage[256]="Welcome to Kraby's level editor !";

void InitLevelEditor()
{
    SetupEntityFunctions();

    SDL_Texture *font1 = Hashmap_str_get(graphicalResources, "fonttable.png");
	font = SpriteSheet_Create(font1,32,64);
}

void UnloadLevel()
{
    printf("\nFREE1");
    Level_Destroy(level);
    printf("\nFREE2");
    List_free(entities, free);
}

void ReloadTileset()
{
    printf(";;Gonna load tilset %s", level->tileset);
    Tileset *tempTileset =Tileset_Load(level->tileset);
    if(tempTileset != NULL)
    {
        SDL_Texture *tilesetTex = Hashmap_str_get(graphicalResources, tempTileset->textureName);
        printf("\nTexture\n");
        if(tilesetTex!=NULL)
        {
            Tileset_Destroy(level_tileset);
            SpriteSheet_Destroy(tilesetSheet);
            level_tileset = tempTileset;
            tilesetSheet=SpriteSheet_Create(tilesetTex, level_tileset->cellSize_x, level_tileset->cellSize_y);
            tilesetCellcount.x = (tilesetSheet->width/tilesetSheet->cellSize_x);
            tilesetCellcount.y = (tilesetSheet->height/tilesetSheet->cellSize_y);
        }
    }
    printf("\nTOO BAD\n");
}

void CreateLevel()
{
    int w,h;
    SDL_GL_GetDrawableSize(mainWindow, &w, &h);
    SDL_Point alertBoxSize = {300,200};
    SDL_Rect alertBoxRect = {w/2-alertBoxSize.x/2, h/2-alertBoxSize.y/2, alertBoxSize.x, alertBoxSize.y};
    SDLH_DrawRectFill(renderer,
                    alertBoxRect,
                    (SDL_Color){30,30,30,255});
    SpriteSheet_ASCII_Write(renderer,font,"--CREATE LEVEL--\nEnter the level's name", 25,
                            (SDL_Point){alertBoxRect.x+5,alertBoxRect.y+5});
    char fileName[HASHPAIR_STR_KEYLENGHT] = "";

    //Make user enter the level name, and if he confirmed, create it
    if(SDLH_TextEntry(renderer, fileName, HASHPAIR_STR_KEYLENGHT,
                    (SDL_Rect){alertBoxRect.x+10, alertBoxRect.y+60, alertBoxSize.x-20,30},font,
                    (SDL_Color){255,255,255,255},(SDL_Color){15,15,15,255},
                    SDLK_RETURN, SDLK_ESCAPE))
    {
        SDL_Delay(50);
        SDL_Point levelSize = {39,14};
        if(SDLH_SizeSelect2D(renderer, &levelSize, (SDL_Point){0,0}
                                , (SDL_Point){9999,9999},"Level size" 
                                ,(SDL_Rect){alertBoxRect.x, alertBoxRect.y+80, alertBoxRect.w, alertBoxRect.h-80}
                                , font, (SDL_Color){255,255,255,255}, (SDL_Color){20,20,20,255}
                                , SDLK_RETURN, SDLK_ESCAPE, SDLK_RIGHT, SDLK_LEFT, SDLK_UP,
                                SDLK_DOWN))
        {
            printf("size %d, %d", levelSize.x, levelSize.y);
            
            //Now that we've gathered the main data needed to create a level, create it
            UnloadLevel();
            level = Level_Create(levelSize.x, levelSize.y, "DEFAULTILESETDEBUGTEST");
            entities = NULL;

            //Set the path
            String_AddPrefix(fileName, levelFolder);
            strcpy(levelPath, fileName);
        }
    }
}

void EditLevelProperties()
{
    int w,h;
    SDL_GL_GetDrawableSize(mainWindow, &w, &h);
    SDL_Point alertBoxSize = {300,200};
    SDL_Rect alertBoxRect = {w/2-alertBoxSize.x/2, h/2-alertBoxSize.y/2, alertBoxSize.x, alertBoxSize.y};
    
    int stopEditing = 0;
    
    while(!stopEditing)
    {
        int action = 0;

        SDL_Event e;
        while( SDL_PollEvent( &e ) != 0 )
        {
            if(e.type == SDL_QUIT)
            {
                stopEditing = 1;
            }
            //Special key input
            else if( e.type == SDL_KEYDOWN )
            {
                if(e.key.keysym.sym==SDLK_F1)
                {
                    action = 1;
                }
                else if(e.key.keysym.sym==SDLK_ESCAPE)
                {
                    stopEditing = 1;
                }
            }
        }
        
        SDLH_DrawRectFill(renderer,
                alertBoxRect,
                (SDL_Color){30,30,30,255});
        if(action==0)
        {
            SpriteSheet_ASCII_Write(renderer,font,"--LEVEL PROPERTIES--\nF1=Tileset\nESC=Back", 25,
                                    (SDL_Point){alertBoxRect.x+5,alertBoxRect.y+5});
        }
        else if(action==1)
        {
            char fileName[HASHPAIR_STR_KEYLENGHT];
            strcpy(fileName, level->tileset);

            //Make user enter the level name, and if he confirmed, create it
            if(SDLH_TextEntry(renderer, fileName, HASHPAIR_STR_KEYLENGHT,
                            (SDL_Rect){alertBoxRect.x+10, alertBoxRect.y+60, alertBoxSize.x-20,30},font,
                            (SDL_Color){255,255,255,255},(SDL_Color){15,15,15,255},
                            SDLK_RETURN, SDLK_ESCAPE))
            {
                strcpy(level->tileset, fileName);
                printf("\nnew tileset : %s", level->tileset);
                ReloadTileset();
            }
        }
        SDL_RenderPresent(renderer);
    }

}

void LoadLevel()
{
    int w,h;
    SDL_GL_GetDrawableSize(mainWindow, &w, &h);
    SDL_Point alertBoxSize = {300,200};
    SDL_Rect alertBoxRect = {w/2-alertBoxSize.x/2, h/2-alertBoxSize.y/2, alertBoxSize.x, alertBoxSize.y};
    SDLH_DrawRectFill(renderer,
                    alertBoxRect,
                    (SDL_Color){30,30,30,255});
    SpriteSheet_ASCII_Write(renderer,font,"--LOAD LEVEL--\nEnter the level's name", 25,
                            (SDL_Point){alertBoxRect.x+5,alertBoxRect.y+5});
    char fileName[HASHPAIR_STR_KEYLENGHT] = "";

    //Make user enter the level name, and if he confirmed, create it
    if(SDLH_TextEntry(renderer, fileName, HASHPAIR_STR_KEYLENGHT,
                    (SDL_Rect){alertBoxRect.x+10, alertBoxRect.y+60, alertBoxSize.x-20,30},font,
                    (SDL_Color){255,255,255,255},(SDL_Color){15,15,15,255},
                    SDLK_RETURN, SDLK_ESCAPE))
    {
        String_AddPrefix(fileName, levelFolder);
        if( access( fileName, F_OK ) != -1 )
        {
            printf("\nAAAA");
            printf("\n???");
            Level *newLevel = Level_Load(fileName);
            if(newLevel!=NULL)
            {
                // file exists, load it
                UnloadLevel();
                strcpy(levelPath, fileName);
                level = newLevel;
                printf("ENTITY COUNT: %d", level->entityCount);
                printf("\nf");
                entities = List_fromArray(level->entities_spawns, level->entityCount, sizeof(GameEntity_SpawnData));
                ReloadTileset();
            }

        }
    } 
}

void SaveLevel()
{
    level->entityCount=List_size(entities);
    if(level->entities_spawns!=NULL)
        free(level->entities_spawns);
    level->entities_spawns=List_toArray(entities, sizeof(GameEntity_SpawnData));
    printf("\n[0]: %d, %d",level->entities_spawns[0].position.x, level->entities_spawns[0].position.y);
    printf("\n[1]: %d, %d",level->entities_spawns[1].position.x, level->entities_spawns[1].position.y);

    Level_Save(level, levelPath);
}


Tileset *TilesetEditor_Create(char path[])
{
    int w,h;
    SDL_GL_GetDrawableSize(mainWindow, &w, &h);
    SDL_Point alertBoxSize = {w,200};
    SDL_Rect alertBoxRect = {0, h/2-alertBoxSize.y/2, alertBoxSize.x, alertBoxSize.y};
    SDLH_DrawRectFill(renderer,
                    alertBoxRect,
                    (SDL_Color){30,30,30,255});
    SpriteSheet_ASCII_Write(renderer,font,"--CREATE TILESET--\nTileset file name :", 25,
                            (SDL_Point){alertBoxRect.x+5,alertBoxRect.y+1});
    
    char fileName[HASHPAIR_STR_KEYLENGHT] = "";
    char imageName[HASHPAIR_STR_KEYLENGHT] = "";
    SDL_Point cellSize = {15,16};

    //Make user enter the level name, and if he confirmed, create it
    if(SDLH_TextEntry(renderer, fileName, HASHPAIR_STR_KEYLENGHT,
                    (SDL_Rect){alertBoxRect.x+10, alertBoxRect.y+60, alertBoxSize.x-20,30},font,
                    (SDL_Color){255,255,255,255},(SDL_Color){15,15,15,255},
                    SDLK_RETURN, SDLK_ESCAPE))
    {
        
        SDLH_DrawRectFill(renderer,
                alertBoxRect,
                (SDL_Color){30,30,30,255});
        SpriteSheet_ASCII_Write(renderer,font,"--CREATE TILESET--\nTileset's image name:", 25,
                                (SDL_Point){alertBoxRect.x+5,alertBoxRect.y+1});

        if(SDLH_TextEntry(renderer, imageName, HASHPAIR_STR_KEYLENGHT,
                    (SDL_Rect){alertBoxRect.x+10, alertBoxRect.y+60, alertBoxSize.x-20,30},font,
                    (SDL_Color){255,255,255,255},(SDL_Color){15,15,15,255},
                    SDLK_RETURN, SDLK_ESCAPE))
        {
            if(SDLH_SizeSelect2D(renderer, &cellSize, (SDL_Point){0,0}
                            , (SDL_Point){9999,9999},"Tile size" 
                            ,(SDL_Rect){alertBoxRect.x, alertBoxRect.y+80, alertBoxRect.w, alertBoxRect.h-80}
                            , font, (SDL_Color){255,255,255,255}, (SDL_Color){20,20,20,255}
                            , SDLK_RETURN, SDLK_ESCAPE, SDLK_RIGHT, SDLK_LEFT, SDLK_UP,
                            SDLK_DOWN))
            {
                Tileset *tileset = Tileset_Create(imageName,cellSize.x,cellSize.y);

                //Copy the tileset's file name for output
                String_AddPrefix(fileName, tilesetFolder);
                strcpy(path,fileName);
   
                //Copy the image's name into the tileset
                strcpy(tileset->textureName, imageName);
                return tileset;
            }
        }
    }
    return NULL;
}

Tileset *TilesetEditor_Load(char path[])
{
    int w,h;
    SDL_GL_GetDrawableSize(mainWindow, &w, &h);
    SDL_Point alertBoxSize = {w,200};
    SDL_Rect alertBoxRect = {0, h/2-alertBoxSize.y/2, alertBoxSize.x, alertBoxSize.y};
    SDLH_DrawRectFill(renderer,
                    alertBoxRect,
                    (SDL_Color){30,30,30,255});
    SpriteSheet_ASCII_Write(renderer,font,"--LOAD TILESET--\nTileset file name :", 25,
                            (SDL_Point){alertBoxRect.x+5,alertBoxRect.y+1});
    
    char fileName[HASHPAIR_STR_KEYLENGHT] = "";

    //Make user enter the level name, and if he confirmed, create it
    if(SDLH_TextEntry(renderer, fileName, HASHPAIR_STR_KEYLENGHT,
                    (SDL_Rect){alertBoxRect.x+10, alertBoxRect.y+60, alertBoxSize.x-20,30},font,
                    (SDL_Color){255,255,255,255},(SDL_Color){15,15,15,255},
                    SDLK_RETURN, SDLK_ESCAPE))
    {
        String_AddPrefix(fileName, tilesetFolder);
        strcpy(path, fileName);
        printf("\ngonna load tileset");
        printf("\ngonna load tileset");
        Tileset *ts = Tileset_Load(fileName);
        printf("\nloaded %s %d %d | %d %d %d", ts->textureName, ts->tileCount, ts->cellSize_x,
                *(ts->tilesCollisions),*(ts->tilesCollisions+1),*(ts->tilesCollisions+2));
        printf("\ngonna load tileset");
        return ts;
    }
    return NULL;
}

void TilesetEditor()
{
    int w,h;
    SDL_GL_GetDrawableSize(mainWindow, &w, &h);

    SDL_Point te_selectedTile={0,0};
    float te_zoom = 1;
    SDL_Point te_camera = {20,30};
    char te_tilesetPath[HASHPAIR_STR_KEYLENGHT] = "tilesets/tileset.ts";
    Tileset *te_tileset = NULL;
    SDL_Texture *te_tilesetTexture = NULL;
    SpriteSheet *te_tilesetSpritesheet = NULL;
    SDL_Point te_cellCount = {1,1}; //Combien il y a de tiles en x et en y sur le tileset
    int quit = 0;


    while(!quit)
    {
        //Get inputs
        SDL_Event e;
        while( SDL_PollEvent( &e ) != 0 )
        {
            if(e.type == SDL_QUIT)
            {
                quit=1;
            }
            else if( e.type == SDL_KEYDOWN )
            {
                if(e.key.keysym.sym==SDLK_ESCAPE)
                {
                    quit = 1;
                }
                else if(e.key.keysym.sym==SDLK_F1)
                {
                    char newTilesetPath[HASHPAIR_STR_KEYLENGHT];
                    Tileset *newTileset = TilesetEditor_Create(newTilesetPath);
                    if(newTileset!=NULL)
                    {
                        /*if(te_tileset!=NULL)
                        {
                            Tileset_Destroy(te_tileset);
                        }*/
                        selectedCell=(SDL_Point){0,0};
                        te_tileset=newTileset;
                        strcpy(te_tilesetPath, newTilesetPath);
                        te_tilesetTexture = Hashmap_str_get(graphicalResources, te_tileset->textureName);
                        if(te_tilesetTexture != NULL)
                        {
                            te_tilesetSpritesheet = SpriteSheet_Create(te_tilesetTexture,
                                                                        te_tileset->cellSize_x,
                                                                        te_tileset->cellSize_y);
                            te_cellCount.x = (te_tilesetSpritesheet->width/te_tilesetSpritesheet->cellSize_x);
                            te_cellCount.y = (te_tilesetSpritesheet->height/te_tilesetSpritesheet->cellSize_y);
                            te_tileset->tileCount=te_cellCount.x*te_cellCount.y;
                        }
                    }
                }
                else if(e.key.keysym.sym==SDLK_F2)
                {
                    char newTilesetPath[HASHPAIR_STR_KEYLENGHT];
                    Tileset *newTileset = TilesetEditor_Load(newTilesetPath);
                    if(newTileset!=NULL)
                    {
                        selectedCell=(SDL_Point){0,0};
                        te_tileset=newTileset;
                        strcpy(te_tilesetPath, newTilesetPath);
                        te_tilesetTexture = Hashmap_str_get(graphicalResources, te_tileset->textureName);
                        if(te_tilesetTexture != NULL)
                        {
                            te_tilesetSpritesheet = SpriteSheet_Create(te_tilesetTexture,
                                                                        te_tileset->cellSize_x,
                                                                        te_tileset->cellSize_y);
                            printf("\nOK !");
                            te_cellCount.x = (te_tilesetSpritesheet->width/te_tilesetSpritesheet->cellSize_x);
                            te_cellCount.y = (te_tilesetSpritesheet->height/te_tilesetSpritesheet->cellSize_y);
                            te_tileset->tileCount=te_cellCount.x*te_cellCount.y;
                            printf("\nOK 2!");
                            printf("\nOK !33");
                        }
                    }
                }
                else if(e.key.keysym.sym==SDLK_F4)
                {
                    printf("Saving :img=%s, x=%d, y=%d, c=%d, [0]=%d, [1]=%d, [2]=%d"
                    , te_tileset->textureName
                    , te_tileset->cellSize_x, te_tileset->cellSize_y, te_tileset->tileCount,
                    te_tileset->tilesCollisions[0], te_tileset->tilesCollisions[1],
                    te_tileset->tilesCollisions[2]);
                    Tileset_Save(te_tileset, te_tilesetPath);
                }
                else if(e.key.keysym.sym==SDLK_d)
                {
                    te_selectedTile.x = KMath_ClampInt(te_selectedTile.x+1,0,te_cellCount.x-1);
                }
                else if(e.key.keysym.sym==SDLK_q)
                {
                    te_selectedTile.x = KMath_ClampInt(te_selectedTile.x-1,0,te_cellCount.x-1);
                }
                else if(e.key.keysym.sym==SDLK_s)
                {
                    te_selectedTile.y = KMath_ClampInt(te_selectedTile.y+1,0,te_cellCount.y-1);
                }
                else if(e.key.keysym.sym==SDLK_z)
                {
                    te_selectedTile.y = KMath_ClampInt(te_selectedTile.y-1,0,te_cellCount.y-1);
                }
                else if(e.key.keysym.sym==SDLK_m)
                {
                    te_camera.x-=10;
                }
                else if(e.key.keysym.sym==SDLK_k)
                {
                    te_camera.x+=10;
                }
                else if(e.key.keysym.sym==SDLK_o)
                {
                    te_camera.y+=10;
                }
                else if(e.key.keysym.sym==SDLK_l)
                {
                    te_camera.y-=10;
                }
                else if(e.key.keysym.sym==SDLK_w)
                {
                    te_zoom-=0.2;
                    if(te_zoom<0.2)
                        te_zoom=0.2;
                }
                else if(e.key.keysym.sym==SDLK_x)
                {
                    te_zoom+=0.2;
                }
                
                if(te_tileset!=NULL)
                {
                    int te_selectedTileIndex = te_selectedTile.x + te_selectedTile.y*te_cellCount.x;
                    if(te_selectedTileIndex<te_tileset->tileCount)
                    {
                        int newValue = -1;

                        switch(e.key.keysym.sym)
                        {
                            case SDLK_1:
                                newValue=TILECOL_NONE;
                                break;
                            case SDLK_2:
                                newValue=TILECOL_WALL;
                                break;
                        }

                        if(newValue!=-1)
                            *(te_tileset->tilesCollisions+te_selectedTileIndex)=newValue;
                    }
                }
            }
        }

        //draw
        SDLH_DrawRectFill(renderer, (SDL_Rect){0,0,w,h}, (SDL_Color){40,40,40,255});


        if(te_tilesetSpritesheet != NULL)
        {
            for(int i = 0; i<te_tileset->tileCount; i++)
            {
                SDL_Rect destination = {(i%te_cellCount.x)*(1+te_tileset->cellSize_x+(te_tileset->cellSize_x/2)),
                                        (i/te_cellCount.x)*te_tileset->cellSize_y,
                                        te_tileset->cellSize_x, te_tileset->cellSize_y};
                KMath_Rect_Offset(&destination, te_camera);
                KMath_Rect_Multiply(&destination, te_zoom);
                SpriteSheet_RenderCopy_FromIndex(renderer, te_tilesetSpritesheet, i, destination, 0);

                char represChar[2] = "_\0";
                represChar[0]=TILE_COLLISION_GetRepresentationChar(*(te_tileset->tilesCollisions+i));
                
                SDL_Rect dest2 = destination;
                dest2.w=dest2.w/2;
                dest2.x=destination.x-dest2.w;
                SDLH_DrawRectFill(renderer, dest2, (SDL_Color){0,0,0,30});
                SpriteSheet_ASCII_Write(renderer, font, represChar, dest2.h, (SDL_Point){dest2.x,dest2.y});
            }
            SDL_Rect cursorRect = {(1+te_tileset->cellSize_x+(te_tileset->cellSize_x/2))*te_selectedTile.x,
                                    te_tileset->cellSize_y*te_selectedTile.y,
                                    te_tileset->cellSize_x, te_tileset->cellSize_y};
            KMath_Rect_Offset(&cursorRect, te_camera);
            KMath_Rect_Multiply(&cursorRect, te_zoom);
            SDL_SetRenderDrawColor(renderer,255,255,0,255);
            SDL_RenderDrawRect(renderer, &cursorRect);
        }   

        SDLH_DrawRectFill(renderer, (SDL_Rect){0,0,w,20}, (SDL_Color){30,30,30,255});
        SpriteSheet_ASCII_Write(renderer, font, "Tileset Editor", 20, (SDL_Point){130,0});


        SDL_RenderPresent(renderer);
    }
}

int main()
{
    mainWindow=NULL;
    renderer=NULL;
	if(SDLH_Init(&mainWindow, &renderer, 700,400)!=0)
	{
		return -1;
	}
	SetupResources(renderer);
    InitLevelEditor();


    level = Level_Create(1, 1, "tilesets/ts1");
    entities = NULL;

    int w,h;
    SDL_GL_GetDrawableSize(mainWindow, &w, &h);

    int quit = 0;

    while(!quit)
    {
        //Inputs
        SDL_Event e;
        while( SDL_PollEvent( &e ) != 0 )
        {
            if(e.type == SDL_QUIT)
            {
                quit = 1;
            }
            else if( e.type == SDL_KEYDOWN )
            {
                //Special key input
                if(e.key.keysym.sym==SDLK_F1)
                {
                    CreateLevel();
                }
                else if(e.key.keysym.sym==SDLK_F2)
                {
                    LoadLevel();
                }
                else if(e.key.keysym.sym==SDLK_F3 && level!=NULL)
                {
                    EditLevelProperties();
                }
                else if(e.key.keysym.sym==SDLK_F4)
                {
                    SaveLevel();
                }
                else if(e.key.keysym.sym==SDLK_F5)
                {
                    TilesetEditor();
                }
                else if(e.key.keysym.sym==SDLK_1)   //Level mode
                {
                    editMode=0;
                    strcpy(lastMessage,"Level geometry edition mode");
                }
                else if(e.key.keysym.sym==SDLK_2)   //Entity mode
                {
                    editMode=1;
                    strcpy(lastMessage,"Entity edition mode");
                }
                //Camera move
                else if(e.key.keysym.sym==SDLK_UP)
                {
                    cameraPos.y = cameraPos.y+16;
                }
                else if(e.key.keysym.sym==SDLK_DOWN)
                {
                    cameraPos.y = cameraPos.y-16;
                }
                else if(e.key.keysym.sym==SDLK_RIGHT)
                {
                    cameraPos.x = cameraPos.x-16;
                }
                else if(e.key.keysym.sym==SDLK_LEFT)
                {
                    cameraPos.x = cameraPos.x+16;
                }
                //Zoom
                else if(e.key.keysym.sym==SDLK_w)
                {
                    zoom = zoom - 0.2;
                    if(zoom<0.2)
                        zoom = 0.2;
                }
                else if(e.key.keysym.sym==SDLK_x)
                {
                    zoom = zoom + 0.2;
                }

                if(editMode==0 && level!=NULL) //Level edit
                {
                    //Cursor move
                    if(e.key.keysym.sym==SDLK_z)
                    {
                        selectedCell.y = KMath_ClampInt(selectedCell.y-1, 0, level->sizeY-1);
                    }
                    else if(e.key.keysym.sym==SDLK_s)
                    {
                        selectedCell.y = KMath_ClampInt(selectedCell.y+1, 0, level->sizeY-1);
                    }
                    else if(e.key.keysym.sym==SDLK_q)
                    {
                        selectedCell.x = KMath_ClampInt(selectedCell.x-1, 0, level->sizeX-1);
                    }
                    else if(e.key.keysym.sym==SDLK_d)
                    {
                        selectedCell.x = KMath_ClampInt(selectedCell.x+1, 0, level->sizeX-1);
                    }
                    //Place
                    else if(e.key.keysym.sym==SDLK_i)
                    {
                        *(level->cells + selectedCell.x + selectedCell.y*level->sizeX)=selectedTile;
                    }
                    else if(e.key.keysym.sym==SDLK_p)
                    {
                        *(level->cells + selectedCell.x + selectedCell.y*level->sizeX)=-1;
                    }
                }
                else //Entities edit
                {
                    entitiesCursorAction = 0;   //reset wanted action
                    //Cursor move
                    int cursorMoveSpeed=8;
                    if(level_tileset!=NULL)
                        cursorMoveSpeed=level_tileset->cellSize_x/2;
                    
                    if(e.key.keysym.sym==SDLK_LALT)
                    {
                        cursorMoveSpeed=1;
                    }
                    //Move entity cursor
                    if(e.key.keysym.sym==SDLK_z)    
                    {
                        entitiesCursor.y -= cursorMoveSpeed;
                    }
                    else if(e.key.keysym.sym==SDLK_s)
                    {
                        entitiesCursor.y += cursorMoveSpeed;
                    }
                    else if(e.key.keysym.sym==SDLK_q)
                    {
                        entitiesCursor.x -= cursorMoveSpeed;
                    }
                    else if(e.key.keysym.sym==SDLK_d)
                    {
                        entitiesCursor.x += cursorMoveSpeed;
                    }
                    //Entity type selection
                    else if(e.key.keysym.sym==SDLK_b)
                    {
                        entityPlacingType = (entityPlacingType+1)%ENTITY_TYPE_COUNT;
                        sprintf(lastMessage, "Entity placing type : [%s]", placeableEntitiesNames[entityPlacingType]);
                    }
                    else if(e.key.keysym.sym==SDLK_n)
                    {
                        entityPlacingType = entityPlacingType-1;
                        if(entityPlacingType<0)
                            entityPlacingType=ENTITY_TYPE_COUNT-1;
                        sprintf(lastMessage, "Entity placing type : [%s]", placeableEntitiesNames[entityPlacingType]);
                    }
                    //Entity actions
                    else if(e.key.keysym.sym==SDLK_i) //Create entity
                    {
                        GameEntity_SpawnData *data = malloc(sizeof(GameEntity_SpawnData));
                        data->type=placeableEntities[entityPlacingType];
                        data->position = entitiesCursor;
                        for(int i = 0; i<SPAWNDATA_ARGS; i++)
                        {
                            data->args[i]=0;
                        }
                        List_add(&entities, data);sprintf(lastMessage, "Entity placing type : [%s]", placeableEntitiesNames[entityPlacingType]);
                    }
                    else if(e.key.keysym.sym==SDLK_p)   //Delete
                    {
                        entitiesCursorAction=2; //Remove entity
                    }
                    else if(e.key.keysym.sym==SDLK_e)   //Edit
                    {
                        entitiesCursorAction=1; //Edit entity
                    }
                    //Arg edit
                    else if(e.key.keysym.sym==SDLK_o)
                    {
                        selectedEntityArg = KMath_ClampInt(selectedEntityArg-1, 0, SPAWNDATA_ARGS+1);
                    }
                    else if(e.key.keysym.sym==SDLK_l)
                    {
                        selectedEntityArg = KMath_ClampInt(selectedEntityArg+1, 0, SPAWNDATA_ARGS-1);
                    }
                    if(selectedEntity!=NULL)
                    {
                        int offset = 0;
                        if(e.key.keysym.sym==SDLK_m)
                        {
                            offset=1;
                        }
                        else if(e.key.keysym.sym==SDLK_k)
                        {
                            offset=-1;
                        }
                        selectedEntity->args[selectedEntityArg] += offset;
                    }
 
                }


                //Tileset , tile select
                if(level_tileset!=NULL && tilesetSheet!=NULL)
                {

                    if(e.key.keysym.sym==SDLK_k)
                    {
                        selectedTile = KMath_ClampInt(selectedTile-1,0,level_tileset->tileCount);
                    }
                    else if(e.key.keysym.sym==SDLK_m)
                    {
                        selectedTile = KMath_ClampInt(selectedTile+1,0,level_tileset->tileCount);
                    }
                    else if(e.key.keysym.sym==SDLK_o)
                    {
                        selectedTile = KMath_ClampInt(selectedTile-tilesetCellcount.x,0,level_tileset->tileCount);
                    }
                    else if(e.key.keysym.sym==SDLK_l)
                    {
                        selectedTile = KMath_ClampInt(selectedTile+tilesetCellcount.x,0,level_tileset->tileCount);
                    }
                }
            }
        }
        //


        //clear screen
        SDLH_DrawRectFill(renderer, (SDL_Rect){0,0,w,h}, (SDL_Color){255,127,0,255});

    


        if(level!=NULL && level_tileset!=NULL)
        {
            //Display level
            for(int x = 0; x<level->sizeX; x++)
            {
                for(int y = 0; y<level->sizeY; y++)
                {
                    SDL_Rect cellRect={x*CELL_SIZE, y*CELL_SIZE, CELL_SIZE, CELL_SIZE};
                    KMath_Rect_Multiply(&cellRect, zoom);
                    KMath_Rect_Offset(&cellRect, cameraPos);
                    int value = *(level->cells + x + y*level->sizeX);
                    if(value >= 0 && value <=level_tileset->tileCount)
                    {
                        SpriteSheet_RenderCopy_FromIndex(renderer,
                                                        tilesetSheet, value, cellRect, 0);
                    }
                    else
                    {
                        //Empty cell
                        //SDLH_DrawRectFill(renderer,cellRect,(SDL_Color){0, (value*40)%256, 0, 255});
                    }
                    
                }
            }

            //Level outline
            SDL_Rect levelOutlineRect={0,0,level->sizeX*level_tileset->cellSize_x, level->sizeY*level_tileset->cellSize_y}; 
            KMath_Rect_Multiply(&levelOutlineRect, zoom);
            KMath_Rect_Offset(&levelOutlineRect, cameraPos);
            SDL_SetRenderDrawColor(renderer, 255,0,0,255);
            SDL_RenderDrawRect(renderer,&levelOutlineRect);
        }
  

        //Draw entities
        GameEntity_SpawnData *entitiyToRemove = NULL;

        List *entity_browser = entities;
        while(entity_browser!=NULL)
        {
            GameEntity_SpawnData *data = (GameEntity_SpawnData*)entity_browser->value;
            SDL_Rect sourceRect;
            SDL_Texture *tex = GameEntity_SpawnData_GetIcon(data, &sourceRect);
            if(tex!=NULL)
            {
                SDL_Rect dest = {data->position.x, data->position.y, sourceRect.w, sourceRect.h};
                int hovered = SDL_PointInRect(&entitiesCursor, &dest);
                //Make sure to do the scaling/offset after the "physics" check (cursor hover)
                KMath_Rect_Multiply(&dest, zoom);
                KMath_Rect_Offset(&dest, cameraPos);
                if(hovered)
                {
                    SDL_SetRenderDrawColor(renderer, 0, 255, 0,255);
                    SDL_RenderDrawRect(renderer, &dest);
                    if(entitiesCursorAction==1)
                    {
                        if(selectedEntity==NULL)
                        {
                            strcpy(lastMessage,"Selected entity");
                            selectedEntity=data;
                        }
                        else
                        {
                            strcpy(lastMessage,"Deselected entity");
                            selectedEntity=NULL;
                        }
                        entitiesCursorAction=0;
                    }
                    else if(entitiesCursorAction==2)
                    {
                        strcpy(lastMessage,"Removed entity");
                        entitiyToRemove=data;
                        entitiesCursorAction=0;
                    }
                }
                if(selectedEntity==data)
                {
                    data->position=entitiesCursor;
                }


                SDL_RenderCopy(renderer, tex, &sourceRect, &dest);
            }

            entity_browser=entity_browser->next;
        }
        //Remove the an entity if a delete is needed
        if(entitiyToRemove!=NULL)
            List_remove(&entities, entitiyToRemove, free);


        //Entities cursor
        if(editMode==1)
        {
            SDL_Point cursorSize = {10,10};
            SDL_SetRenderDrawColor(renderer,255,0,0,255);
            SDL_RenderDrawLine(renderer, (entitiesCursor.x-cursorSize.x/2)*zoom+cameraPos.x,
                                (entitiesCursor.y)*zoom+cameraPos.y,
                                (entitiesCursor.x+cursorSize.x/2)*zoom+cameraPos.x,
                                (entitiesCursor.y)*zoom+cameraPos.y);
            SDL_RenderDrawLine(renderer, (entitiesCursor.x)*zoom+cameraPos.x,
                                (entitiesCursor.y-cursorSize.y/2)*zoom+cameraPos.y,
                                (entitiesCursor.x)*zoom+cameraPos.x,
                                (entitiesCursor.y+cursorSize.y/2)*zoom+cameraPos.y);
        }

        //Tile cursor
        if(editMode==0)
        {
            //Display cursor
            SDL_Rect cursorRect = {selectedCell.x*CELL_SIZE, selectedCell.y*CELL_SIZE, CELL_SIZE, CELL_SIZE}; 
            //printf("\nz:%f, (%d,%d,%d,%d)",zoom, cursorRect.x, cursorRect.y, cursorRect.w, cursorRect.h);
            KMath_Rect_Multiply(&cursorRect, zoom);
            KMath_Rect_Offset(&cursorRect, cameraPos);
            SDL_SetRenderDrawColor(renderer, 0,255,255,255);
            SDL_RenderDrawRect(renderer, &cursorRect); 
        }


        //draw right panel (tileset)
        if(editMode==0 && level_tileset != NULL && tilesetSheet!=NULL)
        {
            SDL_Rect tilesetPanelRect = {w-300, 0, 300, h};
            SDLH_DrawRectFill(renderer,
                    tilesetPanelRect,
                    (SDL_Color){20,20,20,255});
            char selectedTileText[100];
            sprintf(selectedTileText, "%d", selectedTile);
            SpriteSheet_ASCII_Write(renderer, font, selectedTileText, 20, (SDL_Point){tilesetPanelRect.x,0});
            

            for(int i = 0; i<level_tileset->tileCount; i++)
            {
                SDL_Rect destination = {tilesetPanelRect.x + (i%tilesetCellcount.x)*(level_tileset->cellSize_x),
                                        tilesetPanelRect.y + (i/tilesetCellcount.x)*level_tileset->cellSize_y,
                                        level_tileset->cellSize_x, level_tileset->cellSize_y};
                //KMath_Rect_Offset(&destination, cameraPos);
                //KMath_Rect_Multiply(&destination, zoom);
                SpriteSheet_RenderCopy_FromIndex(renderer, tilesetSheet, i, destination, 0);
                if(selectedTile==i)
                {
                    SDL_SetRenderDrawColor(renderer,255,0,255,255);
                    SDL_RenderDrawRect(renderer, &destination); 
                }
            }
        }

        //Draw entity attributes (entity mode)
        if(editMode==1 && selectedEntity!=NULL)
        {
            char entityPropertiesText[28*SPAWNDATA_ARGS+1] = "";
            for(int i=0; i<SPAWNDATA_ARGS; i++)
            {
                char line[28] = "";
                if(i==selectedEntityArg)
                    strcat(entityPropertiesText, ">");
                snprintf(line, 28,"[%d]: %f\n", i, selectedEntity->args[i]);
                strcat(entityPropertiesText, line);
            }
            SpriteSheet_ASCII_Write(renderer, font, entityPropertiesText, 16, (SDL_Point){w-170, 0});
        }

        //Display info message
        SpriteSheet_ASCII_Write(renderer, font, lastMessage, 16, (SDL_Point){0, h-16});

        //Render
        SDL_RenderPresent(renderer);

    }


    return 0;
}
