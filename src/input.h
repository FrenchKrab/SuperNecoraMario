#ifndef INPUT_H
#define INPUT_H


enum PressType
{
    INPUT_UP = 0,
    INPUT_DOWN = 1,
    INPUT_JUSTUP = 2,
    INPUT_JUSTDOWN = 3
};

typedef struct
{
    int right;
    int left;
    int jump;
    int run;
    int quit;
} InputActions;


InputActions *InputActions_Create();    //Create an InputActions with all its input released

//Free the previous and create a new input actions
InputActions *InputActions_PollFromSDL(InputActions *previous); 

int Input_GetNewState(int current, int previous);

//Returns 1 if input is pressed or just pressed
int Input_IsAnyDown(int input);
//Returns 1 if input is released or just released
int Input_IsAnyUp(int input);

#endif