#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "auxlibs/uthash/uthash.h"
#include "auxlibs/stack/stack.h"
#include "auxlibs/bstring/bstrlib.h"
#include "API.h"

/*
Descomentar uno u el otro dependiendo el tipo de int a usar.
*/
typedef uint64_t u64;
/*typedef uint_fast64_t u64;*/


#define NOT_USED -1    /* Valor nulo de distancia si el elem no se uso*/


/*
 *                  Flags de permisos y estados
 */
#ifndef MACRO_TOOLS
#define SINK_REACHED    0b00010000      /*Se llego a t. Tambien implica corte
                                          seteo solo en actualizarDistancias()*/
#define MAXFLOW         0b00001000      /*Imprimir flujo maximal*/
#define SOURCE          0b00000100      /*Fuente fijada*/
#define SINK            0b00000010      /*Resumidero fijado*/
#define PATHUSED        0b00000001      /*Camino usado para aumentar flujo*/
/*
 *          Macros para manejo de flags
 */
#define SET_FLAG(f) network->flags |= f
#define UNSET_FLAG(f) network->flags &= ~f
#define CLEAR_FLAG() 0b00000000
#define IS_SET_FLAG(f) (network->flags & f) > 0
#endif

/*
 *          Estructuras
 */
typedef struct NetworkSt{
    u64 x;                      /* hash key - vertice x*/
    Nbrhd nbrs;                 /* hash value - vecinos del vertice x*/
    int level;                  /* nivel de distancia del vertice x*/
    UT_hash_handle hhNet,hhCut; /* makes this structure hashable */
} Network;

typedef struct NetworkYFlujo{
    Network *net;       /* Network de los vertices para acceder a las aristas */
    u64 flow;           /* Valor del flujo del network */
    u64 source;         /* Vertice fijado como fuente (s) */
    u64 sink;           /* Vertice fijado como resumidero (t) */
    Network *cut;       /* Corte (se aprovecha como network auxiliar)*/
    Stack path;         /* Camino de vertices, de s a t */
    u64 pCounter;       /* Contador para la cantidad de caminos*/
    int flags;          /* Flags de estado, explicados en la seccion #define */
} DovahkiinSt;


/*
 *      Funciones estaticas
 */
static u64 get_pathFlow(DovahkiinP network, bool print);
static void increment_flow(Dovahkiin network, u64 pflow);
static void network_destroy(Network net);


/*Devuelve un puntero a la St o Null en caso de error */
DovahkiinP NuevoDovahkiin(){
    DovahkiinP network;
    
    network = (DovahkiinP) malloc (sizeof(DovahkiinSt));
    assert(network!=NULL);
    
    network->net = NULL;
    network->flow = u64_new(); /*TODO*/
    network->source = u64_new();
    network->sink = u64_new();
    network->cut = NULL;
    network->path = NULL;
    network->flags = CLEAR_FLAG();
    network->pCounter = u64_new(); 
    return network;
}


/*Constructor de Network */
Network *network_create(u64 x){
    Network *node;
    
    node = (Network*) malloc(sizeof(Network));
    assert(node!=NULL);
    
    u64 x = x;
    node->nbrs = NULL;
    int level = NOT_USED; 
    
    return node;
}


/*Destruye D, devuelve 1 si no hubo errores, 0 en caso contrario */
int DestruirDovahkiin(DovahkiinP network){
    assert(network!=NULL);
    
    network_destroy(network->net);  /*esto tambien libera el corte*/
    u64_destroy(network->flow);
    u64_destroy(network->source);
    u64_destroy(network->sink);
    if (network->path != NULL)
        stack_destroy(network->path);
    
    u64_destroy(network->pCounter);
    
    free(network);
}


/*Destruye un elemento del network */
void network_destroy(Network net){
    Network elem = NULL;    /*el i-esimo elemento de la hash table*/
    Network eTmp = NULL;    /*el i-esimo + 1 elemento de la hash table*/
    
    HASH_ITER(net->hhNet, net, elem, eTmp){
        HASH_DEL(net, elem); /*elimina la referencia sobre la hash table*/
        u64_destroy(elem->x);
        nbrhd_destroy(elem->nbrs);
        u64_destroy(elem->level);
        free(elem);
    }
}


