#include "auxlibs/u64/u64.h"
#include "lado.h"

/* 'dir' options:
 * Es la direccion en la que se encuentra un vecino, respecto al nodo padre.
 * Una vecindad (Nbrhd) se divide en 2 zonas: FWD y BWD.
 * Un mismo vecino no puede estar en ambas zonas (loops)*/
#define FWD 1   /*Forward*/
#define BWD -1  /*Backward*/

/* Estructura de la vecindad de un nodo*/
typedef struct NeigbourhoodSt * Nbrhd;

/*Constructor de un nuevo Nbrhd*/
Nbrhd nbrhd_create();

/*Destructor de un Nbrhd*/
void nbrhd_destroy(Nbrhd nbrhd);

/* Genera el vinculo entre 'x' e 'y' (edge) convirtiendolos en vecinos,
 * la relacion es xy: 'y' vecino forward de 'x', 'x' vecino backward de 'y'
 * Precondicion: x, y, edge != NULL
 */
void nbrhd_addEdge(Nbrhd x, Nbrhd y, Lado edge);

/* Busca el vecino que sigue despues de 'y' en la direccion 'dir' (FWD o BWD)
 * Retorno: y = NULL, u64 nombre del primer vecino en esa direccion
 *          y != NULL, u64 nombre del sig vecino desp de 'y' en esa direccion
 *          NULL, si no existen mas vecinos en esa direccion */
u64 nbrhd_getNext(Nbrhd nbrs, u64 y, int dir);

/* Se aumenta el flujo para con el vecino 'y' por 'vf' cantidad. 
 * Si 'y' es un vecino BWD, el valor del flujo se disminuye por 'vf' cantidad
 * Precondicion: y != NULL y es vecino del nodo padre. vf > 0
 * Retorno: valor del nuevo flujo que se esta enviando */
u64 nbrhd_increaseFlow(Nbrhd nbrs, u64 y, u64 vf); 

/*devuelve la capacidad con el vecino 'y'
 Precondicion: y != NULL y es vecino del nodo padre */
u64 nbrhd_getCap(Nbrhd nbrs, u64 y);  

/*devuelve el valor del flujo con el vecino 'y'
 Precondicion: y != NULL y es vecino del nodo padre */
u64 nbrhd_getFlow(Nbrhd nbrs, u64 y); 

/*devuelve la direccion (FWD o BWD) en la que se encuentra el vecino 'y'
 Precondicion: y != NULL y es vecino del nodo padre */
int nbrhd_getDir(Nbrhd nbrs, u64 y); 

