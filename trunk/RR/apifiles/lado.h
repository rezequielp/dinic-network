#ifndef LADO_H
#define LADO_H

#include "u64.h"

/*
 * Lado es una estructura compuesta por el nombre del nodo x, el nombre del 
 * nodo y, y la capacidad de flujo del lado
 */
typedef struct LadoSt *Lado;

#define LadoNulo NULL

/* Constructores
*/
/*Crea un nuevo lado con los valores del nodo x, nodo y, capacidad c.*/
Lado lado_new(u64 x, u64 y, u64 c);

/*Destruye la estructura lado, no los elementos que lo componen.*/
void lado_destroy(Lado edge);


/*  Operaciones
*/
u64 lado_getX(Lado edge);   /*devuelve el nombre del nodo x*/
u64 lado_getY(Lado edge);   /*devuelve el nombre del nodo y*/
u64 lado_getCap(Lado edge); /*devuelve la capacidad de flujo del lado*/

#endif
