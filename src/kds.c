#include <stdio.h>
#include <stdlib.h>
#include "kds.h"
#include <math.h>
#include <string.h>


/*-----LIST FUNCTIONS-----*/

List *List_create(void *value, List *next)
{
    List *new_list = malloc(sizeof(List));
    new_list->next = next;
    new_list->value = value;
    return new_list;
}

void List_remove(List **list, void *element, void(*freeOp) (void*))
{
    List *l = *list;
    List *prev = NULL;
    while(l != NULL)
    {
        if(l->value==element)   //Si l contient l'element à supprimer
        {
            if(prev==NULL)  //Si l est le premier element de la liste
            {
                *list = l->next;    //On définit l->next comme nouveau premier elt de la liste
            }
            else    //Sinon, on relie la cellule precedant l à celle le suivant
            {
                prev->next = l->next;
            }
            if(freeOp!=NULL)
                freeOp(l->value);    //Libérer l
            free(l);
            return;
        }
        else    //Sinon, passer à l'element suivant
        {
            prev = l;
            l=l->next;
        }
    }
}


void List_add(List** list, void *element)
{
    *list=List_create(element, *list);
}

int List_contains(List *list, void *element)
{
    List *L = list;
    while(L!=NULL)
    {
        if(element==L->value)
            return 1;
        L=L->next;
    }
    return 0;
}

int List_addUnique(List **list, void *element)
{
    if(!List_contains(*list, element))
    {
        List_add(list, element);
        return 1;   //Added successfully
    }
    return 0;   //Couldn't add
}

void List_append(List **list, void *element)
{
    List *l = *list;
    List *prev = NULL;
    while(l != NULL)
    {
        prev = l;
        l=l->next;
    }

    if(prev == NULL)
    {
        *list = List_create(element, NULL);
    }
    else
    {
        prev->next = List_create(element, NULL);
    }
}

List *List_last(List *list)
{
    List *l = list;
    while(l!=NULL && l->next != NULL)
    {
        l=l->next;
    }
    return l;
}

void List_link(List **a, List *b)
{
    if(*a==NULL)    //If a contains no list, set a as b
    {
        *a = b;
    }
    else if(b==NULL)
    {
        //We have nothing to do
    }
    else
    {
        List *a_last = List_last(*a);
        a_last->next = b;
    }
}

void List_free(List *list, void (*freeOp) (void*))
{
    List *l = list;
    while(l != NULL)
    {
        if(freeOp != NULL)
        {
            freeOp(l->value);
        }
        List *toFree = l;
        l=l->next;
        free(toFree);
    }
}

size_t List_size(List *list)
{
    List *l = list;
    size_t size = 0;
    while(l!=NULL)
    {
        size += 1;
        l=l->next;
    }
    return size;
}

void *List_toArray(List *list, size_t elementSize)
{
    size_t size = List_size(list);
    /*Utiliser un pointeur d'unsigned char permet d'être sûr que la taille
    du pointeur est de 1 byte, et donc de pouvoir offset par un nombre de byte voulu*/
    char *array = malloc(elementSize*size);
    List *l = list;
    for(int i = 0; i<size; i++)
    {
        memcpy(array+(i*elementSize), l->value, elementSize);
        l=l->next;
    }
    return array;
}

List *List_fromArray(void *array, size_t size, size_t typeSize)
{
    char *arr = (char*)array;
    List *list = NULL;
    for(int i=size-1; i>=0; i--)
    {
        void *value = malloc(typeSize);
        memcpy(value, arr+i*typeSize, typeSize);
        list = List_create(value, list);
    }
    return list;
}

Hashmap_str *Hashmap_str_create(size_t capacity)
{
    Hashmap_str *newMap = malloc(sizeof(Hashmap_str) + sizeof(List)*capacity);
    newMap->capacity = capacity;
    for(int i = 0; i<capacity; i++)
    {
        *(newMap->array + i)=NULL;
    }
    return newMap;
}

Hashpair_str *Hashpair_str_create(char key[HASHPAIR_STR_KEYLENGHT], void *value)
{
    Hashpair_str *pair = malloc(sizeof(Hashpair_str));
    strncpy(pair->key, key, HASHPAIR_STR_KEYLENGHT);
    pair->value = value;
    //printf("Pair created with : %s as key", pair->key);   //debug
    return pair;
}

void Hashmap_str_add(Hashmap_str *map, char key[], void *value)
{
    long unsigned int index = Hash_calculate_from_str(key, map->capacity);
    //printf("Hash:%lu", index);  //debug
    List_append(&(map->array[index]), Hashpair_str_create(key, value));
}

void *Hashmap_str_get(Hashmap_str *map, char key[])
{
    size_t index = Hash_calculate_from_str(key, map->capacity);
    List *l = map->array[index];
    while(l != NULL)
    {
        if(strcmp(((Hashpair_str*)l->value)->key, key) == 0)
        {
            return ((Hashpair_str*)l->value)->value;
        }
        else
        {
            l=l->next;
        }
    }
    return NULL;    //If no value found
}

size_t Hash_calculate_from_str(char str[], size_t capacity)
{
    double factor = (sqrt(5)-1)/2;
    size_t e = 0;

    for (int i = 0; i < strlen(str); i++)
    {
        char c = str[i];
        size_t a = (size_t)c;
        a = a << (i%7);
        e += a;
    }
    double fracpart = 0.0;
    fracpart = (double)e * factor;
    fracpart = fracpart - (long)fracpart;
    size_t position = (size_t)(fracpart * capacity);
    return position;
}