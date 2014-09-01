#ifndef API_H
#define API_H

/*
*   Integrantes:
*       Rossi Nahuel
*       Ruiz Ezequiel Pablo
*/

/*      
*               TADs
*/

#include "_u64.h"
#include "lado.h"

/** \struct DovahkiinP
 * Puntero al Dovahkiin. */
typedef struct DovahkiinSt  *DovahkiinP;


/*
*             Funciones
*/

/* Creador de un nuevo DovahkiinP.
 * return: un DovahkiinP vacío.
 */
DovahkiinP NuevoDovahkiin(void);

/* Destructor de un DovahkiinP.
 * dova El dova a destruir.
 * return: devuelve 1 si no hubo errores, 0 en caso contrario.
 * note: Por la cátedra se indica un retorno distinto en caso de error, pero 
 * en la implementación no surgen casos en lo que esto pueda suceder.
 */
int DestruirDovahkiin(DovahkiinP dova);

/* Establece un nodo como fuente del network.
 * dova  El dova en el que se trabaja.
 * s     El nombre del nodo.
 * pre: dova debe ser un DovahkiinP no nulo.
 */
void FijarFuente(DovahkiinP dova, u64 x);

/* Establece un nodo como resumidero del network.
 * dova  El dova en el que se trabaja.
 * t     El nombre del nodo.
 * pre: dova debe ser un DovahkiinP no nulo.
 */
void FijarResumidero(DovahkiinP dova, u64 x);

/* Imprime por la salida estandar el nombre del nodo que es fuente.
 * Imprime por pantalla:
 * Fuente: s 
 * Donde s es el nodo que estamos conciderando como fuente. Este es el unico 
 * caso donde la fuente se imprimira con su nombre real y no con la letra s. 
 * dova  El dova en el que se trabaja.
 * pre: dova debe ser un DovahkiinP no nulo.
 * return:  -1 si la fuente no esta fijada.
 *          0 caso contrario e imprime por pantalla.
 */
int ImprimirFuente(DovahkiinP dova);

/* Imprime por la salida estandar el nombre del nodo que es resumidero.
 * Imprime por pantalla:
 * Resumidero: t 
 * Donde t es el nodo que estamos conciderando como resumidero. Este es el unico 
 * caso donde el resumidero se imprimira con su nombre real y no con la letra t. 
 * dova  El dova en el que se trabaja.
 * pre: dova debe ser un DovahkiinP no nulo.
 * return:  -1 si el resumidero no esta fijado.
 *          0 caso contrario e imprime por pantalla.
 */
int ImprimirResumidero(DovahkiinP dova);

/* Lee un lado desde la entrada estandar.
 * Lee una linea desde Standar Input que representa un lado y
 * devuelve el elemento de tipo Lado que lo representa si la linea es valida, 
 * sino devuelve el elemento LadoNulo. 
 * Cada linea es de la forma x y c, siendo todos u64 representando el lado xy 
 * de capacidad c. 
 * return:  Un lado legal con los datos leidos.
 *          LadoNulo si la linea leida no es valida.
 */
Lado LeerUnLado(void);

/* Carga un lado no nulo en un DovahkiinP.
 * Se utilizan los datos que contiene el lado para crear las estructuras de los
 * nodos y cargarlos en el netwrok, y se establecen como vecinos para crear la
 * arista que los relaciona.
 * dova  El dova en el que se trabaja.
 * edge  El lado a cargar.
 * pre: dova Debe ser un DovahkiinP no nulo.
 * return:  1 si no hubo problemas.
 *          0 caso contrario.
 */
int CargarUnLado(DovahkiinP dova, Lado L);

/* Preprocesa el DovahkiinP para empezar a buscar caminos aumentantes. 
 * Aqui se debe chequear y preparar todo lo que sea necesario para comenzar
 * a buscar caminos aumentantes. 
 * Por el momento solo hace falta chequear que esten seteados s y t, 
 * y que estos nodos existen en el network.
 * dova  El dova en el que se trabaja.
 * pre: dova Debe ser un DovahkiinP no nulo.
 * return:  1 si los preparativos fueron exitosos.
 *          0 caso contrario.
 */
int Prepararse(DovahkiinP dova);

