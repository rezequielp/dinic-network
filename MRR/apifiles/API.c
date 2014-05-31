#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "auxlibs/uthash/uthash.h"
#include "auxlibs/stack/stack.h"
#include "auxlibs/bstring/bstrlib.h"
#include "API.h"



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
 *      Macro: iterador sobre un path
 */
#define PATH_ITER(path, x, y)                                               \
    stack_resetViewer=(path);                                               \
    y = stack_nextItem(path);                                               \
    x = stack_nextItem(path);                                               \
    for(; (x)!=NULL; (y)=(x), (x)=stack_nextItem(path))

/*
 *          Estructuras
 */
typedef struct NetworkSt{
    u64 name;                   /* hash key - nombre del vertice*/
    Nbrhd nbrs;                 /* hash value - vecinos del vertice*/
    int level;                  /* nivel de distancia del vertice*/
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
static Network *network_create(u64 n);
static void network_destroy(Network net);


/*Devuelve un puntero a la St o Null en caso de error */
DovahkiinP NuevoDovahkiin(){
    DovahkiinP network;
    
    network = (DovahkiinP) malloc (sizeof(DovahkiinSt));
    assert(network!=NULL);
    
    network->net = NULL;
    network->flow = 0;
    network->source = 0;
    network->sink = 0;
    network->cut = NULL;
    network->path = NULL;
    network->flags = CLEAR_FLAG();
    network->pCounter = 0; 
    return network;
}


/*Constructor de Network */
static Network *network_create(u64 n){
    Network *node;
    
    node = (Network*) malloc(sizeof(Network));
    assert(node!=NULL);
    
    node->name = n;
    node->nbrs = NULL;
    int level = NOT_USED; 
    
    return node;
}


/*Destruye D, devuelve 1 si no hubo errores, 0 en caso contrario */
int DestruirDovahkiin(DovahkiinP network){
    assert(network!=NULL);
    
    network_destroy(network->net);  /*esto tambien libera el corte*/
    if (network->path != NULL)
        stack_destroy(network->path);
    
    free(network);
}


/*Destruye un elemento del network */
static void network_destroy(Network net){
    Network elem = NULL;    /*el i-esimo elemento de la hash table*/
    Network eTmp = NULL;    /*el i-esimo + 1 elemento de la hash table*/
    
    HASH_ITER(hhNet, net, elem, eTmp){
        HASH_DEL(net, elem); /*elimina la referencia sobre la hash table*/
        nbrhd_destroy(elem->nbrs);
        free(elem);
    }
}


/*Setea al vertice s como fuente */
void FijarFuente(DovahkiinP network, u64 s){
    assert(network != NULL);
    network->source = s;
}


/*Setea al vertice t como resumidero */
void FijarResumidero(DovahkiinP network, u64 t){
    assert(network != NULL);
    network->sink = t;
}


/*Si la fuente NO esta fijada devuelve -1, sino 0 e imprime por pantalla:
Fuente: s
Donde s es el vertice que estamos conciderando como fuente.
Este es el unico caso donde la fuente se imprimira con su nombre real y 
no con la letra s */
int ImprimirFuente(DovahkiinP network){
    int result = -1;
    
    assert(network != NULL);
    
    if(IS_SET_FLAG(SOURCE)){
        printf("Fuente: %" PRIx64 "\n", network->source);
        result=0;
    }
    return result;
}


/*Si el Resumidero NO esta fijada devuelve -1, sino 0 e imprime por pantalla:
Resumidero: t
Donde x es el vertice que estamos conciderando como Resumidero.
Este es el unico caso donde el resumidero se imprimira con su nombre real y 
no con la letra t */
int ImprimirResumidero(DovahkiinP D){
    int result = -1;
    
    assert(network != NULL);
  
    if(IS_SET_FLAG(SOURCE)){
        printf("Resumidero: %" PRIx64 "\n", network->sink);
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
    Lado edge = NULL;
    u64 x, y, c;
    lexer *input;   /*analizador lexico por lineas de un archivo*/
    int garbage = PARSE_OK;      /*Indica si no se encontro basura al parsear*/
   
    /*construyo el lexer sobre la entrada estandar*/
    input = lexer_new(stdin);

    if (input! = NULL){
        /*Leo un lado mientras no llegue a un fin de archivo o haya ocurrido
          algun error*/
        if (!lexer_is_off(input)){
            /*se parsea un lado*/
            edge = parse_edge(input, x, y, c);
            /*se corre el parseo hasta la siguiente linea (o fin de archivo)*/
            garbage = parse_nextLine(input);
            if (edge != LadoNulo && garbage) /*habia basura, error*/
                lado_destroy(edge);
        }
        lexer_destroy(input);
    }
    
    return edge;
}


/*Carga un edge al network. Retorna 1 si no hubo problemas y 0 caso contrario.*/
int CargarUnLado(DovahkiinP network, Lado edge){
    Network nodeX = NULL;
    Network nodeY = NULL;
    Network net = NULL
    u64 x, y;
    int result = 0;
    
    assert(network != NULL);
    net = network->net;
    
    if (edge != LadoNulo){
        x = lado_getX(edge);
        /* cargo el nodo 'x', si todavia no existe*/
        HASH_FIND(hnet, net, &(x), sizeof(x), nodeX);
        if (nodeX == NULL){
            nodeX = network_create(x);
            HASH_ADD(hnet, net, net->name, sizeof(net->name), nodeX);
        }
        y = lado_getY(edge);
        /*cargo el nodo 'y', si todavia no existe*/
        HASH_FIND(hnet, net, &(y), sizeof(y), nodeY);
        if (nodeY == NULL){
            nodeY = network_create(y);
            HASH_ADD(hnet, net, net->name, sizeof(net->name), nodeY);
        }
        /*se establecen como vecinos*/
        nbrhd_addEdge(nodeX->nbrs, nodeY->nbrs, edge);
        result = 1;
    }
    
    return result;
}


/*Preprocesa el Dovahkiin para empezar a buscar caminos aumentantes. 
Debe chequear que esten seteados s y t. 
Devuelve 1 si puede preparar y 0 caso contrario*/
int Prepararse(Dovahkiin network){
    int status=0;
    Network *src = NULL;
    Network *snk = NULL;
    
    assert(network != NULL);
    network->net = net;
    if(IS_SET_FLAG(SINK) && IS_SET_FLAG(SOURCE)){
        HASH_FIND(hhNet, net, &(netwrok->source), sizeof(netwrok->source), src);
        HASH_FIND(hhNet, net, &(netwrok->sink), sizeof(netwrok->sink), snk);
        if (src != NULL && snk != NULL)
            status = 1;
    }
    return 
}


/*Actualiza haciendo una busqueda BFS-FF. 
 * Devuelve 1 si existe un camino aumentante entre s y t, 0 caso contrario*/
int ActualizarDistancias(DovahkiinP network){
    
    Queue q, qNext, qAux;       /*Colas para el manejo de los niveles. 
                                 *q = actual, qNext = siguiente, qAux = swap*/
    Network node = NULL;        /* nodo actual de 'q' en el cual se itera*/
    Network cut = NULL;        /* Puntero al corte, para codigo legible*/
    Network k = NULL;           /* iterador en el reseteo de las distancias*/
    int lvl = 0;                /* Distancia para elementos de qNext*/

    assert(network != NULL);
    
    /* preparacion de las cosas que voy a usar*/
    UNSET_FLAG(SINK_REACHED);
    cut = network->cut;
    HASH_CLEAR(hhCut, cut);    /*antes de empezar se limpia el corte*/
    q = queue_create();
    qNext = queue_create();
    
    /* Reset de distancias por corridas anteriores*/
    for(k = network->net; k != NULL; k = k->hhNet.next){
        k->level = NOT_USED;
    }
    /*seteo Source en nivel 0*/
    node = set_lvl(network->net, network->source, lvl);
    queue_enqueue(q, node);
    lvl++;
    
    /*actualizacion de distancias por BFS */
    while(!queue_isEmpty(q) && IS_SET_FLAG(SINK_REACHED)){
        node = queue_head(q);
        /*Busqueda y actualizacion de niveles de nodos Forward*/
        set_lvlNbrs(network->net, qNext, node, FWD, lvl);
        /*Busqueda y actualizacion de niveles de nodos Backward*/
        set_lvlNbrs(network->net, qNext, node, BWD, lvl);
        /* Agregamos el elemento al candidato a corte*/
        HASH_ADD(hhCut, cut, cut->name, sizeof(cut->name), queue_head(q));
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

/* Actualiza las distancias de los vecinos FWD o BWD de un nodo. 
 * Los vecinos que se actualizaron se agregan a la cola 'q'.
 * Precondicion: 
 */
static void set_lvlNbrs(Network net, Queue q, Network node, int dir, int lvl){
    Network yNode = NULL;
    canBeUsed = false;
    Nbrhd nbrs = NULL;
    u64 y; 
    
    assert(node != NULL && q != NULL);
    assert(dir == FWD || dir == BWD);
    
    nbrs = node->nbrs;
    obteined = nbrhd_getNext(nbrs, FST, dir, &y);
    while(IS_SET_FLAG(SINK_REACHED) && obteined != NONE){
        if (dir = FWD) /*chequeo si se puede usar para mandar flujo*/
            canBeUsed = nbrhd_getCap(nbrs, y) > nbrhd_getFlow(nbrs, y); 
        else
            canBeUsed = nbrhd_getFlow(nbrs, y) > 0;
    
        if (canBeUsed){ /*busco el nodo y actualizo su nivel*/
            yNode = set_lvl(net, y, lvl);
            if (yNode != NULL)
                Queue_enqueue(q, yNode);
            if(y == network->sink)  /*llego a t*/
                SET_FLAG(SINK_REACHED);
            else
                obteined = nbrhd_getNext(node->nbrs, y, dir, &y);
        }
    }
    return result;
}

/* Busca por 'name' y actualiza el valor de distancia de un nodo.
 * Retorna un puntero al nodo modificado.
 * Precondicion: 'name' debe ser un nombre de nodo existente
 */
static Network set_lvl(Network net, u64 name, int lvl){
    Network node = NULL;
    
    assert(net != NULL);
    
    HASH_FIND(hhNet, net, &name, sizeof(name), node);
    assert(node != NULL);
    if(node->level == NOT_USED)
        node->level = lvl;
    
    return node;
}

/*Hace una busqueda FF DFS usando las etiquetas de ActualizarDistancia(). 
 * Devuelve 1 si llega a t, 0 caso contrario.*/
int BusquedaCaminoAumentante(DovahkiinP network){
    U64 s, t;
    Network elem = NULL;
    
    assert(network != NULL);
    s = network->source;
    t = network->sink;
    /*preparacion de todas las cosas que uso para buscar un camino aumentante*/
    stack_destroy(network->path);
    HASH_FIND(hhNet, network->net, &(s), sizeof(s), elem);
    stack_push(network->path, elem);
    
    if (IS_SET_FLAG(SINK_REACHED) && !IS_SET_FLAG(PATHUSED)){
        while(elem->name != t && !stack_isEmpty(network->path)){
            elem = network_nextItem(elem); /*TODO debe cumplir condicion de distancia*/
            if (elem != NULL){
                stack_push(network->path, elem);
            }else{
                stack_pop(network->path);
                elem = stack_top(network->path); /* NULL si la pila esta vacia*/
            }
        }
        SET_FLAG(PATHUSED);
    }
    
    return (elem == t);
}


/*Precondicion: (BusquedaCaminoAumentante()==1) 
 * que todavia no haya aumentado el flujo. (Igual se tiene que checkear). 
 * Aumenta el flujo. 
 * Debe devolver el valor del flujo aumentado si no hubo problemas, 
 * 0 caso contrario (inclusive !precondicion).*/
u64 AumentarFlujo(DovahkiinP network){
    u64 pflow = 0;          /*flujo a enviar por el camino aumentante*/ 
    Network x = NULL;       /*nodo x de una arista xy*/
    Netwrok y = NULL;       /*nodo y de una arista xy*/
    
    assert(network != NULL);
    
    if (!IS_SET_FLAG(PATHUSED)){
        pflow = get_pathFlow(network);
        /*incremento del flujo sobre cada arista xy*/
        PATH_ITER(network->path, x, y){
            nbrhd_increaseFlow(x->nbrhd, y->name, pflow); /*por BWD disminuye!*/
        }
    }
    SET_FLAG(PATHUSED);
    return pflow;
}


/*Idem AumentarFlujo() pero tambien imprime el camino con el formato:
 * camino aumentante #:
 * t;x_r;...;x_1;s: <cantDelIncremento>
 * Donde # es el numero del camino aumentante, 
 * ";" se usa en caminos forward y ">" en backward.*/
AumentarFlujoYTambienImprimirCamino(DovahkiinP network){
    u64 pflow = 0;
    Network x = NULL;
    Network y = NULL;
    
    assert(network != NULL);
    
    if (!IS_SET_FLAG(PATHUSED)){
        pflow = AumentarFlujo(network);
        /*printer*/ 
        printf("camino aumentante %i:\n", network->pCounter );      
        PATH_ITER(network->path, x, y){
            dir = nbrhd_getDir(x->nbrs, y->name); /*me fijo si son BWD/FWD*/
            if(dir == FWD){
                if(x->name != network->source)
                    printf(";%"PRIu64, x->name);
                else
                    printf(";s");
            }else
                printf(">%"PRIu64, x->name);
        }
        printf(": <%"PRIu64">\n",  pflow);
        network->pCounter++;
    }

    SET_FLAG(PATHUSED);
    return pflow;
}


/*Imprime el Flujo hasta el momento con el formato:
Flujo �:
Lado x_1,y_2: <FlujoDelLado>
Donde � es "maximal" si el flujo es maximal o "no maximal" caso contrario*/
void ImprimirFlujo(DovahkiinP network){/* TODO el flujo del corte? del network? de que????? TODO*/
    Network x = NULL;
    Network y = NULL;
    u64 vflow = 0;
    assert(network != NULL);
    
    if(IS_SET_FLAG(MAXFLOW))
        printf("Flujo maximal:\n");
    else
        printf("Flujo no maximal:\n");

    PATH_ITER(network->path, x, y){
        vflow = nbrhd_getFlow(x->nbrs, y->name);
        printf("Lado %"PRIu64",%"PRIu64": %"PRIu64"\n",x->name, y->name, vflow);
    }
}


/*Debe imprimir el valor del flujo con el formato
Valor del flujo �: <ValorDelFlujo> 
Donde � es "maximal" si el flujo es maximal o "no maximal" caso contrario*/
void ImprimirValorFlujo(DovahkiinP network){
    if(IS_SET_FLAG(MAXFLOW))
        printf("Valor del flujo maximal: %"PRIu64"\n", network->flow);
    else
        printf("Valor del flujo no maximal: %"PRIu64"\n", network->flow);
}


/*Imprime un corte minimal y su capacidad con el formato:
Corte minimial: S = {s,x_1,...}
Capacidad: <Capacidad>*/
void ImprimirCorte(Dovahkiin network){
/*Si bien la capacidad del corte minimal es igual al flujo maximal, se va a calcular el flujo
de S a su complemento. Eso es mas pesado pero sirve para propositos de debugging.
igual quedan programadas las dos formas y comentada una de ellas.*/
    Network x = NULL;
    
    
    /*TODO ESTA MAL! el corte no queda en el path, sin en cut, rehacer TODO*/
    printf("Corte Minimal: S ={");
    stack_resetViewer=(network->path);
    x = stack_nextItem(network->path);
    
    while (x != NULL){
        x = stack_nextItem(network->path);
        if (x->name != network->source)
            printf("%"PRIu64",", x->name);
    }
    printf("t}\n'");
    printf("Capacidad: %"PRIu64, network->flow)      /*TODO*/
    /*TODO calcular la capacidad del  flujo maximal*/
}



/* 
 *          Funciones estaticas
*/

/*Busca el maximo valor de flujo que se puede enviar por un camino sin usar*/
u64 get_pathFlow(DovahkiinP network, bool print){
    u64 pflow;              /*path flow, valor de retorno*/
    Network x = NULL;       /*nodo x de una arista xy*/
    Netwrok y = NULL;       /*nodo y de una arista xy*/

    assert(network != NULL);
    assert(!IS_SET_FLAG(PATHUSED));
    
    pflow = u64_MAX;
    /*busqueda del flujo maximal sobre el camino aumentante*/
    PATH_ITER(network->path, x, y){
        pflow = u64_min(pflow, nbrhd_getFlow(x->nbrs, y->name));
    }

    return pflow;
}