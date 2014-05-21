
/* dir: es la direccion de la arista, opcional para acelerar busquedas si 
 * que queremos hacer en un sentido en particular. 
 * Sino se espicifica, las busquedas son primero por FWD y luego por BWD*/
#define FWD 1
#define BWD -1

/* Estructura de la vecindad de un nodo*/
typedef struct NeigbourhoodSt * Nbrhd;

/*TODO ALL*/

Nbrhd nbrhd_create(u32);/*WARNING obsoleta, no les dimos uso todavia*/
void nbrhd_destroy(Nbrhd nbr);/*WARNING obsoleta, no les dimos uso todavia*/
void nbrhd_set(Nbrhd xNeig, Nbrhd yNeig, u32 cap);/*WARNING obsoleta, no les dimos uso todavia*/
u64 nbrhd_getX(Nbrhd nbr);/*WARNING obsoleta, no les dimos uso todavia*/


u64 nbrhd_getNext(Nbrhd nbr, u64 y, int dir); /*busca entre los vecinos a y*/

void nbrhd_increaseFlow(Nbrhd nbr, u32 y, u32 flow, int dir); /*aumento el flujo para con el vecino y*/

u64 nbrhd_getCap(Nbrhd nbr, u64 y);  /*devuelve la capacidad con el vecino y*/

u64 nbrhd_getFlow(Nbrhd nbr, u64 y); /*devuelve el valor del flujo con el vecino y*/
