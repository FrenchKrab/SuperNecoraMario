#ifndef KDS_H
#define KDS_H

/*-------DATA STRUCTURES------*/


#define Stack List

typedef struct s_list List;
struct s_list
{
	List *next;
	void *value;
};


typedef struct
{
	int *array;	/*array is a 1d array of int*/
	int capacity;
	/*Note: tail, head are all indexes to tab*/
	int tail;
	int head;
} Queue;

#define HASHPAIR_STR_KEYLENGHT 256

/*A pair (key,value) with the key being a 255 characters string*/
typedef struct
{
	void *value;
	char key[HASHPAIR_STR_KEYLENGHT];
	//size_t (*hashfunction)(char[], size_t capac);	//Could be useful
} Hashpair_str;

/*Hashmap using separate chaining */
typedef struct
{
	size_t capacity;
	List *array[];
} Hashmap_str;




List *List_create(void* value, List* next);	/*Create a list*/ 
void List_remove(List**, void *element, void (*freeOp) (void*));	/*Remove the given element from the list*/
void List_add(List**, void *element);	/*Add an element at the start of the list*/
int List_contains(List*, void *element);	/*Returns 1 if list contains element, 0 else*/
int List_addUnique(List**, void *element);	/*Add an element only if not already present, returns 1 on success, 0 if already present*/
void List_append(List **list, void *element);	/*Append the element at the end of the list*/
List *List_last(List*);	/*Returns last non NULL cell*/
void List_link(List** a, List* b);	/*Links the list a to the list b*/
void List_free(List*, void (*freeOp) (void*));	/*Frees a list and all its values*/
size_t List_size(List *list);
void *List_toArray(List *list, size_t elementSize);
List *List_fromArray(void *array, size_t size, size_t typeSize);

Hashpair_str *Hashpair_str_create(char key[HASHPAIR_STR_KEYLENGHT], void *value);
Hashmap_str *Hashmap_str_create(size_t capacity);
void Hashmap_str_add(Hashmap_str *map, char key[], void *value);
void *Hashmap_str_get(Hashmap_str *map, char key[]);

size_t Hash_calculate_from_str(char str[], size_t capacity);
/*
void Stack_push(Stack**, void *value);
void *Stack_pop(Stack**);
void *Stack_top(Stack*);
*/

#endif