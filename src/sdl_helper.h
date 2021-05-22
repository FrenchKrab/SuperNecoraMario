#ifndef _SDL_HELPER_H
#define _SDL_HELPER_H
#include "resources.h"

int SDLH_Init(SDL_Window **window, SDL_Renderer **renderer, int w, int h);
SDL_Texture *SDLH_LoadPNG(SDL_Renderer *renderer, const char path[]);

//Create a text entry rect. Returns 1 if user confirmed, 0 if user cancelled
int SDLH_TextEntry(SDL_Renderer *renderer, char str[], int strSize
                    , SDL_Rect drawRect, SpriteSheet *font
                    ,SDL_Color textColor, SDL_Color bgColor
                    , SDL_Keycode confirmKey, SDL_Keycode cancelKey);

//Create a dialog and ask to adjust a size with x and y, 1 if ok, 0 if cancelled
int SDLH_SizeSelect2D(SDL_Renderer *renderer,SDL_Point *size, SDL_Point minSize
                            , SDL_Point maxSize, char text[]
                            , SDL_Rect drawRect, SpriteSheet *font, SDL_Color textColor
                            , SDL_Color bgColor, SDL_Keycode confirmKey, SDL_Keycode cancelKey
                            , SDL_Keycode plusXKey, SDL_Keycode minusXKey, SDL_Keycode plusYKey
                            , SDL_Keycode minusYKey );

void SDLH_DrawRectFill(SDL_Renderer *renderer,SDL_Rect rect, SDL_Color color);

void SDLH_ClearRenderer(SDL_Renderer *renderer, SDL_Color color);

#endif
