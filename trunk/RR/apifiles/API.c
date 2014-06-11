#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "auxlibs/uthash/uthash.h"
#include "auxlibs/stack/stack.h"
#include "auxlibs/queue/queue.h"
#include "auxlibs/bstring/bstrlib.h"
#include "auxlibs/lexer/lexer.h"
#include "nbrhd.h"
#include "API.h"
#include "parser_lado.h"


#define BANNED -1    /* Valor nulo de distancia si el elem no se debe usar*/

/*      Macro: Flags de permisos y estados.     */
#ifndef MACRO_TOOLS
#define SINK_REACHED    0b00010000      /*Se llego a t. Tambien implica corte
                                          seteo solo en actualizarDistancias()*/
#define MAXFLOW         0b00001000      /*Es flujo maximal*/
#define SOURCE          0b00000100      /*Fuente fijada*/
#define SINK            0b00000010      /*Resumidero fijado*/
#define PATHUSED        0b00000001      /*Camino usado para aumentar flujo*/
/*
 *          Macro: Manejo de flags.
 */
#define SET_FLAG(f) network->flags |= f
#define UNSET_FLAG(f) network->flags &= ~f
#define CLEAR_FLAG() 0b00000001
#define IS_SET_FLAG(f) (network->flags & f) > 0
#endif


/*      Macro: Iterador sobre un path.          */
#define PATH_ITER(path, x, y)                                               \
    stack_resetViewer(path);                                               \
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

struct  DovahkiinSt{
    Network *net;       /* Network de los vertices para acceder a las aristas */
    u64 flow;           /* Valor del flujo del network */
    u64 source;         /* Vertice fijado como fuente (s) */
    u64 sink;           /* Vertice fijado como resumidero (t) */
    Network *cut;       /* Corte (se aprovecha como network auxiliar)*/
    Stack path;         /* Camino de vertices, de s a t */
    u64 pCounter;       /* Contador para la cantidad de caminos*/
    int flags;          /* Flags de estado, explicados en la seccion #define */
};


/*      Funciones estaticas     */
static u64 get_pathFlow(DovahkiinP network);
static Network *network_create(u64 n);
static void network_destroy(Network *net);
static Network *network_nextElement(Network * net, Network *node);
static void set_lvlNbrs(DovahkiinP network, Network *node, Queue q, int dir, int lvl);
static Network *set_lvl(Network *net, u64 name, int lvl);

/*Devuelve un puntero a la St o Null en caso de error */
DovahkiinP NuevoDovahkiin(void){
    DovahkiinP network;
    
    network = (DovahkiinP) malloc (sizeof(struct DovahkiinSt));
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

/*Destruye D, devuelve 1 si no hubo errores, 0 en caso contrario */
int DestruirDovahkiin(DovahkiinP network){
    assert(network!=NULL);
    
    if (network->cut != NULL)
        HASH_CLEAR(hhCut, network->cut);    
    if (network->net != NULL)
        network_destroy(network->net);  /*esto tambien libera el corte*/
    if (network->path != NULL)
        stack_destroy(network->path, NULL);
    free(network);
    network = NULL;
    return 1;
}

/*Setea al vertice s como fuente */
void FijarFuente(DovahkiinP network, u64 s){
    assert(network != NULL);
    network->source = s;
    SET_FLAG(SOURCE);
}

/*Setea al vertice t como resumidero */
void FijarResumidero(DovahkiinP network, u64 t){
    assert(network != NULL);
    network->sink = t;
    SET_FLAG(SINK);
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
        printf("Fuente: %"PRIu64"\n", network->source);
        result=0;
    }
    return result;
}

/*Si el Resumidero NO esta fijada devuelve -1, sino 0 e imprime por pantalla:
Resumidero: t
Donde x es el vertice que estamos conciderando como Resumidero.
Este es el unico caso donde el resumidero se imprimira con su nombre real y 
no con la letra t */
int ImprimirResumidero(DovahkiinP network){
    int result = -1;
    
    assert(network != NULL);
  
    if(IS_SET_FLAG(SOURCE)){
        printf("Resumidero: %"PRIu64"\n", network->sink);
        result=0;
    }
    return result;
}

