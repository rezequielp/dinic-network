#ifndef _NBRHD_H
#define _NBRHD_H

#include "_u64.h"
#include "lado.h"

/** \file nbrhd.h
 * Encabezado de nbrhd.c.
 * 
 * ESCRIBIR lo que es y hace neighboorhood 
 */

/** \struct Nbrhd 
 * Puntero a la vecindad de un nodo. */
typedef struct NeighbourhoodSt *Nbrhd;

/* dir options:
 * Es la direccion en la que se encuentra un vecino.
 * Una vecindad (Nbrhd) se divide en 2 zonas: FWD y BWD.
 * Un mismo vecino no puede estar en ambas zonas (loops)*/
#define FWD 1   /**<Forward*/
#define BWD -1  /**<Backward*/
#define UNK 0   /**<Direccion desconocida*/

/* Parametros para nbrhd_getNext()  */
/*'flag' options: Indica cual es el siguiente vecino que se quiere obtener.*/
#define FST 0       /**<El primero*/
#define NXT 1       /**<El siguiente del último consultado*/
/* 'return' options: La direccion hacia el vecino devuelto: FWD, NXT, NONE.*/
#define NONE -2   /**<Ninguno. No hay mas vecinos en esa direccion.*/




/*          Funciones
 */

/* Constructor de un nuevo Nbrhd.
 * return: un Nbrhd vacío.
 */
Nbrhd nbrhd_create(void);

/* Destructor de un Nbrhd.
 * pre: Nbrhd no es nulo.
 */
void nbrhd_destroy(Nbrhd nbrhd);

/* Genera el vinculo entre 'x' e 'y' (edge) convirtiendolos en vecinos.
 * la relacion es 'xy': 'y' vecino forward de 'x', 'x' vecino backward de 'y'
 * pre: 'x', 'y', 'edge' no son nulos.
 * x Vecindario del nodo 'x'.
 * y Vecindario del nodo 'y'.
 * edge Lado 'xy' con su capacidad y flujo.
 */
void nbrhd_addEdge(Nbrhd x, Nbrhd y, Lado edge);

/* Busca el siguiente vecino forward.
 * La peticion de busqueda puede ser por el primer nodo (FST) de la tabla, 
 * o bien por el siguiente(NXT) del ultimo pedido. 
 * Si existe almacena el nombre en 'y'.
 * 
 * NOTE Tener en cuenta la documentacion sobre las opciones de los parametros.
 * Verlo como un iterador de consultas a una tabla.
 * 
 * nbrs  El vecindario de 'x'. 
 * rqst  Si se pide el primero 'FST' o un siguiente 'NXT'.
 * y     Variable en la que se almacena el nombre del vecino encontrado.
 * pre: 'nbrs' e 'y' no son nulos y 'rqst' es una opcion valida (FST o NXT)
 * return: 1 Si se encontro y almaceno en 'y' un vecino.
 *         0 Caso contrario.
 */
int nbrhd_getFwd(Nbrhd nbrs, int rqst, u64 *y);

/* Busca el siguiente vecino backward.
 * La peticion de busqueda puede ser por el primer nodo (FST) de la tabla, 
 * o bien por el siguiente(NXT) del ultimo pedido. 
 * Si existe almacena el nombre en 'y'.
 * 
 * NOTE Tener en cuenta la documentacion sobre las opciones de los parametros.
 * Verlo como un iterador de consultas a una tabla.
 * 
 * nbrs  El vecindario de 'x'. 
 * rqst  Si se pide el primero 'FST' o un siguiente 'NXT'.
 * y     Variable en la que se almacena el nombre del vecino encontrado.
 * pre: 'nbrs' e 'y' no son nulos y 'rqst' es una opcion valida (FST o NXT)
 * return: 1 Si se encontro y almaceno en 'y' un vecino.
 *         0 Caso contrario.
 */
int nbrhd_getBwd(Nbrhd nbrs, int rqst, u64 *y);

/* Se aumenta el flujo para con el vecino 'y' por 'vf' cantidad. 
 * Si 'y' es un vecino BWD, el valor del flujo se disminuye por 'vf' cantidad.
 * nbrs  El vecindario de 'x'. 
 * y     El nombre del vecino.
 * vf    El valor de flujo.
 * pre: 'y' es vecino de 'x'. 'vf' > 0
 * return: Valor del nuevo flujo que se esta enviando entre 'x' e 'y'. */
u64 nbrhd_increaseFlow(Nbrhd nbrs, u64 y, u64 vf); 

/* Devuelve la capacidad con el vecino 'y' del lado 'xy'.
 * nbrs  El vecindario de 'x'. 
 * y     El nombre del vecino.
 * pre: 'y' es vecino de 'x'. 
 * return:  La capacidad del lado 'xy'.
 */
u64 nbrhd_getCap(Nbrhd nbrs, u64 y);  

/* Devuelve el valor del flujo con el vecino 'y' del lado 'xy'.
 * nbrs  El vecindario de 'x'. 
 * y     El nombre del vecino.
 * pre: 'y' es vecino de 'x'. 
 * return:  El valor de flujo del lado 'xy'.
 */
u64 nbrhd_getFlow(Nbrhd nbrs, u64 y); 

/* Devuelve la dirección en la que se encuentra el vecino 'y' respecto a 'x'.
 * nbrs  El vecindario de 'x'.
 * y     El nombre del vecino.
 * pre: 'y' es vecino de 'x'. 
 * return:  La dirección entre el nodo 'x' e 'y'.
 */
int nbrhd_getDir(Nbrhd nbrs, u64 y); 

#endif
