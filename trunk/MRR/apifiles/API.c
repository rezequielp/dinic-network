#include <bool.h>
#include <stdio.h>
#include <stdlib.h>
#include "auxlib/uthash/uthash.h"
#include "auxlib/queue&stack/queue.h"
#include "API.h"


#define NOT_USED  -1    /* Constante para verificar si se actualizo la distancia de un elem*/


/*Flags*/
#define MAXFLOW         0b00001000
#define SOURCE          0b00000100
#define SINK            0b00000010
#define PATHUSED        0b00000001
/*Macros*/
#ifndef MACRO_FLAG
#define SET_FLAG(f) network->flags |= f
#define UNSET_FLAG(f) network->flags &= ~f
#define CLEAR_FLAG() 0b00000000
#define IS_SET_FLAG(f) (network->flags & f) > 0
#endif

typedef struct LadoSt{
    u64 x;
    u64 y;
    u64 c;
};

typedef struct NetworkSt{
    u64 node;                   /* key */
    Edges edges;                /* value */
    u64 level;                  /* nivel de distancia */
    UT_hash_handle hh;          /* makes this structure hashable */
} Network;

typedef struct NetworkYFlujo{
    Network *net;       /* Network de los nodos para acceder a las aristas */
    u64 flow;           /* Valor del flujo del network */
    u64 source;         /* Vertice fijado como fuente */
    u64 sink;           /* Vertice fijado como resumidero */
    Abb cut;            /* TODO ver por que era abb y no pila o cola*/ /*Un abb de punteros a nodos para el corte*/
    queue path;         /* Camino en forma de un flujo de caracteres */
    int flags;          /* Flags de estado, explicados en la seccion de #define */
    u64 tempFlow;       /* Cant de flujo a aumentar (cap min del camino actual) */
} DovahkiinSt;


static void network_elem_destroy(Network elem);


/*Devuelve un puntero a la St o Null en caso de error*/
DovahkiinP NuevoDovahkiin(){
    DovahkiinP network;
    
    network = (DovahkiinP) malloc (sizeof(DovahkiinSt));
    assert(network!=NULL);
    
    network->net = NULL;
    network->flow = 0;
    network->cut = NULL /*TODO*/;
    network->path = NULL;
    network->flags = CLEAR_FLAG();
    network->tempFlow = 0;

    return network;
}

/*Destruye D, devuelve 1 si no hubo errores, 0 en caso contrario*/
int DestruirDovahkiin(Dovahkiin network){
    Network hTable = NULL;  /*hash table que contiene todo el network*/
    Network elem = NULL;    /*el i-esimo elemento de la hash table*/
    Network eTmp = NULL;    /*el i-esimo + 1 elemento de la hash table*/
    
    assert(network!=NULL);
    
    hTable = network->net;
    HASH_ITER(hTable->hh, hTable, elem, eTmp){
        HASH_DEL(hTable, elem); /*elimina la referencia del node sobre la hash table*/
        network_elem_destroy(elem);
   }
    
    /*TODO falta liberar cut y path*/
    free(network);
}

void network_elem_destroy(Network elem){
    assert(elem != NULL);
    edges_destroy(elem->edges);
    free(elem);
}



/*Setea al vertice s como fuente*/
void FijarFuente(DovahkiinP network, u64 s){
    assert(network != NULL);
    network->s = s;
}

/*Setea al vertice t como resumidero*/
void FijarResumidero(DovahkiinP network, u64 t){
    assert(network !*/
    F= NULL);
    network->t = t;
}

/*Si la fuente NO esta fijada devuelve -1, sino 0 e imprime por pantalla:
Fuente: s
Donde s es el vertice que estamos conciderando como fuente.
Este es el unico caso donde la fuente se imprimira con su nombre real y no con la letra s*/
int ImprimirFuente(DovahkiinP network){
    int result = -1;
    assert(network != NULL);
    if(IS_SET_FLAG(SOURCE)){
        printf("Fuente: %u64\n", network->s);
        result=0;
    }
    return result;
}

/*Si el Resumidero NO esta fijada devuelve -1, sino 0 e imprime por pantalla:
Resumidero: t
Donde x es el vertice que estamos conciderando como Resumidero.
Este es el unico caso donde el resumidero se imprimira con su nombre real y no con la letra t*/
int ImprimirResumidero(DovahkiinP D){
    int result = -1;
    assert(network != NULL);
    if(IS_SET_FLAG(SINK)){
        printf("Resumidero: %u64\n", network->s);
        result=0;
    }
    return result;
}

/*Lee una linea desde Standar Imput que representa un lado y
devuelve el elemento de tipo Lado que lo representa si la linea es valida, 
sino devuelve el elemento LadoNulo.
Cada linea es de la forma x y c, siendo todos u64 representando el lado xy de capacidad c.*/
Lado LeerUnLado(){
 /*TODO PARSER*/   
}

