#ifndef _NBRHD_H
#define _NBRHD_H

#include "u64.h"
#include "lado.h"

/* 'dir' options:
 * Es la direccion en la que se encuentra un vecino.
 * Una vecindad (Nbrhd) se divide en 2 zonas: FWD y BWD.
 * Un mismo vecino no puede estar en ambas zonas (loops)*/
#define FWD 1   /*Forward*/
#define BWD -1  /*Backward*/
#define UNK 0   /*direccion desconocida*/

/*          Parametros para nbrhd_getNext()  */
#ifndef _GET_NEXT
#define _GET_NEXT
/* 'flag' options:
 * Indica cual es el siguiente vecino que se quiere obtener.*/
#define FST 0       /*El primero*/
#define NXT 1       /*El siguiente del último consultado*/
/* 'return' options:
 * La direccion en la que se encuentra el vecino devuelto (FST o NXT), o bien*/
#define NONE -2   /*Ninguno. No hay mas vecinos en esa direccion*/
#endif


/*          Estructura de la vecindad de un nodo
 */
typedef struct NeighbourhoodSt *Nbrhd;

/*          Funciones
 */

/*Constructor de un nuevo Nbrhd*/
Nbrhd nbrhd_create(void);


/*Destructor de un Nbrhd*/
void nbrhd_destroy(Nbrhd nbrhd);


/* Genera el vinculo entre 'x' e 'y' (edge) convirtiendolos en vecinos,
 * la relacion es xy: 'y' vecino forward de 'x', 'x' vecino backward de 'y'
 * Precondicion: x, y, edge != NULL
 */
void nbrhd_addEdge(Nbrhd x, Nbrhd y, Lado edge);


/* NOTE Tener en cuenta la documentacion sobre las opciones de los parametros
 * y retorno. Verlo como una iteracion sobre una lista en la que empiezo por 
 * el primer(FST) elemento, o bien por el siguiente(NXT) del ultimo consultado
 * 
 * Busca el vecino siguiente en la direccion 'dir' y si existe 
 * almacena el nombre en 'y'. Si dir=UNK entonces primero intenta por FWD, y si
 * no hay (o no existen) entonces intenta por BWD.
 * Precondicion: nbrs!=NULL, flag=FST|NXT, dir=FWD|BWD|UNK, y!=NULL
 * Retorno(r):  r = 'dir', en la que se encontro un vecino
 *              r = NONE, ya no hay mas vecinos que devolver
 */
int nbrhd_getNext(Nbrhd nbrs, int flag, int dir, u64 *y);


/* Se aumenta el flujo para con el vecino 'y' por 'vf' cantidad. 
 * Si 'y' es un vecino BWD, el valor del flujo se disminuye por 'vf' cantidad
 * Precondicion: 'y' es vecino. vf > 0
 * Retorno: valor del nuevo flujo que se esta enviando */
u64 nbrhd_increaseFlow(Nbrhd nbrs, u64 y, u64 vf); 


/*devuelve la capacidad con el vecino 'y'
 Precondicion: 'y' es vecino */
u64 nbrhd_getCap(Nbrhd nbrs, u64 y);  


/*devuelve el valor del flujo con el vecino 'y'
 Precondicion: 'y' es vecino */
u64 nbrhd_getFlow(Nbrhd nbrs, u64 y); 


/*devuelve la direccion (FWD o BWD) en la que se encuentra el vecino 'y'
 Precondicion: 'y' es vecino */
int nbrhd_getDir(Nbrhd nbrs, u64 y); 

#endif
