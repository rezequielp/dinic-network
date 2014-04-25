#ifndef _API_H
#define _API_H
/*
*   Integrantes:
*       Joaquin Moine
*       Rossi Nahuel
*       Pablo Ezequiel Ruiz
*/

/*      
*               TADs
 */
typedef DovahkiinSt Dovahkiin;
typedef DovahkiinSt *DovahkiinP;
/*typedef ladoSt lado; */ /*Definida como Edge? hacer un include?*/
/*typedef ladoNuloSt ladoNulo;*/ /*Idem ladoSt*/



/*
*             Funciones
*/

/*Devuelve un puntero a la St o Null en caso de error*/
DovahkiinP NuevoDovahkiin();

/*Destruye D, devuelve 1 si no hubo errores, 0 en caso contrario*/
int DestruirDovahkiin(Dovahkiin D);

/*Setea al vertice x como fuente*/
void FijarFuente(DovahkiinP D u64 x);

/*Setea al vertice x como resumidero*/
void FijarResumidero(DovahkiinP D u64 x);

/*Si la fuente NO esta fijada devuelve -1, sino 0 e imprime por pantalla:
Fuente: x
Donde x es el vertice que estamos conciderando como fuente.
Este es el unico caso donde la fuente se imprimira con su nombre real y no con la letra s*/
int ImprimirFuente(DovahkiinP D);

/*Si el Resumidero NO esta fijada devuelve -1, sino 0 e imprime por pantalla:
Resumidero: x
Donde x es el vertice que estamos conciderando como Resumidero.
Este es el unico caso donde el resumidero se imprimira con su nombre real y no con la letra t*/
int ImprimirResumidero(DovahkiinP D);

/*Lee una linea desde Standar Imput que representa un lado y
devuelve el elemento de tipo Lado que lo representa si la linea es valida, 
sino devuelve el elemento LadoNulo.
Cada linea es de la forma x y c, siendo todos u64 representando el lado xy de capacidad c.*/
Lado LeerUnLado();

/*Carga un lado L en D. Retorna 1 si no hubo problemas y 0 caso contrario.*/
int CargarUnLado(DovahkiinP D, Lado L);

/*Preprosesa el Dovahkiin para empezar a buscar caminos aumentantes. Debe chequear que esten
seteados s y t. Devuelve 1 si puede preparar y 0 caso contrario*/
int Prepararse(Dovahkiin D);

/*Actualiza haciendo una busqueda BFS FF. Devuelve 1 si existe un camino aumentante entre s y t,
0 caso contrario*/
int ActualizarDistancias(Dovahkiin D);

/*Hace una busqueda FF DFS usando las etiquetas de ActualizarDistancia(). Devuelve 1 si llega a t,
0 caso contrario.*/
int BusquedaCaminoAumentante(Dovahkiin D);

/*Precondicion: (BusquedaCaminoAumentante()==1) que todavia no haya aumentado el flujo.
(Igual se tiene que checkear). 
Aumenta el flujo.
Debe devolver el valor del flujo aumentado si no hubo promblemas, 0 caso contrario(inclusive !precondicion).*/
u64 AumentarFlujo(Dovahkiin D);

/*Idem AumentarFlujo() pero tambien imprime el camino con el formato:
camino aumentante #:
t;x_r;...;x_1;s: <cantDelIncremento>
Donde # es el numero del camino aumentante, ";" se usa en caminos forward y ">" en backward.*/
AumentarFlujoYTambienImprimirCamino(Dovahkiin D);

/*Imprime el Flujo hasta el momento con el formato:
Flujo ¢:
Lado x_1,y_2: <FlujoDelLado>
Donde ¢ es "maximal" si el flujo es maximal o "no maximal" caso contrario*/
void ImprimirFlujo(Dovahkiin D);

/*Debe imprimir el valor del flujo con el formato
Valor del flujo ¢: <ValorDelFlujo> 
Donde ¢ es "maximal" si el flujo es maximal o "no maximal" caso contrario*/
void ImprimirValorFlujo(Dovahkiin D);

/*Imprime un corte minimal y su capacidad con el formato:
Corte minimial: S = {s,x_1,...}
Capacidad: <Capacidad>*/
void ImprimirCorte(Dovahkiin D);


/*
Definir u64
*/
#endif