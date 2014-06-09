#include "lado.h"
#include <assert.h>
#include <stdlib.h>

struct LadoSt{
    u64 x;
    u64 y;
    u64 c;
};

/* Constructores
*/

Lado lado_new(u64 x, u64 y, u64 c){
	Lado edge = NULL;
    
    edge = (Lado)malloc(sizeof(struct LadoSt));
    assert(edge != NULL);
    edge->x = x;
    edge->y = y;
    edge->c = c;
    
    return edge;
}

void lado_destroy(Lado edge){
    if(edge != LadoNulo){
        free(edge);
    }
    edge = LadoNulo;
}

/*  Operaciones
*/
u64 lado_getY(Lado edge){
    assert(edge!=LadoNulo);
	return edge->y;
}

u64 lado_getX(Lado edge){
    assert(edge!=LadoNulo);
	return edge->x;
}

u64 lado_getCap(Lado edge){
    assert(edge!=LadoNulo);
	return edge->c;
}
