#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include "sdl_helper.h"
#include "entities.h"
#include "input.h"
#include "kds.h"
#include "resources.h"
#include "level.h"
#include "kengine.h"

#define LEVEL_COUNT 2

List *game_entities = NULL;

char levels[LEVEL_COUNT][HASHPAIR_STR_KEYLENGHT] = {"1","2"};

int setWindowColor(SDL_Renderer *renderer, SDL_Color color)
{
    if(SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a) < 0)
        return -1;
    if(SDL_RenderClear(renderer) < 0)
        return -1;
    return 0;
}


int main()
{
	SDL_Window *window = NULL;
	SDL_Renderer *renderer = NULL;

	if(SDLH_Init(&window, &renderer, 400,400)!=0)
	{
		return -1;
	}
	SetupEntityFunctions();
	SetupResources(renderer);

	SDL_Texture *font1 = Hashmap_str_get(graphicalResources, "fonttable.png");
	SpriteSheet *font = SpriteSheet_Create(font1,32,64);

	int currentLevel = 0;
	int coins = 0;
	int score = 0;
	int lives = 3;
	int quit = 0;

	while(!quit && lives >=0 && currentLevel < LEVEL_COUNT)
	{

		
		int w, h;
		SDL_GL_GetDrawableSize(window, &w, &h);
		SDLH_DrawRectFill(renderer, (SDL_Rect){0,0,w,h}, (SDL_Color){0,0,0,255});
		char levelText[32] = "";
		sprintf(levelText, "LEVEL %d", currentLevel+1);
		SpriteSheet_ASCII_Write(renderer, font, levelText, 64, (SDL_Point){w/2-3*32, h/2-64});
		SDL_RenderPresent(renderer);
		SDL_Delay(1000);

		World *world=World_Create(renderer, levels[currentLevel], score, lives, coins);
		while(!quit && !world->isDead && !world->finished)
		{
			World_Update(world);
			/*FPS debug*/
			char text[256];
			//snprintf(text, 16, "%lf FPS\n", 1/(world->delta));
			snprintf(text, 256, "M x%02d   %02d coins   Score: %010d\n", lives, world->coins, world->score);
			SDL_Point p = {0,0};
			SpriteSheet_ASCII_Write(renderer, font, text, 16, p);

			SDL_RenderPresent(world->renderer);

			if(Input_IsAnyDown(world->actions->quit))
			{
				quit=1;
				break;
			}
		}
		if(quit)
			break;

		if(world->isDead)
		{
			coins = 0;
			lives--;
		}
		else if(world->finished)
		{
			score=world->score;
			currentLevel++;
		}
		
	}
}

