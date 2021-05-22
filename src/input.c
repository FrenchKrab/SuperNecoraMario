#include <SDL2/SDL.h>
#include <stdio.h>
#include "input.h"

InputActions *InputActions_Create()
{
    InputActions *actions = malloc(sizeof(InputActions));
    actions->jump=INPUT_UP;
    actions->left=INPUT_UP;
    actions->right=INPUT_UP;
    actions->run=INPUT_UP;
    actions->quit=INPUT_UP;
    return actions;
}

InputActions *InputActions_PollFromSDL(InputActions *previous)
{
    InputActions *actions = malloc(sizeof(InputActions)); //Copy old actions
    actions = memcpy(actions, previous, sizeof(InputActions));
    //Input_InitInputActions(actions);

    SDL_Event e;
    while(SDL_PollEvent(&e) != 0)   //Retrieve all events
    {
        if(e.type == SDL_QUIT)
        {
            actions->quit = INPUT_DOWN;
        }
        else if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP)
        {
            int new_state = INPUT_DOWN;
            if(e.type == SDL_KEYUP)
                new_state = INPUT_UP;
    
            switch(e.key.keysym.sym)
            {
                case SDLK_RIGHT:
                actions->right=Input_GetNewState(new_state, previous->right);
                break;

                case SDLK_LEFT:
                actions->left=Input_GetNewState(new_state, previous->left);
                break;

                case SDLK_w:
                actions->run=Input_GetNewState(new_state, previous->run);
                break;

                case SDLK_x:
                actions->jump=Input_GetNewState(new_state, previous->jump);
                break;
            }
        }
    }

    actions->right=Input_GetNewState(actions->right, previous->right);
    actions->left=Input_GetNewState(actions->left, previous->left);
    actions->jump=Input_GetNewState(actions->jump, previous->jump);
    actions->run=Input_GetNewState(actions->run, previous->run);

    free(previous);
    return actions;
}

void Input_InitInputActions(InputActions *actions)
{
    actions->right=INPUT_UP;
    actions->left=INPUT_UP;
    actions->jump=INPUT_UP;
    actions->run=INPUT_UP;
}

int Input_GetNewState(int current, int previous)
{
    if(current==INPUT_DOWN || current==INPUT_JUSTDOWN)
    {
        if(previous==INPUT_UP || previous==INPUT_JUSTUP)
            return INPUT_JUSTDOWN;
        else
            return INPUT_DOWN;
    }
    else
    {
        if(previous==INPUT_DOWN || previous==INPUT_JUSTDOWN)
            return INPUT_JUSTUP;
        else
            return INPUT_UP;
    }
}

int Input_IsAnyDown(int input)
{
    if(input == INPUT_DOWN || input == INPUT_JUSTDOWN)
        return 1;
    else
        return 0;
}

int Input_IsAnyUp(int input)
{
    if(input == INPUT_UP || input == INPUT_JUSTUP)
        return 1;
    else
        return 0;
}