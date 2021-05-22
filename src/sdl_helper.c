#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include "resources.h"
#include "kmath.h"

int SDLH_Init(SDL_Window **window, SDL_Renderer **renderer, int w, int h)
{
	//Initialiser le SDL
	if(SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		fprintf(stderr, "Erreur SDL_Init : %s", SDL_GetError());
		return -1;
	}
	//Créer la fenetre et son renderer
    *window = SDL_CreateWindow("Super Cancer Mario", SDL_WINDOWPOS_UNDEFINED,
                    SDL_WINDOWPOS_UNDEFINED, w,h,SDL_WINDOW_SHOWN);
	if(*window==NULL)
	{
		fprintf(stderr, "Erreur SDL_CreateWindow : %s", SDL_GetError());
		return -1;
	}
    *renderer = SDL_CreateRenderer(*window, 0, SDL_RENDERER_ACCELERATED);
    if(*renderer==NULL)
	{
		fprintf(stderr, "Erreur SDL_CreateRenderer : %s", SDL_GetError());
		return -1;
	}
	return 0;	//Si tout s'est bien passé, renvoyer 0
}

SDL_Texture *SDLH_LoadPNG(SDL_Renderer *renderer, const char path[])
{
	SDL_Surface *surf = NULL;
    SDL_Texture *texture = NULL;
    surf = IMG_Load(path);
    if(surf == NULL)
    {
        fprintf(stderr, "Erreur IMG_Load : %s", SDL_GetError());
        return NULL;
    }
    texture = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);	//Libérer la Surface puisque seul la Texture nous interesse
    if(texture == NULL)
    {
        fprintf(stderr, "Erreur SDL_CreateTextureFromSurface : %s", SDL_GetError());
        return NULL;
    }
    return texture;
}

void SDLH_DrawRectFill(SDL_Renderer *renderer,SDL_Rect rect, SDL_Color color)
{
    SDL_SetRenderDrawColor(renderer, color.r,color.g,color.b,color.a);
    SDL_RenderFillRect(renderer,&rect);
}


int SDLH_TextEntry(SDL_Renderer *renderer, char inputText[], const int strSize
                    , SDL_Rect drawRect, SpriteSheet *font
                    , SDL_Color textColor, SDL_Color bgColor
                    , SDL_Keycode confirmKey, SDL_Keycode cancelKey)
{
    //Event handler
    SDL_Event e;
    //Enable text input
    SDL_StartTextInput();
    int inputEnd = 0;
    int needRender = 1;
    //While application is running
    while( !inputEnd )
    {
        //Handle events on queue
        while( SDL_PollEvent( &e ) != 0 )
        {
            //Special key input
            if( e.type == SDL_KEYDOWN )
            {
                //Handle backspace
                if( e.key.keysym.sym == SDLK_BACKSPACE && strlen(inputText) > 0 )
                {
                    int len = strlen(inputText);
                    inputText[len-1]='\0';//delete lastchar
                    needRender = 1;
                }
                //Handle copy
                else if( e.key.keysym.sym == SDLK_c && SDL_GetModState() & KMOD_CTRL )
                {
                    SDL_SetClipboardText( inputText);
                }
                //Handle paste
                else if( e.key.keysym.sym == SDLK_v && SDL_GetModState() & KMOD_CTRL )
                {
                    strncpy(inputText, SDL_GetClipboardText(), 256);
                }
            }
            else if(e.type == SDL_KEYUP)
            {
                if(e.key.keysym.sym == confirmKey)
                {
                    inputEnd = 1;
                }
                else if(e.key.keysym.sym == cancelKey)
                {
                    return 0;
                }
            }
            //Special text input event
            else if( e.type == SDL_TEXTINPUT )
            {
                //Not copy or pasting
                if( !( ( e.text.text[ 0 ] == 'c' || e.text.text[ 0 ] == 'C' ) && ( e.text.text[ 0 ] == 'v' || e.text.text[ 0 ] == 'V' ) && SDL_GetModState() & KMOD_CTRL ) )
                {
                    //Append character
                    strncat(inputText,e.text.text,strSize-strlen(inputText));
                    needRender=1;
                }
            }
        }
        if(needRender)
        {
            //Set font color
            SDL_SetTextureColorMod(font->texture, textColor.r, textColor.g, textColor.b);
            //Set bg color and draw bg 
            SDL_SetRenderDrawColor(renderer,bgColor.r, bgColor.g, bgColor.b, bgColor.a);
            SDL_RenderFillRect(renderer,&drawRect);
            //Display text
            int maxVisibleChars=(int)(drawRect.w/(font->cellSize_x*((float)drawRect.h/(float)font->cellSize_y)));
            int offset = KMath_MaxInt(0,strlen(inputText)-maxVisibleChars);
            char *tempTextDisplay = malloc(maxVisibleChars*sizeof(char));
            strcpy(tempTextDisplay, inputText+offset);
            SpriteSheet_ASCII_Write(renderer, font, tempTextDisplay, drawRect.h, (SDL_Point){drawRect.x,drawRect.y});
            free(tempTextDisplay);

            SDL_RenderPresent(renderer);
            needRender=0;
        }

    }

    SDL_SetTextureColorMod(font->texture, 255,255,255); 
    //Disable text input
    SDL_StopTextInput();
    return 1;
}

