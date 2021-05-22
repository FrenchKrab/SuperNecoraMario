#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include "kmath.h"

int Vect2_Equals(Vect2 v1, Vect2 v2)
{
    if(v1.x == v2.x && v1.y == v2.y)
        return 1;
    else
        return 0;
}

Vect2 Vect2_Add(Vect2 v1, Vect2 v2)
{
    Vect2 result;
    result.x = v1.x + v2.x;
    result.y = v1.x + v2.y;
    return result;
}

SDL_Point Vect2_ToPoint(Vect2 v)
{
    SDL_Point result;
    result.x = KMath_RoundToInt(v.x);
    result.y = KMath_RoundToInt(v.y);
    return result;
}

Vect2 Vect2_FromPoint(SDL_Point point)
{
    return (Vect2){point.x, point.y};
}

int KMath_RoundToInt(float f)
{
    int whole = (int)f;
    if(f-whole>=0.5)
    {
        return whole+1;
    }
    else
    {
        return whole;
    }
}

int KMath_SignInt(int a)
{
    if(a>=0)
        return 1;
    else
        return -1;
}

float KMath_ClampFloat(float x, float min, float max)
{
    if(x < min)
        return min;
    if(x > max)
        return max;
    return x;
}

int KMath_ClampInt(int x, int min, int max)
{
    if(x < min)
        return min;
    if(x > max)
        return max;
    return x;
}

int KMath_MaxInt(int a, int b)
{
    if(a>b)
        return a;
    else
        return b;
}

int KMath_MinInt(int a, int b)
{
    if(a<b)
        return a;
    else
        return b;
}

void KMath_Rect_Multiply(SDL_Rect *rect, float multiplier)
{
    rect->x = ((float)rect->x * multiplier);
    rect->y = ((float)rect->y * multiplier);
    rect->w = ((float)rect->w * multiplier);
    rect->h = ((float)rect->h * multiplier);
}

void KMath_Rect_Offset(SDL_Rect *a, SDL_Point b)
{
    a->x=a->x + b.x;
    a->y=a->y + b.y;
}

SDL_Point KMath_Rect_GetCenter(SDL_Rect a)
{
    return (SDL_Point){a.x+a.w/2, a.y+a.h/2};
}