/*Setea al vertice s como fuente */
void FijarFuente(DovahkiinP network, u64 s){
    assert(network != NULL);
    network->source = u64_copy(s);
}


/*Setea al vertice t como resumidero */
void FijarResumidero(DovahkiinP network, u64 t){
    assert(network != NULL);
    network->sink = u64_copy(t);
}


/*Si la fuente NO esta fijada devuelve -1, sino 0 e imprime por pantalla:
Fuente: s
Donde s es el vertice que estamos conciderando como fuente.
Este es el unico caso donde la fuente se imprimira con su nombre real y 
no con la letra s */
int ImprimirFuente(DovahkiinP network){
    int result = -1;
    char * cStr;
    bstring bStr;
    
    assert(network != NULL);
    
    if(IS_SET_FLAG(SOURCE)){
        u64_toString(network->sink, bStr):
        cStr = bstr2cstr(bStr)
        printf("Fuente: %s\n", cStr);
        bcstrfree(cStr);
        bdestroy(bStr);
        result=0;
    }
    free(cStr);
    return result;
}


/*Si el Resumidero NO esta fijada devuelve -1, sino 0 e imprime por pantalla:
Resumidero: t
Donde x es el vertice que estamos conciderando como Resumidero.
Este es el unico caso donde el resumidero se imprimira con su nombre real y 
no con la letra t */
int ImprimirResumidero(DovahkiinP D){
    int result = -1;
    bstring bStr;
    
    assert(network != NULL);
    
    if(IS_SET_FLAG(SINK)){
        u64_toString(network->sink, bStr):
        cStr = bstr2cstr(bStr)
        printf("Resumidero: %s\n", cStr);
        bcstrfree(cStr);
        bdestroy(bStr);
        result=0;
    }
    return result;
}


/*Lee una linea desde Standar Imput que representa un lado y
devuelve el elemento de tipo Lado que lo representa si la linea es valida, 
sino devuelve el elemento LadoNulo.
Cada linea es de la forma x y c, siendo todos u64 representando el lado xy 
de capacidad c. */
Lado LeerUnLado(){
    Lado lado = NULL;
    u64 x, y, c = NULL;
    lexer *input;   /*analizador lexico por lineas de un archivo*/
    int clean;      /*Indica si no se encontro basura al parsear*/
   
    /*construyo el lexer sobre la entrada estandar*/
    input = lexer_new(stdin);

    if (input! = NULL){
        /*Leo los lados mientras no llegue a un fin de archivo o haya ocurrido
          algun error*/
        if (!lexer_is_off(input)){
            /*se parsea un lado*/
            lado = parse_edge(input, x, y, c);
            /*se corre el parseo hasta la siguiente linea (o fin de archivo)*/
            clean = parse_nextLine(input);
        }
        lexer_destroy(input);
    }
    
    return result;
}


/*Carga un edge al network. Retorna 1 si no hubo problemas y 0 caso contrario.*/
int CargarUnLado(DovahkiinP network, Lado edge){
    Network nodeX = NULL;
    Network nodeY = NULL;
    Network hNet = NULL
    u64 x, y = NULL;
    int result = 0;
    
    assert(network != NULL);
    
    hNet = network->net;
    
    if (edge != NULL){
    
        x = lado_getX(Lado edge);
        y = lado_getY(Lado edge);
        
        /* cargo el nodo 'x'*/
        HASH_FIND(hNet->hhNet, hNet, &(x), sizeof(x), nodeX);
        if (nodeX == NULL){
            nodeX = network_create(x);
            HASH_ADD(hNet->hhNet, hNet, hNet->x, sizeof(hNet->x), nodeX);
        }
        
        /*cargo el nodo 'y'*/
        HASH_FIND(hNet->hhNet, hNet, &(y), sizeof(y), nodeY);
        if (nodeY == NULL){
            nodeY = network_create(y);
            HASH_ADD(hNet->hhNet, hNet, hNet->x, sizeof(hNet->x), nodeY);
        }
        
        /*se establecen como vecinos*/
        nbrhd_addEdge(nodeX->nbrs, nodeY->nbrs, Lado edge);
        result = 1;
    }
    
    return result;
}