int SDLH_SizeSelect2D(SDL_Renderer *renderer,SDL_Point *size, SDL_Point minSize
                            , SDL_Point maxSize, char text[]
                            , SDL_Rect drawRect, SpriteSheet *font, SDL_Color textColor
                            , SDL_Color bgColor, SDL_Keycode confirmKey, SDL_Keycode cancelKey
                            , SDL_Keycode plusXKey, SDL_Keycode minusXKey, SDL_Keycode plusYKey
                            , SDL_Keycode minusYKey )
{
    SDL_Point sizeAdd = {1,1};  //Size to add to existing size
    size->x-=1;
    size->y-=1;
    int sizeChoosen = 0;
    while(!sizeChoosen)
    {
        SDL_Event e;
        while( SDL_PollEvent( &e ) != 0 )
        {
            if(e.type == SDL_QUIT)
            {
                return 0;
            }
            else if(e.type == SDL_KEYDOWN)
            {
                if(e.key.keysym.sym == plusYKey)
                {
                    sizeAdd.y+=1;
                }
                else if(e.key.keysym.sym == minusYKey)
                {
                    sizeAdd.y-=1;
                }
                else if(e.key.keysym.sym == plusXKey)
                {
                    sizeAdd.x+=1;
                }
                else if(e.key.keysym.sym == minusXKey)
                {
                    sizeAdd.x-=1;
                }
            }
            else if( e.type == SDL_KEYUP )
            {
                if(e.key.keysym.sym==cancelKey)
                {
                    return 0;
                }
                else if(e.key.keysym.sym==confirmKey)
                {
                    sizeChoosen = 1;
                }
            }
        }
        size->x=KMath_ClampInt(size->x + sizeAdd.x, minSize.x, maxSize.x);
        size->y=KMath_ClampInt(size->y + sizeAdd.y, minSize.y, maxSize.y);
        if(sizeAdd.x!=0 || sizeAdd.y!=0)    //If we need a redraw
        {
            SDLH_DrawRectFill(renderer,
                drawRect,
                (SDL_Color){20,20,20,255});
            char levelSizeString[100];
            sprintf(levelSizeString, "%s:\n%dx%d", text, size->x, size->y);
            SpriteSheet_ASCII_Write(renderer,font,levelSizeString, 25,
                        (SDL_Point){drawRect.x,drawRect.y});
            SDL_RenderPresent(renderer);
        }
        sizeAdd = (SDL_Point) {0,0};
    }
    return 1;
}

void SDLH_ClearRenderer(SDL_Renderer *renderer, SDL_Color color)
{
    int h, w;
    SDL_GetRendererOutputSize(renderer, &w, &h);
    SDLH_DrawRectFill(renderer, (SDL_Rect){0,0,w,h}, color);
}
