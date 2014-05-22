
/* 'dir':
 * Es la direccion en la que se encuentra un vecino, respecto al nodo padre.
 * Una vecindad (Nbrhd) se divide en 2 zonas: FWD y BWD.
 * Un mismo vecino no puede estar en ambas zonas (loops)*/
#define FWD 1   /*Forward*/
#define BWD -1  /*Backward*/

/* Estructura de la vecindad de un nodo*/
typedef struct NeigbourhoodSt * Nbrhd;

/*TODO ALL*/

Nbrhd nbrhd_create(u32);/*WARNING obsoleta, no les dimos uso todavia*/
void nbrhd_destroy(Nbrhd nbr);/*WARNING obsoleta, no les dimos uso todavia*/
void nbrhd_set(Nbrhd xNeig, Nbrhd yNeig, u32 cap);/*WARNING obsoleta, no les dimos uso todavia*/
u64 nbrhd_getX(Nbrhd nbr);/*WARNING obsoleta, no les dimos uso todavia*/


/* Busca el vecino que sigue despues de 'y' en la direccion 'dir' (FWD o BWD)
 * Retorno: y = NULL, u64 con el primer vecino en esa direccion
 *          y != NULL, u64 con el nombre del siguente vecino en esa direccion
 *          NULL, si no existen mas vecinos en esa direccion; o bien
 *                'y' no se encuentra en esa direccion, o no es vecino del nodo padre*/
u64 nbrhd_getNext(Nbrhd nbrs, u64 y, int dir);

/* Se aumenta el flujo para con el vecino 'y' por 'vf' cantidad. 
 * Si 'y' es un vecino BWD, el valor del flujo se disminuye por 'vf' cantidad
 * Retorno: valor del nuevo flujo, NULL si ocurrio algun error o 'y' no se encontro*/
u64 nbrhd_increaseFlow(Nbrhd nbrs, u64 y, u64 vf); 

/*devuelve la capacidad con el vecino y
 NULL, si 'y' no se encuentra entre los vecinos*/
u64 nbrhd_getCap(Nbrhd nbrs, u64 y);  

/*devuelve el valor del flujo con el vecino y
 NULL, si 'y' no se encuentra entre los vecinos*/
u64 nbrhd_getFlow(Nbrhd nbrs, u64 y); 
