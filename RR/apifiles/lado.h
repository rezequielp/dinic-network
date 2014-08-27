#ifndef LADO_H
#define LADO_H

#include "u64.h"

/** \file lado.h
 * Lado es una estructura compuesta por el nombre del nodo x, el nombre del 
 * nodo y, y la capacidad de flujo del lado.
 */

typedef struct LadoSt *Lado;

/* Representación de un lado nulo*/
#define LadoNulo (Lado)NULL /**< Un lado que es nulo.*/

/* Constructores */

/* Creador de un nuevo lado.
 * Es la representación de una arista \a xy con capacidad de flujo \a c.
 * x Nombre del nodo x.
 * y Nombre del nodo y.
 * c Capacidad del lado xy.
 * return: Un nuevo lado con los correspondientes valores asignados.
 */
Lado lado_new(u64 x, u64 y, u64 c);

/* Destructor de un lado.
 * Se destruye un lado, al menos que ya sea un lado nulo.
 * edge lado a destruir.
 */
void lado_destroy(Lado edge);


/*  Operaciones */

/* Obtener el nombre del nodo \a x.
 * edge lado en el que se desea consultar.
 * pre: \a edge no es un lado nulo.
 * return: El nombre del nodo \a x.
 */
u64 lado_getX(Lado edge);

/* Obtener el nombre del nodo \a y.
 * edge lado en el que se desea consultar.
 * pre: \a edge no es un lado nulo.
 * return: El nombre del nodo \a y.
 */
u64 lado_getY(Lado edge);

/* Obtener la capacidad de un lado.
 * edge lado en el que se desea consultar.
 * pre: \a edge no es un lado nulo.
 * return: La capacidad del lado
 */
u64 lado_getCap(Lado edge);

#endif