/*Carga un lado L en D. Retorna 1 si no hubo problemas y 0 caso contrario.*/
int CargarUnLado(DovahkiinP D, Lado L){
 /*TODO PARSER*/   
}

/*Preprocesa el Dovahkiin para empezar a buscar caminos aumentantes. Debe chequear que esten
seteados s y t. Devuelve 1 si puede preparar y 0 caso contrario*/
int Prepararse(Dovahkiin network){
    assert(network != NULL);
    return (IS_SET_FLAG(SINK) && IS_SET_FLAG(SOURCE))
}
    
/*Actualiza haciendo una busqueda BFS FF. Devuelve 1 si existe un camino aumentante entre s y t,
0 caso contrario*/
int ActualizarDistancias(Dovahkiin network){
    
    Queue q, qNext, qAux;        /*Colas para el manejo de los niveles. 
                                 *q = actual, qNext = siguiente, qAux = auxiliar para swap*/
    Network elem = NULL;        /* Elementos de la cola q en la cual se itera.*/
    Network neig = NULL;        /* Vecinos de elem*/
    Network k = NULL;
    FNode i = NULL;
    BNode j = NULL;
    int lvl = 0;                /* Distancia de los elementos de qNext*/
     ;
    assert(network != NULL);
    
    q = queue_create();
    qNext = queue_create();
    
    /* Reset de distancias de la corrida anterior*/
    for(k=network->net; k != NULL; k=k->hh.next){
        k->level = NOT_USED;
    }
    /*seteo Source en nivel 0*/
    HASH_FIND(network->net, &(network->source), elem);
    queue_enqueue(q, elem)
    elem->level = lvl;
    lvl++;
    
    while(!queue_isEmpty(q) && i != network->sink){
        /*agrego vecinos de head a qNext*/        
        elem = queue_head(q);
        
        /*Busqueda Forward*/
        do{
            i = edge_getFNext(i);
            if(i!=NULL && edge_getCap(i) > edge_getFFlow(i)){  /*se puede mandar flujo*/
                HASH_FIND(network->net, edge_getName(i), neig);
                assert(neig != NULL);
                if(neig->level == NOT_USED){
                    neig->level=lvl;
                    Queue_enqueue(qNext, neig);
                }
            }
        }while(i != NULL && i != network->sink);
        
        
        do{
            j = edge_getBNext(j);
            if(i!=NULL && 0 < edge_getBFlow(j)){  /*se puede mandar flujo*/
                HASH_FIND(network->net, edge_getName(j), neig);
                assert(neig != NULL);
                if(neig->level == NOT_USED){
                    neig->level=lvl;
                    Queue_enqueue(qNext, neig);
                }
            }
        }while(j != NULL);

        queue_dequeue(q);
        /* Se terminaron los nodos de este nivel, se pasa al siguiente*/
        if(queue_isEmpty(q)){
            qAux = q;
            q = qNext;
            qNext = qAux;
            lvl++;
        }
    }
    queue_destroy(q);
    queue_destroy(qNext);

    return (elem);
}







/*Hace una busqueda FF DFS usando las etiquetas de ActualizarDistancia(). Devuelve 1 si llega a t,
0 caso contrario.*/
int BusquedaCaminoAumentante(Dovahkiin network);

/*Precondicion: (BusquedaCaminoAumentante()==1) que todavia no haya aumentado el flujo.
(Igual se tiene que checkear). 
Aumenta el flujo.
Debe devolver el valor del flujo aumentado si no hubo promblemas, 0 caso contrario(inclusive !precondicion).*/
u64 AumentarFlujo(Dovahkiin network);

/*Idem AumentarFlujo() pero tambien imprime el camino con el formato:
camino aumentante #:
t;x_r;...;x_1;s: <cantDelIncremento>
Donde # es el numero del camino aumentante, ";" se usa en caminos forward y ">" en backward.*/
AumentarFlujoYTambienImprimirCamino(Dovahkiin network);

/*Imprime el Flujo hasta el momento con el formato:
Flujo ¢:
Lado x_1,y_2: <FlujoDelLado>
Donde ¢ es "maximal" si el flujo es maximal o "no maximal" caso contrario*/
void ImprimirFlujo(Dovahkiin network);

/*Debe imprimir el valor del flujo con el formato
Valor del flujo ¢: <ValorDelFlujo> 
Donde ¢ es "maximal" si el flujo es maximal o "no maximal" caso contrario*/
void ImprimirValorFlujo(Dovahkiin network);

/*Imprime un corte minimal y su capacidad con el formato:
Corte minimial: S = {s,x_1,...}
Capacidad: <Capacidad>*/
void ImprimirCorte(Dovahkiin network);