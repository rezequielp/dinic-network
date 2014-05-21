#include <bool.h>
#include <stdio.h>
#include <stdlib.h>
#include "auxlib/uthash/uthash.h"
#include "auxlib/queue&stack/queue.h"
#include "API.h"


#define NOT_USED  -1    /* Constante para verificar si se actualizo la distancia de un elem*/

/*Flags*/
#define SINK_REACHED    0b00010000      /* Tambien implica si hay corte*/
#define MAXFLOW         0b00001000
#define SOURCE          0b00000100
#define SINK            0b00000010
#define PATHUSED        0b00000001
/*Macros para manejo de flags*/
#ifndef MACRO_FLAG
#define SET_FLAG(f) network->flags |= f
#define UNSET_FLAG(f) network->flags &= ~f
#define CLEAR_FLAG() 0b00000000
#define IS_SET_FLAG(f) (network->flags & f) > 0
#endif

/* Estructura pedida en la API, la usamos solo para leer y cargar los lados*/
typedef struct LadoSt{
    u64 x;
    u64 y;
    u64 c;
};

typedef struct NetworkSt{
    u64 x;                      /* hash key - vertice x*/
    Nbrhd nbrs;                  /* hash value - vecinos del vertice x*/
    u64 level;                  /* nivel de distancia del vertice x*/
    UT_hash_handle hhNet,hhCut; /* makes this structure hashable */
} Network;

typedef struct NetworkYFlujo{
    Network *net;       /* Network de los vertices para acceder a las aristas */
    u64 flow;           /* Valor del flujo del network */
    u64 source;         /* Vertice fijado como fuente (s) */
    u64 sink;           /* Vertice fijado como resumidero (t) */
    Network *cut;      /* Corte (nos aprovechamos para operar como network auxiliar)*/
    Stack path;         /* Camino de vertices, de s a t */
    int flags;          /* Flags de estado, explicados en la seccion de #define */
} DovahkiinSt;


static void network_elem_destroy(Network elem);


/*Devuelve un puntero a la St o Null en caso de error*/
DovahkiinP NuevoDovahkiin(){
    DovahkiinP network;
    
    network = (DovahkiinP) malloc (sizeof(DovahkiinSt));
    assert(network!=NULL);
    
    network->net = NULL;
    network->flow = 0;
    network->cut = NULL;
    network->path = NULL;
    network->flags = CLEAR_FLAG();

    return network;
}

/*Destruye D, devuelve 1 si no hubo errores, 0 en caso contrario*/
int DestruirDovahkiin(Dovahkiin network){
    Network hTable = NULL;  /*hash table que contiene todo el network*/
    Network elem = NULL;    /*el i-esimo elemento de la hash table*/
    Network eTmp = NULL;    /*el i-esimo + 1 elemento de la hash table*/
    
    assert(network!=NULL);
    
    hTable = network->net;
    HASH_ITER(hTable->hhNet, hTable, elem, eTmp){
        HASH_DEL(hTable, elem); /*elimina la referencia del x sobre la hash table*/
        network_elem_destroy(elem);
   }
    
    /*TODO falta liberar cut y path*/
    free(network);
}

void network_elem_destroy(Network elem){
    assert(elem != NULL);
    nbrhd_destroy(elem->nbrs);
    free(elem);
}



/*Setea al vertice s como fuente*/
void FijarFuente(DovahkiinP network, u64 s){
    assert(network != NULL);
    network->source = s;
}

/*Setea al vertice t como resumidero*/
void FijarResumidero(DovahkiinP network, u64 t){
    assert(network != NULL);
    network->sink = t;
}