/*Lee una linea desde Standar Imput que representa un lado y
devuelve el elemento de tipo Lado que lo representa si la linea es valida, 
sino devuelve el elemento LadoNulo.
Cada linea es de la forma x y c, siendo todos u64 representando el lado xy 
de capacidad c. */
Lado LeerUnLado(void){
    Lado edge = LadoNulo;
    Lexer *input;               /*analizador lexico por lineas de un archivo*/
    int clean = PARSER_OK;      /*Indica si no se encontro basura al parsear*/
   
    /*construyo el lexer sobre la entrada estandar*/
    input = lexer_new(stdin);

    if (input != NULL){
        /*Leo un lado mientras no llegue a un fin de archivo o algun error*/
        if (!lexer_is_off(input)){
            /*se parsea un lado*/
            edge = parser_lado(input);
            /*se corre el parseo hasta la siguiente linea (o fin de archivo)*/
            clean = parser_nextLine(input);
            
            if (edge != LadoNulo && !clean){ /*habia basura, error*/
                lado_destroy(edge);
                edge = LadoNulo;
            }
        }
        lexer_destroy(input);
    }
    
    return edge;
}

/*Carga un edge al network. Retorna 1 si no hubo problemas y 0 caso contrario.*/
int CargarUnLado(DovahkiinP network, Lado edge){
    Network *nodeX = NULL;
    Network *nodeY = NULL;
    u64 x, y;
    int result = 0;
    
    assert(network != NULL);
    
    if (edge != LadoNulo){
        x = lado_getX(edge);
        /* cargo el nodo 'x', si todavia no existe*/
        HASH_FIND(hhNet, network->net, &(x), sizeof(x), nodeX);
        if (nodeX == NULL){
            nodeX = network_create(x);
            HASH_ADD(hhNet, network->net, name, sizeof(network->net->name), nodeX);
        }
        
        y = lado_getY(edge);
        /*cargo el nodo 'y', si todavia no existe*/
        HASH_FIND(hhNet, network->net, &(y), sizeof(y), nodeY);
        if (nodeY == NULL){
            nodeY = network_create(y);
            HASH_ADD(hhNet, network->net, name, sizeof(network->net->name), nodeY);
        }
        /*se establecen como vecinos*/
        nbrhd_addEdge(nodeX->nbrs, nodeY->nbrs, edge);
        result = 1;
        lado_destroy(edge); /*Destruyo el lado, ya no nos sirve*/
    }
    
    return result;
}

/*Preprocesa el Dovahkiin para empezar a buscar caminos aumentantes. 
Debe chequear que esten seteados s y t. 
Devuelve 1 si puede preparar y 0 caso contrario*/
int Prepararse(DovahkiinP network){
    int status = 0;
    u64 s, t;
    Network *src = NULL;
    Network *snk = NULL;
    
    assert(network != NULL);
    if(IS_SET_FLAG(SINK) && IS_SET_FLAG(SOURCE)){
        s = network->source;
        t = network->sink;
        HASH_FIND(hhNet, network->net, &(s), sizeof(s), src);
        HASH_FIND(hhNet, network->net, &(t), sizeof(t), snk);
        if (src != NULL && snk != NULL)
            status = 1;
    }
    return status;
}

/*Actualiza haciendo una busqueda BFS-FF. 
 * Devuelve 1 si existe un camino aumentante entre s y t, 0 caso contrario*/
int ActualizarDistancias(DovahkiinP network){
    
    Queue q, qNext, qAux;       /*Colas para el manejo de los niveles. 
                                 *q = actual, qNext = siguiente, qAux = swap*/
    Network *node = NULL;       /* nodo actual de 'q' en el cual se itera*/
    Network *k = NULL;          /* iterador en el reseteo de las distancias*/
    int lvl = 0;                /* Distancia para elementos de qNext*/

    assert(network != NULL);
   
    /* preparacion de las cosas que voy a usar*/
    UNSET_FLAG(SINK_REACHED);
    HASH_CLEAR(hhCut, network->cut);    /*antes de empezar se limpia el corte*/
    q = queue_create();
    qNext = queue_create();
   
    /* Reset de distancias por corridas anteriores*/
    for(k = network->net; k != NULL; k = k->hhNet.next){
        k->level = BANNED;
    }

    /*seteo Source en nivel 0*/
    node = set_lvl(network->net, network->source, lvl);
    assert(node != NULL);
/**/printf("\n----------- NUEVA ITERACION\n");
/**/printf("\n--- Distancias (BFS)\n");
/**/printf("nodo: %"PRIu64" lvl: %i(%i)\n", node->name, node->level, lvl);
/**/printf("vflow: %"PRIu64"\n", network->flow);
    queue_enqueue(q, node);
    lvl++;

    /*actualizacion de distancias por BFS */
    while(!queue_isEmpty(q) && !IS_SET_FLAG(SINK_REACHED)){
        node = queue_head(q);
        /*Busqueda y actualizacion de niveles de nodos*/
        set_lvlNbrs(network, node, qNext, FWD, lvl);
        set_lvlNbrs(network, node, qNext, BWD, lvl);
        printf("qNext_size %i\n", queue_size(qNext));
        HASH_ADD(hhCut, network->cut, name, sizeof(network->cut->name), node);
        queue_dequeue(q);        
        /* Se terminaron los vertices de este nivel, se pasa al siguiente*/
        if(queue_isEmpty(q)){
            qAux = q;
            q = qNext;
            qNext = qAux;
            lvl++;
        }
    }
    if (IS_SET_FLAG(SINK_REACHED))
        SET_FLAG(MAXFLOW);
    queue_destroy(q, NULL);
    queue_destroy(qNext, NULL);
    return IS_SET_FLAG(SINK_REACHED);
}