/* Actualiza las distancias haciendo una busqueda BFS-FF.
 * Se reinician todas las distancias a nulo y se comienzan a actualizar a partir
 * del nodo fuente 's' utilizando BFS-FF, hasta encontrarse con el
 * nodo resumidero 't'. 
 * Si se alcanza 't' entonces implica un camino aumentante. Caso contrario, el
 * flujo actual es maximal y los nodos que fueron actualizados conforman el
 * corte minimal.
 * dova  El dova en el que se trabaja.
 * pre: dova Debe ser un DovahkiinP no nulo.
 * return:  1 si existe un camino aumentante entre 's' y 't'. 
 *          0 caso contrario.
 */
int ActualizarDistancias(DovahkiinP dova);

/* Hace una busqueda DFS-FF de un camino aumentante de menor longitud.
 * Solo se utilizan los nodos que tengan su distancia actualizada.
 * El ultimo nodo agregado al camino solo agrega a otro nodo si este ultimo 
 * tiene una distancia +1 que el, y si se puede aumentar (o disminuir) flujo 
 * entre ellos.
 * dova  El dova en el que se trabaja.
 * pre: dova Debe ser un DovahkiinP no nulo.
 * return:  1 si llega a 't'. 
 *          0 caso contrario.
 */
int BusquedaCaminoAumentante(DovahkiinP dova);

/* Aumenta el flujo del network.
 * Actualiza el flujo en el network de  dova sobre el camino aumentante 
 * encontrado. La actualizacion es por el maximo aumento de flujo que se pueda 
 * enviar por ese camino, teniendo en cuenta flujos anteriores.
 * dova  El dova en el que se trabaja.
 * pre: dova Debe ser un DovahkiinP no nulo.
 *      Se busco y encontro un camino aumentante que todavia no se ha usado para
 *      actualizar el flujo.
 * return:  Valor por el cual se aumenta el flujo, si no hubo errores.
 *          0 si hubo error o no se cumple la precondicion de camino aumentante.
 */
u64 AumentarFlujo(DovahkiinP dova);

/* Idem AumentarFlujo() pero tambien imprime el camino por el Estandar Output.
 * Imprime el camino con el formato: 
 * camino aumentante #:
 * t;x_r;...;x_1;s: <cantDelIncremento>
 * Donde '#' es el numero del camino aumentante, ';' se usa en caminos forward 
 * y '>' en backward.
 * dova  El dova en el que se trabaja.
 * pre: dova Debe ser un DovahkiinP no nulo.
 * return:  Valor por el cual se aumenta el flujo, si no hubo errores.
 *          0 si hubo error o no se cumple la precondicion de camino aumentante.
 */
u64 AumentarFlujoYTambienImprimirCamino(DovahkiinP dova);

/* Imprime por Estandar Output el Flujo hasta el momento. 
 * Imprime con el formato: 
 * Flujo �:
 * Lado x_1,y_2: <FlujoDelLado>
 * Donde � es "maximal" si el flujo es maximal o "no maximal" caso contrario.
 * dova  El dova en el que se trabaja.
 * pre: dova Debe ser un DovahkiinP no nulo.
 */
void ImprimirFlujo(DovahkiinP dova);

/* Imprime por Estandar Output el valor del Flujo calculado hasta el momento. 
 * Imprime con el formato: 
 * Valor del flujo �: <ValorDelFlujo> 
 * Donde � es "maximal" si el flujo es maximal o "no maximal" caso contrario.
 * dova  El dova en el que se trabaja.
 * pre: dova Debe ser un DovahkiinP no nulo.
 */
void ImprimirValorFlujo(DovahkiinP dova);

/* Imprime por Estandar Output un corte minimal y su capacidad.
 * La capacidad del corte minimal es igual a la suma de todas las capicades
 * de las aristas que van desde el corte a su complemento. Este valor debe
 * ser igual al valor del flujo maximal. Aquí no se compara dicha igualdad, pero
 * si se calcula la capacidad del corte como se enuncia.
 * Imprime con el formato:
 * Corte Minimial: S = {s,x_1,...}
 * Capacidad: <Capacidad>
 * Donde � es "maximal" si el flujo es maximal o "no maximal" caso contrario.
 * dova  El dova en el que se trabaja.
 * pre: dova Debe ser un DovahkiinP no nulo. 
        Debe existir corte minimal. ie, debe haberse llegado a flujo maximal.
 */  
void ImprimirCorte(DovahkiinP dova);

#endif