/*Preprocesa el Dovahkiin para empezar a buscar caminos aumentantes. 
Debe chequear que esten seteados s y t. 
Devuelve 1 si puede preparar y 0 caso contrario*/
int Prepararse(Dovahkiin network){
    int status=0;
    assert(network != NULL);
    if(IS_SET_FLAG(SINK) && IS_SET_FLAG(SOURCE)){
        HASH_FIND(network->net->hhNet, network->net, &(network->source), sizeof(network->source), src);
        HASH_FIND(network->net->hhNet, network->net, &(network->sink), sizeof(network->sink), snk);
        if(src != NULL && snk != NULL)
            status = 1;
    }
    return 
}


/*Actualiza haciendo una busqueda BFS FF. Devuelve 1 si existe un camino aumentante entre s y t,
0 caso contrario*//* TODO modularizar los whiles*/
int ActualizarDistancias(DovahkiinP network){
    
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
            if(nbrhd_getCap(elem->nbrs, i) > nbrhd_getFlow(elem->nbrs, i)){  /*se puede mandar flujo*/
                HASH_FIND(hNet->hhNet, hNet, &i, sizeof(i), nbr);
                assert(nbr != NULL);
                if(nbr->level == NOT_USED){
                    nbr->level=lvl;
                    Queue_enqueue(qNext, nbr);
                }
            }
            if(U64_i==network->sink)  /*llego a t*/
                SET_FLAG(SINK_REACHED);
            else
                i = nbrhd_getNext(elem->nbrs, i, FWD);
        }
        
        /*Busqueda backward*/
        j = nbrhd_getNext(elem->nbrs, j, BWD);
        while(IS_SET_FLAG(SINK_REACHED) && j != NULL){
            if(0 < nbrhd_getFlow(elem->nbrs, j)){  /*se puede mandar flujo*/
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
int BusquedaCaminoAumentante(DovahkiinP network){
    
    Network elem = NULL;
    
    assert(network != NULL);
    
    /*preparacion de todas las cosas que uso para buscar un camino aumentante*/
    stack_destroy(network->path);
    HASH_FIND(network->net->hhNet, network->net, &(network->source), sizeof(network->source), elem);
    stack_push(network->path, elem);
    
    if (IS_SET_FLAG(SINK_REACHED) && !IS_SET_FLAG(PATHUSED)){
        while(elem->x != network->sink && !stack_isEmpty(network->path)){
            elem = network_nextItem(elem); /*TODO debe cumplir condicion de distancia*/
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
u64 AumentarFlujo(DovahkiinP network){
    /*pflow debe ser liberado por el llamador con u64_destroy()*/
    u64 pflow;  /*flujo a enviar por el camino aumentante*/ 
    
    assert(network != NULL);
    
    if  (!IS_SET_FLAG(PATHUSED)){
        pflow = get_pathFlow(network);
        increment_flow(network, pflow);
    }
    SET_FLAG(PATHUSED)
    return pflow;
}


/*Idem AumentarFlujo() pero tambien imprime el camino con el formato:
camino aumentante #:
t;x_r;...;x_1;s: <cantDelIncremento>
Donde # es el numero del camino aumentante, ";" se usa en caminos forward y ">" en backward.*/
AumentarFlujoYTambienImprimirCamino(DovahkiinP network){
    u64 pflow;
    Network x,y;
    
    assert(network != NULL);
    
    if  (!IS_SET_FLAG(PATHUSED)){
        pflow = get_pathFlow(network);
        increment_flow(network, pflow);
        
        /*printer*/ 
        printf("camino aumentante %i:\n", network->pCounter );
        stack_resetViewer=(network->path);
        x = stack_nextItem(network->path);
        while(x != NULL){
            y = x;
            x = stack_nextItem(network->path);
            dir = nbrhd_getDir(x->nbrs, y->x); /*me fijo si son BWD/FWD*/
            assert(dir!=0);
            if(dir = FWD){
                if(x->x != network->source){
                    printf(";%u64", );      /*TODO*/
                }else{
                    printf(";s");
                }
            }else{
                printf(">%u64", );      /*TODO*/
            }
        }
        printf(": <%i>",  pflow);
        network->pCounter++;
    }
    SET_FLAG(PATHUSED)
    return pflow;
}


/*Imprime el Flujo hasta el momento con el formato:
Flujo �:
Lado x_1,y_2: <FlujoDelLado>
Donde � es "maximal" si el flujo es maximal o "no maximal" caso contrario*/
void ImprimirFlujo(DovahkiinP network){/* TODO el flujo del corte? del network? de que????? TODO*/
    Network x, y = NULL;
    assert(network != NULL);
    
    if(IS_SET_FLAG(MAXFLOW))
        printf("Flujo maximal:\n");
    else
        printf("Flujo no maximal:\n");

    stack_resetViewer=(network->path);
    x = stack_nextItem(network->path);

    while (x != NULL){
        y = x;
        x = stack_nextItem(network->path);
        printf("Lado %s,%s: %u64\n",x->x, y->x );      /*TODO*/
    }
}


/*Debe imprimir el valor del flujo con el formato
Valor del flujo �: <ValorDelFlujo> 
Donde � es "maximal" si el flujo es maximal o "no maximal" caso contrario*/
void ImprimirValorFlujo(DovahkiinP network){
    if(IS_SET_FLAG(MAXFLOW))
        printf("Valor del flujo maximal: %u64\n", network->flow);      /*TODO*/
    else
        printf("Valor del flujo no maximal: %u64\n", network->flow);      /*TODO*/
}


/*Imprime un corte minimal y su capacidad con el formato:
Corte minimial: S = {s,x_1,...}
Capacidad: <Capacidad>*/
void ImprimirCorte(Dovahkiin network){
/*Si bien la capacidad del corte minimal es igual al flujo maximal, se va a calcular el flujo
de S a su complemento. Eso es mas pesado pero sirve para propositos de debugging.
igual quedan programadas las dos formas y comentada una de ellas.*/
    Network x = NULL;
    
    printf("Corte Minimal: S ={");
    stack_resetViewer=(network->path);
    x = stack_nextItem(network->path);
    
    while (x != NULL){
        x = stack_nextItem(network->path);
        if (x->x != network->source)
            printf("%u64,",x->x);      /*TODO*/
    }
    printf("t}\n'");
    printf("Capacidad: %u64", network->flow)      /*TODO*/
    /*TODO calcular la capacidad del  flujo maximal*/
}



/* 
 *          Funciones estaticas
*/

/*Busca el maximo valor de flujo que se puede enviar por un camino sin usar*/
u64 get_pathFlow(DovahkiinP network, bool print){
    /* TODO (hacer algo como next_nbrhd_xy(path, pathAux, x, y)*/
    u64 pflow;              /*path flow, valor de retorno*/
    Network x = NULL;       /*nodo x de una arista xy*/
    Netwrok y = NULL;       /*nodo y de una arista xy*/

    assert(network != NULL);
    assert(!IS_SET_FLAG(PATHUSED));
    
    pflow = INFINITE;   /*TODO declarar INFINITE (se puede hacer chanchadas como inf = 1/0) */    
    stack_resetViewer=(network->path);
    y = stack_nextItem(network->path);
    x = stack_nextItem(network->path);
    /*busqueda del flujo maximal sobre el camino aumentante*/
    while (x != NULL){ 
        pflow = u64_min(pflow, nbrhd_getFlow(x->nbrs, y->x));
        y = x;
        x = stack_nextItem(network->path);
    }
    return pflow;
}


/*Aumenta el flujo sobre un camino aumentante*/
void increment_flow(DovahkiinP network, u64 pflow){
    Network x = NULL;       /*nodo x de una arista xy*/
    Netwrok y = NULL;       /*nodo y de una arista xy*/
    
    assert(network != NULL);
    assert(!IS_SET_FLAG(PATHUSED));
    
    stack_resetViewer=(network->path);
    y = stack_nextItem(network->path);
    x = stack_nextItem(network->path);
    
    while (x != NULL){
        /*WARNING si es BWD, disminuye!*/
        nbrhd_increaseFlow(x->nbrhd, y->x, pflow);
        y = x;
        x = stack_nextItem(network->path);
    }
}