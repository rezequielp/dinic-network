#ifndef _NBRHD_H
#define _NBRHD_H

#include "_u64.h"
#include "lado.h"

/** \file nbrhd.h
 * Encabezado de nbrhd.c.
 * \note
 * Lo que figure en doble parentesis en este texto, como por ej ((algo)), estará
 * indicando una notación sobre toda la implementación de este archivo.
 * 
 * La vecindad ((Nbrhd)) de un nodo ancestro (('x')), es la representación del 
 * conjunto de nodos vecinos (('y')) que estan relacionados a éste por medio de 
 * un lado en el network. Nbrhd se divide en 2 subconjuntos de nodos 'y': los 
 * forward ((FWD)) y los backward ((BWD)).
 * Un nodo 'y' es FWD de 'x' si la dirección de envio de flujo es de 'x' hacia 
 * 'y'. Cuando nos refiramos a un lado FWD vamos a estar indicando esta 
 * correspondencia y lo denotaremos como lado (('xy')). Todo flujo enviado
 * por un lado FWD aumenta el valor del flujo existente en este lado.
 * Por el contrario, 'y' es BWD de 'x' si dicha dirección es de 'y' hacia 'x', y 
 * denotaremos al lado BWD como (('yx')). Todo flujo enviado por un lado BWD
 * disminuye el valor del flujo existente en este lado.
 * 
 * Con esta noción de Nbrhd se puede deducir que todos los lados 'xy' 
 * pertenecientes al network estan representados como lado FWD en el Nbrhd de
 * su correspondiente nodo 'x', y como lado BWD en el Nbrhd de su 
 * correspondiente nodo 'y'.
 * 
 * Como pueden existir loops (ie. existen 2 lados FWD: x->y , y->x) un nodo 
 * puede tener a un mismo vecino tanto por FWD como por BWD. Por lo tanto, es 
 * necesario especificar cuál lado es con el que se esta queriendo trabajar. 
 * Esto se soluciona indicando la dirección, ya que para el nodo 'x' el vecino 
 * por FWD esta representando al lado 'xy', es decir x->y; mientras que el BWD 
 * es al lado 'yx' que justamente viene a ser el FWD y->x.
 */

/** \struct Nbrhd 
 * Puntero a la vecindad de un nodo. */
typedef struct NeighbourhoodSt *Nbrhd;

/* dir options:
 * Es la direccion en la que se encuentra un vecino.
 * Una vecindad (Nbrhd) se divide en 2 zonas: FWD y BWD.*/
#define FWD 1   /**<Forward*/
#define BWD -1  /**<Backward*/

/* Parametros para nbrhd_getFwd() y nbrhd_getBwd */
/*'rqst' options: Indica cual es el siguiente vecino que se quiere obtener.*/
#define FST 0       /**<El primero*/
#define NXT 1       /**<El siguiente del último consultado*/


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
 * La relacion es 'xy': 'y' vecino forward de 'x'; 'x' vecino backward de 'y'.
 * pre: 'x', 'y', 'edge' no son nulos.
 * x Vecindario del nodo 'x'.
 * y Vecindario del nodo 'y'.
 * edge Lado 'xy' con su capacidad y flujo.
 */
void nbrhd_addEdge(Nbrhd x, Nbrhd y, Lado edge);

/* Busca el siguiente vecino forward.
 * La peticion de busqueda puede ser por el primer nodo (FST) de la tabla, 
 * o bien por el siguiente(NXT) del ultimo pedido. 
 * Si existe, almacena el nombre en 'y'.
 * 
 * NOTE Tener en cuenta la documentacion sobre las opciones de los parametros.
 * Verlo como un iterador de consultas a una tabla.
 * 
 * nbrs  El vecindario del nodo ancestro 'x'. 
 * rqst  Si se pide el primero 'FST' o un siguiente 'NXT'.
 * y     Variable en la que se almacena el nombre del vecino encontrado.
 * pre: 'nbrs' e 'y' no son nulos y 'rqst' es una opcion valida (FST o NXT)
 * return: 1 Si se encontro y se almaceno en 'y' un vecino.
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
 * nbrs  El vecindario del nodo ancestro 'x'. 
 * rqst  Si se pide el primero 'FST' o un siguiente 'NXT'.
 * y     Variable en la que se almacena el nombre del vecino encontrado.
 * pre: 'nbrs' e 'y' no son nulos y 'rqst' es una opcion valida (FST o NXT)
 * return: 1 Si se encontro y se almaceno en 'y' un vecino.
 *         0 Caso contrario.
 */
int nbrhd_getBwd(Nbrhd nbrs, int rqst, u64 *y);

/* Se aumenta el flujo para con el vecino 'y' por 'vf' cantidad. 
 * Si 'y' es un vecino BWD, el valor del flujo se disminuye por 'vf' cantidad.
 * nbrs  El vecindario del nodo ancestro 'x'. 
 * y     El nombre del vecino.
 * vf    El valor de flujo.
 * pre: 'y' es vecino de 'x'. 'vf' > 0
 * return: Valor del nuevo flujo que se esta enviando entre 'x' e 'y'.*/
u64 nbrhd_increaseFlow(Nbrhd nbrs, u64 y, short int dir, u64 vf); 

/* Devuelve la capacidad del lado que relaciona al nodo ancestro 'x' con el 
 * vecino 'y'.
 * Como pueden haber loops hay que especificar si se esta tratando del lado 'xy'
 * o 'yx'.
 * nbrs  El vecindario del nodo ancestro 'x'. 
 * y     El nombre del vecino.
 * dir   Direccion que se encuentra el vecino (lado FWD o BWD)
 * pre: 'y' es vecino de 'x'. 
 * return:  La capacidad sobre este lado.
 */
u64 nbrhd_getCap(Nbrhd nbrs, u64 y, short int dir);  

/* Devuelve el valor del flujo del lado que relaciona al nodo ancestro 'x' con 
 * el vecino 'y'.
 * Como pueden haber loops hay que especificar si se esta tratando del lado 'xy'
 * o 'yx'.
 * nbrs  El vecindario de 'x'. 
 * y     El nombre del vecino.
 * dir   Direccion que se encuentra el vecino (lado FWD o BWD)
 * pre: 'y' es vecino de 'x'. 
 * return:  El valor del flujo sobre este lado.
 */
u64 nbrhd_getFlow(Nbrhd nbrs, u64 y, short int dir); 

#endif
