#include "lado.h"
#include "auxlibs\u64"

typedef struct Lado{
    u64 x;
    u64 y;
    u64 c;
};

/* Constructores
*/

Lado lado_new(u64 x, u64 y, u64 c){
	Lado edge;
    edge = (Lado)malloc(sizeof(struct LadoSt)))
    edge->x = x;
    edge->y = y;
    edge->c = c;
    return edge;
}
void lado_destroy(Lado edge){
	free(edge);
}

/*  Operaciones
*/
u64 lado_getY(Lado edge){
	return edge->y;
}
u64 lado_getX(Lado edge){
	return edge->x;
}
u64 lado_getCap(Lado edge){
	return edge->c;
}
