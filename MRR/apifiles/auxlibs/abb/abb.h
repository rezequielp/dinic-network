#ifndef ABB_H
#define ABB_H

#include <stdbool.h>

typedef struct AbbSt *Abb;

Abb abb_create (void);/* constructor */
void abb_add (Abb tree, Key x, void *data);/* agrega una clave y un dato */
bool abb_exists (Abb , u32 x);/* devuelve si existe la clave en el abb */
PandorasBox abb_search (Abb tree, Key x);/* busca un dato segun la clave*/
int abb_size (Abb tree);/* devuelve la cantidad de elementos */
void abb_pprint (Abb l);/* imprime todos los pares (Key, Data) en el arbol *//* VER PARA IMPR*/
void abb_destroy(Abb tree, Stack garbage);/* Destructor. Destruye el abb y su contenido *//* PRE: Stack viene creado*/
bool abb_isEmpty(Abb tree);/*TODO*/



#endif