/*Hace una busqueda FF DFS usando las etiquetas de ActualizarDistancia(). 
 * Devuelve 1 si llega a t, 0 caso contrario.*/
int BusquedaCaminoAumentante(DovahkiinP network){
    u64 s, t;
    Network *node = NULL;
    int t_reached = 0;
    
    assert(network != NULL);
    s = network->source;
    t = network->sink;
   
    if (IS_SET_FLAG(PATHUSED)){
        /*preparacion de todas las cosas que uso para buscar un camino aumentante*/
        if (network->path != NULL)
            stack_destroy(network->path, NULL);
        network->path = stack_create();
        HASH_FIND(hhNet, network->net, &(s), sizeof(s), node);
        stack_push(network->path, node);
        /*Busqueda*/
        while(!stack_isEmpty(network->path) && (node->name != t)){
            node = network_nextElement(network->net, node);
            if (node != NULL){
                stack_push(network->path, node);
            }else{
                stack_pop(network->path);
                node = (Network*)stack_top(network->path); /* NULL si la pila esta vacia*/
            }
        }
    }
    
    if (network->path != NULL && !stack_isEmpty(network->path)){
        t_reached = ((Network*)stack_top(network->path))->name == t;
        if (t_reached)
            UNSET_FLAG(PATHUSED); 
    }
    return t_reached;
}

/*Precondicion: (BusquedaCaminoAumentante()==1) 
 * que todavia no haya aumentado el flujo. (Igual se tiene que checkear). 
 * Aumenta el flujo. 
 * Debe devolver el valor del flujo aumentado si no hubo problemas, 
 * 0 caso contrario (inclusive !precondicion).*/
u64 AumentarFlujo(DovahkiinP network){
    u64 pflow = 0;          /*flujo a enviar por el camino aumentante*/ 
    Network *x = NULL;       /*nodo x de una arista xy*/
    Network *y = NULL;       /*nodo y de una arista xy*/
    
    assert(network != NULL);
    
    if (!IS_SET_FLAG(PATHUSED)){
        pflow = get_pathFlow(network);
        printf("\n-- AumentarFlujo pflow: %"PRIu64"\n\n", pflow);
        /*incremento del flujo sobre cada arista xy*/
        PATH_ITER(network->path, x, y){
            nbrhd_increaseFlow(x->nbrs, y->name, pflow); /*por BWD disminuye!*/
        }
        network->flow += pflow; 
        network->pCounter++;
    }
    SET_FLAG(PATHUSED);
    return pflow;
}

/*Idem AumentarFlujo() pero tambien imprime el camino con el formato:
 * camino aumentante #:
 * t;x_r;...;x_1;s: <cantDelIncremento>
 * Donde # es el numero del camino aumentante, 
 * ";" se usa en caminos forward y ">" en backward.*/
u64 AumentarFlujoYTambienImprimirCamino(DovahkiinP network){
    u64 pflow = 0;
    Network *x = NULL;
    Network *y = NULL;
    int dir = NONE;
    
    assert(network != NULL);
    
    if (!IS_SET_FLAG(PATHUSED)){
        pflow = AumentarFlujo(network);
        /*printer*/ 
        printf("camino aumentante %"PRIu64":\nt", network->pCounter );      
        PATH_ITER(network->path, x, y){
            dir = nbrhd_getDir(x->nbrs, y->name); /*me fijo si son BWD/FWD*/
            assert(dir == FWD || dir == BWD);
            if(dir == FWD){
                if(x->name != network->source)
                    printf(";%"PRIu64, x->name);
                else
                    printf(";s");
            }else
                printf(">%"PRIu64, x->name);
        }
        printf(": <%"PRIu64">\n",  pflow);
    }

    SET_FLAG(PATHUSED);
    return pflow;
}

