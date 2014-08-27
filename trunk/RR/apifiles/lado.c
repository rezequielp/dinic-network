#include "lado.h"
#include <assert.h>
#include <stdlib.h>

/** Estructura de un Lado*/
struct LadoSt{
    u64 x;
    u64 y;
    u64 c;
};

/* Constructores */

/** Creador de un nuevo lado.
 * Es la representación de una arista \a xy con capacidad de flujo \a c.
 * \param x Nombre del nodo x.
 * \param y Nombre del nodo y.
 * \param c Capacidad del lado xy.
 * \return Un nuevo lado con los correspondientes valores asignados.
 */
Lado lado_new(u64 x, u64 y, u64 c){
	Lado edge = NULL;   /*El lado a crear y devolver*/
    
    edge = (Lado)malloc(sizeof(struct LadoSt));
    assert(edge != NULL);
    edge->x = x;
    edge->y = y;
    edge->c = c;
    
    return edge;
}

/** Destructor de un lado.
 * Se destruye un lado, al menos que ya sea un lado nulo.
 * \param edge Lado a destruir.
 */
void lado_destroy(Lado edge){
    if(edge != LadoNulo){
        free(edge);
    }
    edge = LadoNulo;
}

/*  Operaciones */

/** Obtener el nombre del nodo \a x.
 * \param edge Lado en el que se desea consultar.
 * \pre \a edge no es un lado nulo.
 * \return El nombre del nodo \a x.
 */
u64 lado_getX(Lado edge){
    assert(edge!=LadoNulo);
    return edge->x;
}

/** Obtener el nombre del nodo \a y.
 * \param edge Lado en el que se desea consultar.
 * \pre \a edge no es un lado nulo.
 * \return El nombre del nodo \a y.
 */
u64 lado_getY(Lado edge){
    assert(edge!=LadoNulo);
	return edge->y;
}

/** Obtener la capacidad de un lado.
 * \param edge Lado en el que se desea consultar.
 * \pre \a edge no es un lado nulo.
 * \return La capacidad del lado
 */
u64 lado_getCap(Lado edge){
    assert(edge!=LadoNulo);
	return edge->c;
}