/*Si la fuente NO esta fijada devuelve -1, sino 0 e imprime por pantalla:
Fuente: s
Donde s es el vertice que estamos conciderando como fuente.
Este es el unico caso donde la fuente se imprimira con su nombre real y no con la letra s*/
int ImprimirFuente(DovahkiinP network){
    int result = -1;
    assert(network != NULL);
    if(IS_SET_FLAG(SOURCE)){
        printf("Fuente: %u64\n", network->source);
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
        printf("Resumidero: %u64\n", network->sink);
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
/* TODO modularizar los whiles*/
int ActualizarDistancias(Dovahkiin network){
    
    Queue q, qNext, qAux;       /*Colas para el manejo de los niveles. 
                                 *q = actual, qNext = siguiente, qAux = auxiliar para swap*/
    Network elem = NULL;        /* Elementos de la cola q en la cual se itera.*/
    Network nbr = NULL;         /* Vecinos de elem*/
    Network hNet, hCut = NULL;  /* Punteros directos a las tablas hash, para codigo mas legible*/
    Network k = NULL;           /* usado para iterar en el reseteo de las distancias*/
    u64 i, j = NULL;            /* usados para iterar en la actualizacion de las distancias*/
    int lvl = 0;                /* Valor de distancia para los siguientes elementos (qNext)*/

    assert(network != NULL);
    
    /* preparacion de las cosas que voy a usar*/
    UNSET_FLAG(SINK_REACHED);
    HASH_CLEAR(hhCut, network->cut);    /*antes de empezar se limpia el corte*/
    q = queue_create();
    qNext = queue_create();
    hNet = network->net;
    hCut = network->cut;
    
    /* Reset de distancias de la corrida anterior*/
    for(k=hNet; k != NULL; k=k->hhNet.next){
        k->level = NOT_USED;
    }
    /*comienzo a actualizar las distancias*/
    /*seteo Source en nivel 0*/
    HASH_FIND(hNet->hhNet, hNet, &(network->source), sizeof(network->source), elem);
    queue_enqueue(q, elem);
    elem->level = lvl;
    lvl++;
    
    /*actualizacion de distancias por BFS */
    while(!queue_isEmpty(q) && IS_SET_FLAG(SINK_REACHED)){
        elem = queue_head(q);
        
        /*Busqueda Forward*/
        i = nbrhd_getNext(elem->nbrs, i, FWD);
        while(IS_SET_FLAG(SINK_REACHED) && i != NULL){
            if(nbrhd_getCap(elem->nbrs, i, FWD) > nbrhd_getFlow(elem->nbrs, i, FWD)){  /*se puede mandar flujo*/
                HASH_FIND(hNet->hhNet, hNet, &i, sizeof(i), nbr);
                assert(nbr != NULL);
                if(nbr->level == NOT_USED){
                    nbr->level=lvl;
                    Queue_enqueue(qNext, nbr);
                }
            }
            if(i == network->sink)  /*llego a t*/
                SET_FLAG(SINK_REACHED);
            else
                i = nbrhd_getNext(elem->nbrs, i, FWD);
        }
        
        /*Busqueda backward*/
        j = nbrhd_getNext(elem->nbrs, j, BWD);
        while(IS_SET_FLAG(SINK_REACHED) && j != NULL){
            if(0 < nbrhd_getFlow(elem->nbrs, j, BWD)){  /*se puede mandar flujo*/
                HASH_FIND(hNet->hhNet, hNet, &j, sizeof(j), nbr);
                assert(nbr != NULL);
                if(nbr->level == NOT_USED){
                    nbr->level=lvl;
                    Queue_enqueue(qNext, nbr);
                }
            }
            j = nbrhd_getNext(elem->nbrs, j, BWD);
        };

        /* Agregamos el elemento al posible corte*/
        HASH_ADD(hCut->hhCut, hCut, hCut->x, sizeof(hCut->x), queue_head(q));
        queue_dequeue(q);
        /* Se terminaron los vertices de este nivel, se pasa al siguiente*/
        if(queue_isEmpty(q)){
            qAux = q;
            q = qNext;
            qNext = qAux;
            lvl++;
        }
    }

    queue_destroy(q);
    queue_destroy(qNext);
    return IS_SET_FLAG(SINK_REACHED);
}

/*Hace una busqueda FF DFS usando las etiquetas de ActualizarDistancia(). Devuelve 1 si llega a t,
0 caso contrario.*/
int BusquedaCaminoAumentante(Dovahkiin network){
    
    Network elem = NULL;
    
    assert(network != NULL);
    
    /*preparacion de todas las cosas que uso para buscar un camino aumentante*/
    stack_destroy(network->path);
    HASH_FIND(network->net->hhNet, network->net, &(network->source), sizeof(network->source), elem);
    stack_push(network->path, elem);
    
    if (IS_SET_FLAG(SINK_REACHED) && !IS_SET_FLAG(PATHUSED)){
        while(elem->x != network->sink && !stack_isEmpty(network->path)){
            elem = devolveme_el_siguiente_VALIDO(elem); /*TODO debe cumplir condicion de distancia*/
            if (elem != NULL){
                stack_push(network->path, elem);
            }else{
                stack_pop(network->path);
                elem = stack_top(network->path); /*va a ser NULL si la pila esta vacia*/
            }
        }
        SET_FLAG(PATHUSED);
    }
    
    return elem == network->sink;
}

/*Precondicion: (BusquedaCaminoAumentante()==1) que todavia no haya aumentado el flujo.
(Igual se tiene que checkear). 
Aumenta el flujo.
Debe devolver el valor del flujo aumentado si no hubo promblemas, 0 caso contrario(inclusive !precondicion).*/
u64 AumentarFlujo(Dovahkiin network){
    
    u64 pflow = 0;  /*flujo a enviar por el camino aumentante*/ 
    Network x, y = NULL;
    Stack pathAux;
    
    assert(network != NULL);
    
    /*ATTENTION ambos while son identicos! salvo por la ultima linea. modularizar!
     *  (hacer algo como next_nbrhd_xy(path, pathAux, x, y)*/
    if (!IS_SET_FLAG(PATHUSED)){
        pflow = INFINITE;   /*TODO declarar INFINITE (se puede hacer chanchadas como inf = 1/0) */
        while (!stack_isEmpty(network->path){
            x = stack_pop(network->path);
            stack_push(pathAux, x);
            if (x->x != network->sink){  /*si llego a t, ya no hay mas flujos que comparar*/
                y = stack_top(network->path);
                pflow = min(pflow, nbrhd_getFlow(x->nbrs, y->x, NULL));    /*TODO hacer min de u64*/
            }
        }
        
        while (!stack_isEmpty(pathAux){
            x = stack_pop(pathAux);
            stack_push(network->path, x);
            if (x->x != network->sink){  /*si llego a t, ya no hay mas flujos que aumentar*/
                y = stack_top(pathAux);
                nbrhd_increaseFlow(x->nbrhd, y->x, pflow);/*WARNING si es BWD, se disminuiye!*/
        }
    }
    return pflow;
}

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