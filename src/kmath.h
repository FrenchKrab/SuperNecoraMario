#ifndef _KMATH_H
#define _KMATH_H

#include <SDL2/SDL.h>


#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif 

typedef struct {
    float x;
    float y;
} Vect2;


int Vect2_Equals(Vect2 v1, Vect2 v2);
Vect2 Vect2_Add(Vect2 v1, Vect2 v2);
SDL_Point Vect2_ToPoint(Vect2 v);

int KMath_SignInt(int a);
int KMath_ClampInt(int x, int min, int max);
int KMath_MaxInt(int a, int b);
int KMath_MinInt(int a, int b);

float KMath_ClampFloat(float x, float min, float max);

int KMath_RoundToInt(float f);
Vect2 Vect2_FromPoint(SDL_Point point);

void KMath_Rect_Multiply(SDL_Rect *rect, float multiplier);
void KMath_Rect_Offset(SDL_Rect *a, SDL_Point b);
SDL_Point KMath_Rect_GetCenter(SDL_Rect a);

#endif