/*Imprime el Flujo hasta el momento con el formato:
Flujo �:
Lado x_1,y_2: <FlujoDelLado>
Donde � es "maximal" si el flujo es maximal o "no maximal" caso contrario*/
void ImprimirFlujo(DovahkiinP network){
    Network *x = NULL;
    Network *xTmp = NULL;
    u64 yName;
    u64 vflow = 0;
    int flag;
    
    assert(network != NULL);
    
    if(IS_SET_FLAG(MAXFLOW))
        printf("Flujo maximal:\n");
    else
        printf("Flujo no maximal:\n");

    HASH_ITER(hhNet, network->net, x, xTmp){
        flag = FST;
        printf("»");
        while(nbrhd_getNext(x->nbrs, flag, FWD, &yName) != NONE ){
            printf(":");
            flag = NXT;
            vflow = nbrhd_getFlow(x->nbrs, yName);
            printf("Lado %"PRIu64",%"PRIu64": %"PRIu64"\n",x->name, yName, vflow);
        }
    }
    printf("\n");
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

/*Si bien la capacidad del corte minimal es igual al flujo maximal, se va a calcular el flujo
de S a su complemento. Eso es mas pesado pero sirve para propositos de debugging.
igual quedan programadas las dos formas y comentada una de ellas.*/   
void ImprimirCorte(DovahkiinP network){
    Network *node = NULL;
    Network *yNode = NULL;
    u64 yName;
    u64 cflow = 0;
    int dir;
    assert(network!=NULL);
    printf("Corte Minimal: S = {s");

    for(node=network->cut; node != NULL; node=node->hhCut.next){/*Itero sobre los nodos*/
        dir = FST;
        while(nbrhd_getNext(node->nbrs, dir, FWD, &yName) != NONE ){/*Itero sobre los vecinos*/
            HASH_FIND(hhCut, network->cut, &yName, sizeof(yName), yNode);
            if(yNode == NULL){
                /*printf("cflow=%"PRIu64", xy=%"PRIu64" -> %"PRIu64"\n", nbrhd_getFlow(node->nbrs, yName), node->name, yName);*/
                cflow+=nbrhd_getFlow(node->nbrs, yName);}
            dir = NXT;
        }
        if (node->name != network->source)
            printf(",%"PRIu64, node->name);
    }
    printf("}\n");
    printf("Capacidad: %"PRIu64"\n", cflow);
}



/* 
 *          Funciones estaticas
*/

/*Busca el maximo valor de flujo que se puede enviar por un camino sin usar*/
u64 get_pathFlow(DovahkiinP network){
    u64 pflow = u64_MAX;              /*path flow, valor de retorno*/
    u64 cap, flow;
    Network *x = NULL;       /*nodo x de una arista xy*/
    Network *y = NULL;       /*nodo y de una arista xy*/
    assert(network != NULL);
    assert(!IS_SET_FLAG(PATHUSED));

    /*busqueda del flujo maximal sobre el camino aumentante*/
    PATH_ITER(network->path, x, y){        
        flow = nbrhd_getFlow(x->nbrs, y->name);
        if(nbrhd_getDir(x->nbrs, y->name) == FWD){
            cap = nbrhd_getCap(x->nbrs, y->name);
            pflow = u64_min(pflow, cap-flow);
        }else
            pflow = u64_min(pflow, flow);
    }
    return pflow;
}

/*Constructor de Network */
static Network *network_create(u64 n){
    Network *node;
    
    node = (Network*) malloc(sizeof(Network));
    assert(node != NULL);
    
    node->name = n;
    node->nbrs = nbrhd_create();
    node->level = BANNED; 
    
    return node;
}

/*Destructor del network*/
static void network_destroy(Network *net){
    Network *elem = NULL;    /*el i-esimo elemento de la hash table*/
    Network *eTmp = NULL;    /*el i-esimo + 1 elemento de la hash table*/
    
    assert(net!=NULL);
    HASH_ITER(hhNet, net, elem, eTmp){
        HASH_DELETE(hhNet, net, elem); /*elimina la referencia sobre la hash table*/
        nbrhd_destroy(elem->nbrs);
        free(elem);
    }

    net = NULL;
}

/*Busca el siguente elemento que cumple las condiciones de poder mandar flujo 
forward o backward del nodo "node" y devuelve un puntero hacia ese elemento*/
static Network *network_nextElement(Network * net, Network * node){
    Network * nextNode = NULL;
/*    Network * enextNode = NULL;
    Network * fnextNode = NULL;
*/            
    int dir = FWD;
    u64 yName, flow, cap;//, eName,fName;
    bool breakW= false;
    
    assert(net != NULL && node != NULL);
    dir = nbrhd_getNext(node->nbrs, FST, dir, &yName);
    if (dir == NONE){
        dir = nbrhd_getNext(node->nbrs, FST, dir, &yName);
    }
        
    if(node->name == 7)
        printf("el nodo q le dan a f como siguiente es %"PRIu64"\n", yName);

    while(dir != NONE && !breakW){
        HASH_FIND(hhNet, net, &yName, sizeof(yName), nextNode);
        if(nextNode->level !=  BANNED && nextNode->level ==  node->level+1){
            flow = nbrhd_getFlow(node->nbrs, nextNode->name);        
            if(dir == FWD){
                cap = nbrhd_getCap(node->nbrs, nextNode->name);
                breakW = cap > flow;
            }else if(dir == BWD)
                breakW = flow > 0;
           // printf("%i\t %"PRIu64"\n", dir, yName);
        }
        if (dir == FWD ){
            dir = nbrhd_getNext(node->nbrs, NXT, dir, &yName);
            if (dir == NONE){
                dir = BWD;
                dir = nbrhd_getNext(node->nbrs, FST, dir, &yName);
            }
        }else
            dir = nbrhd_getNext(node->nbrs, NXT, dir, &yName);
        if(node->name == 7)
            printf("el nodo q le dan a f como siguiente es (while)%"PRIu64"\n", yName);
    }
/*    eName=6;
    fName=7;
    HASH_FIND(hhNet, net, &eName, sizeof(yName), enextNode);
    HASH_FIND(hhNet, net, &fName, sizeof(yName), fnextNode);
    printf("nodo e(%"PRIu64")->f(%"PRIu64")\n",eName, fName);
    printf("e_lvl:%i   f_lvl:%i\n",enextNode->level, fnextNode->level);
    printf("Capacidad: %"PRIu64"   Flujo: %"PRIu64"\n", nbrhd_getCap(enextNode->nbrs, fnextNode->name),nbrhd_getFlow(enextNode->nbrs, fnextNode->name));
*/
    if(!breakW && dir == NONE){
        node->level = BANNED;
        nextNode = NULL;
    }    
    return nextNode;
}

/* Actualiza las distancias de los vecinos FWD y BWD de un nodo. 
 * Los vecinos que se actualizaron se agregan a la cola 'q'.
 * Precondicion: 
 */
static void set_lvlNbrs(DovahkiinP network, Network *node, Queue q, int dir, int lvl){
    Network *yNode = NULL;
    bool canBeUsed = false;
    Nbrhd nbrs = NULL;
    u64 yName, oldname;

    assert(node != NULL && q != NULL);
    
    nbrs = node->nbrs;
    dir = nbrhd_getNext(nbrs, FST, dir, &yName);
    while(!IS_SET_FLAG(SINK_REACHED) && dir != NONE){
        if(dir == FWD)
            canBeUsed = nbrhd_getCap(nbrs, yName) > nbrhd_getFlow(nbrs, yName);
        else
            canBeUsed = nbrhd_getFlow(nbrs, yName) > 0;
       /* canBeUsed = (nbrhd_getCap(nbrs, yName) > nbrhd_getFlow(nbrs, yName)) ^ \
                    (nbrhd_getFlow(nbrs, yName) > 0);*/
        if (canBeUsed){                 /*busco el nodo y actualizo su nivel*/
            yNode = set_lvl(node, yName, lvl);
/*    printf("nodo: %"PRIu64" lvl: %i(%i) -- edge=%"PRIu64"_%"PRIu64"_%"PRIu64"\n", \
               yNode->name, yNode->level, lvl, node->name, yName, nbrhd_getFlow(nbrs, yName));*/
            if(yNode != NULL)
                queue_enqueue(q, yNode);
            if(yName == network->sink)       /*llego a t*/
                SET_FLAG(SINK_REACHED);
        }
        oldname=yName;
        dir = nbrhd_getNext(nbrs, NXT, dir, &yName);
        printf("nodo: %"PRIu64" ->: %"PRIu64"\n", oldname, yName);
    }
}


/* Busca por 'name' y actualiza el valor de distancia de un nodo.
 * Retorna un puntero al nodo modificado.
 * Precondicion: 'name' debe ser un nombre de nodo existente
 */
static Network *set_lvl(Network *net, u64 name, int lvl){
    Network *node=NULL;
    
    assert(net != NULL);
    
    HASH_FIND(hhNet, net, &name, sizeof(name), node);
    assert(node != NULL);
    if(node->level == BANNED)
        node->level = lvl;
    else
        node=NULL;
    return node;
}

