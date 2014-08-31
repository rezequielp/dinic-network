#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "_uthash.h"
#include "_stack.h"
#include "_queue.h"
#include "_bstrlib.h"
#include "_lexer.h"
#include "nbrhd.h"
#include "API.h"
#include "parser_lado.h"


#define LVL_NIL -1    /**< Valor nulo de distancia para los nodos*/

/* Macro: Flags de permisos y estados.*/
#define SINK_REACHED    0b00010000      /**<Se llego a t.*/
#define MAXFLOW         0b00001000      /**<Es flujo maximal. Implica corte.
                                          Seteo solo en actualizarDistancias()*/
#define SOURCE          0b00000100      /**<Fuente fijada*/
#define SINK            0b00000010      /**<Resumidero fijado*/
#define PATHUSED        0b00000001      /**<Camino usado para aumentar flujo*/

/* Macro: Manejo de flags.*/
#define SET_FLAG(f) dova->flags |= f    /**<Activa(1) el bit de la flag f*/
#define UNSET_FLAG(f) dova->flags &= ~f /**<Desactiva(0) el bit de la flag f*/
#define CLEAR_FLAG() 0b00000001         /**<Inicializa todas las flags (reset)*/
#define IS_SET_FLAG(f) (dova->flags & f) > 0 /**<El bit de la flag f es 1?*/


/* Macro: Iterador sobre un path.*/
/** Itera sobre los elementos de un path*/
#define PATH_ITER(path, x, y)                                               \
    stack_resetViewer(path);                                               \
    y = stack_nextItem(path);                                               \
    x = stack_nextItem(path);                                               \
    for(; (x)!=NULL; (y)=(x), (x)=stack_nextItem(path))

/* Estructuras */
/** Estructura de un netwrok con la información de los nodos.
 * Contiene la información de un nodo: su nombre, quiénes son sus vecinos y 
 * su nivel de distancia en la búsqueda de caminos aumentantes. 
 * Estos nodos estan registrados en una tabla hash: La del network original que 
 * conforman, y en el corte si es que forman parte.
 */
typedef struct NetworkSt{
    u64 name;                   /**<Hash key - nombre del nodo*/
    Nbrhd nbrs;                 /**<Hash value - vecinos del nodo*/
    int level;                  /**<Nivel de distancia del nodo*/
    UT_hash_handle hhNet,hhCut; /**<Hace esta estructura hashable */
} Network;

/** Estructura de un DovahkiinP con los datos que se necesitan almacenar para 
 * ejecutar Dinic.
 * Contiene toda la información en el nivel más alto de ejecución, como el
 * acceso al network, el valor de flujo y corte calculados, qué nodo es la 
 * fuente y cuál es resumidero. El último camino aumentante encontrado sin usar 
 * y la cantidad ya utilizados. También se almacenan las flags de estados 
 * que se necesiten en el correr de Dinic.
 */
struct  DovahkiinSt{
    Network *net;       /**<Network de los nodos para acceder a las aristas.*/
    u64 flow;           /**<Valor del flujo del dova.*/
    u64 source;         /**<Vertice fijado como fuente (s).*/
    u64 sink;           /**<Vertice fijado como resumidero (t).*/
    Network *cut;       /**<Corte (se aprovecha como dova auxiliar)*/
    Stack path;         /**<Camino de nodos, de s a t.*/
    u64 pCounter;       /**<Contador para la cantidad de caminos.*/
    int flags;          /**<Flags de estado, explicados en la seccion define.*/
};


/*      Funciones estaticas     */
static u64 get_pathFlow(DovahkiinP dova);
static Network *network_create(u64 n);
static void network_destroy(Network *net);
static Network *network_nextElement(Network * net, Network *node);
static void set_lvlNbrs(DovahkiinP dova, Network *node, Queue q, int dir, int lvl);
static Network *set_lvl(Network *net, u64 name, int lvl);

/** Creador de un nuevo DovahkiinP.
 * \return un DovahkiinP vacío.
 */
DovahkiinP NuevoDovahkiin(void){
    DovahkiinP dova;    /*El dova a crear y devolver*/
    
    /*Se le asigna memoria*/
    dova = (DovahkiinP) malloc (sizeof(struct DovahkiinSt));
    assert(dova!=NULL);
    /*Valores iniciales de un dova vacío*/
    dova->net = NULL;
    dova->flow = 0;
    dova->source = 0;
    dova->sink = 0;
    dova->cut = NULL;
    dova->path = NULL;
    dova->flags = CLEAR_FLAG();
    dova->pCounter = 0; 
    return dova;
}

/** Destructor de un DovahkiinP.
 * \param dova El dova a destruir.
 * \return devuelve 1 si no hubo errores, 0 en caso contrario.
 * \note Por la cátedra se indica un retorno distinto en caso de error, pero 
 * en la implementación no surgen casos en lo que esto pueda suceder.
 */
int DestruirDovahkiin(DovahkiinP dova){
    assert(dova!=NULL);
    /*Primero hay que destruir las entradas de la hash del corte.
    Si se destruyen primero los nodos entonces pierdo referencias*/
    if (dova->cut != NULL)
        HASH_CLEAR(hhCut, dova->cut);
    /*En este punto ya se puede destruir el network y todos los nodos.
    Esto tambien libera la hash del corte.*/
    if (dova->net != NULL)
        network_destroy(dova->net);
    /*Se destruye cualquier path almacenado.*/
    if (dova->path != NULL)
        stack_destroy(dova->path, NULL);
    free(dova);
    dova = NULL;
    return 1;
}

/** Establece un nodo como fuente del network.
 * \param dova  El dova en el que se trabaja.
 * \param s     El nombre del nodo.
 * \pre \a dova debe ser un DovahkiinP no nulo.
 */
void FijarFuente(DovahkiinP dova, u64 s){
    assert(dova != NULL);
    dova->source = s;
    SET_FLAG(SOURCE);
}

/** Establece un nodo como resumidero del network.
 * \param dova  El dova en el que se trabaja.
 * \param t     El nombre del nodo.
 * \pre \a dova debe ser un DovahkiinP no nulo.
 */
void FijarResumidero(DovahkiinP dova, u64 t){
    assert(dova != NULL);
    dova->sink = t;
    SET_FLAG(SINK);
}

/** Imprime por la salida estandar el nombre del nodo que es fuente.
 * Imprime por pantalla:\n
 * Fuente: s \n
 * Donde s es el nodo que estamos conciderando como fuente. Este es el unico 
 * caso donde la fuente se imprimira con su nombre real y no con la letra s. 
 * \param dova  El dova en el que se trabaja.
 * \pre \a dova debe ser un DovahkiinP no nulo.
 * \return  -1 si la fuente no esta fijada.\n
 *          0 caso contrario e imprime por pantalla.
 */
int ImprimirFuente(DovahkiinP dova){
    int result = -1;    /*Resultado de la operacion.*/
    
    assert(dova != NULL);
    /*Imprimo solo si la fuente fue fijada*/
    if(IS_SET_FLAG(SOURCE)){
        printf("Fuente: %"PRIu64"\n", dova->source);
        result=0;
    }
    return result;
}

/** Imprime por la salida estandar el nombre del nodo que es resumidero.
 * Imprime por pantalla:\n
 * Resumidero: t \n
 * Donde t es el nodo que estamos conciderando como resumidero. Este es el unico 
 * caso donde el resumidero se imprimira con su nombre real y no con la letra t. 
 * \param dova  El dova en el que se trabaja.
 * \pre \a dova debe ser un DovahkiinP no nulo.
 * \return  -1 si el resumidero no esta fijado.\n
 *          0 caso contrario e imprime por pantalla.
 */
int ImprimirResumidero(DovahkiinP dova){
    int result = -1;    /*Resultado de la operacion.*/
    
    assert(dova != NULL);
    /*Imprimo solo si el resumidero fue fijado*/
    if(IS_SET_FLAG(SINK)){
        printf("Resumidero: %"PRIu64"\n", dova->sink);
        result=0;
    }
    return result;
}

/** Lee un lado desde la entrada estandar.
 * Lee una linea desde Standar Input que representa un lado y
 * devuelve el elemento de tipo Lado que lo representa si la linea es valida, 
 * sino devuelve el elemento LadoNulo. \n
 * Cada linea es de la forma x y c, siendo todos u64 representando el lado xy 
 * de capacidad c. 
 * \return  Un lado legal con los datos leidos.\n
 *          LadoNulo si la linea leida no es valida.
 */
Lado LeerUnLado(void){
    Lado edge = LadoNulo;   /*El lado con los datos leidos*/
    Lexer *input;           /*Analizador lexico por lineas de un archivo*/
    int clean = PARSER_OK;  /*Indica si no se encontro basura al parsear*/
   
    /*Construyo el lexer sobre la entrada estandar*/
    input = lexer_new(stdin);
    if (input != NULL){
        /*Leo un lado mientras no llegue a un fin de archivo o algun error*/
        if (!lexer_is_off(input)){
            /*Se parsea un lado*/
            edge = parser_lado(input);
            /*Se corre el parseo hasta la siguiente linea (o fin de archivo)*/
            clean = parser_nextLine(input);
            /*Si habia basura, error*/
            if (edge != LadoNulo && !clean){
                lado_destroy(edge);
                edge = LadoNulo;
            }
        }
        lexer_destroy(input);
    }
    
    return edge;
}

/** Carga un lado no nulo en un DovahkiinP.
 * Se utilizan los datos que contiene el lado para crear las estructuras de los
 * nodos y cargarlos en el netwrok, y se establecen como vecinos para crear la
 * arista que los relaciona.
 * \param dova  El dova en el que se trabaja.
 * \param edge  El lado a cargar.
 * \pre \a dova Debe ser un DovahkiinP no nulo.
 * \return  1 si no hubo problemas.\n
 *          0 caso contrario.
 */
int CargarUnLado(DovahkiinP dova, Lado edge){
    Network *nodeX = NULL;  /*Estructura con los datos del nodo 'x'*/
    Network *nodeY = NULL;  /*Estructura con los datos del nodo 'y'*/
    u64 x, y;               /*Nombres del nodo 'x' e 'y'*/
    int result = 0;         /*retorno*/
    
    assert(dova != NULL);
    
    if (edge != LadoNulo){
        x = lado_getX(edge);
        /*Cargo el nodo 'x', si todavia no existe en el network*/
        HASH_FIND(hhNet, dova->net, &(x), sizeof(x), nodeX);
        if (nodeX == NULL){
            nodeX = network_create(x);
            HASH_ADD(hhNet, dova->net, name, sizeof(dova->net->name), nodeX);
        }
        
        y = lado_getY(edge);
        /*Cargo el nodo 'y', si todavia no existe en el network*/
        HASH_FIND(hhNet, dova->net, &(y), sizeof(y), nodeY);
        if (nodeY == NULL){
            nodeY = network_create(y);
            HASH_ADD(hhNet, dova->net, name, sizeof(dova->net->name), nodeY);
        }
        /*Se establecen como vecinos. Arista 'xy'*/
        nbrhd_addEdge(nodeX->nbrs, nodeY->nbrs, edge);
        result = 1;
        lado_destroy(edge); /*Destruyo el lado, ya no nos sirve*/
    }
    
    return result;
}

/** Preprocesa el DovahkiinP para empezar a buscar caminos aumentantes. 
 * Aqui se debe chequear y preparar todo lo que sea necesario para comenzar
 * a buscar caminos aumentantes. \n
 * Por el momento solo hace falta chequear que esten seteados s y t, 
 * y que estos nodos existen en el network.
 * \param dova  El dova en el que se trabaja.
 * \pre \a dova Debe ser un DovahkiinP no nulo.
 * \return  1 si los preparativos fueron exitosos.\n
 *          0 caso contrario.
 */
int Prepararse(DovahkiinP dova){
    int status = 0;         /*Estado de los preparativos. Retorno*/
    u64 s, t;               /*Nombres del nodo 's' y 't'*/
    Network *src = NULL;    /*Puntero al nodo 's' en el network*/
    Network *snk = NULL;    /*Puntero al nodo 't' en el network*/
    
    assert(dova != NULL);
    /*Si la fuente y el resumidero estan fijados entonces chequeo que existen
     en el network*/
    if(IS_SET_FLAG(SINK) && IS_SET_FLAG(SOURCE)){
        s = dova->source;
        t = dova->sink;
        /*Se buscan en el network*/
        HASH_FIND(hhNet, dova->net, &(s), sizeof(s), src);
        HASH_FIND(hhNet, dova->net, &(t), sizeof(t), snk);
        if (src != NULL && snk != NULL) 
            status = 1; /*Ambos existen, todo OK*/
    }
    return status;
}

/** Actualiza las distancias haciendo una busqueda BFS-FF.
 * Se reinician todas las distancias a nulo y se comienzan a actualizar a partir
 * del nodo fuente \a s utilizando BFS-FF, hasta encontrarse con el
 * nodo resumidero \a t. \n
 * Si se alcanza \a t entonces implica un camino aumentante. Caso contrario, el
 * flujo actual es maximal y los nodos que fueron actualizados conforman el
 * corte.
 * \param dova  El dova en el que se trabaja.
 * \pre \a dova Debe ser un DovahkiinP no nulo.
 * \return  1 si existe un camino aumentante entre \a s y \a t. \n
 *          0 caso contrario.
 */
int ActualizarDistancias(DovahkiinP dova){
    Queue q, qNext, qAux;       /*Colas para el manejo de los niveles. 
                                 *q = actual, qNext = siguiente, qAux = swap*/
    Network *node = NULL;       /* nodo actual de 'q' en el cual se itera*/
    Network *k = NULL;          /* iterador en el reseteo de las distancias*/
    int lvl = 0;                /* Distancia para elementos de qNext*/

    assert(dova != NULL);
    
    /*Preparacion de las cosas que voy a usar*/
    UNSET_FLAG(SINK_REACHED);
    HASH_CLEAR(hhCut, dova->cut);
    q = queue_create();
    qNext = queue_create();
    
    /* Reset de distancias por posibles corridas anteriores*/
    for(k = dova->net; k != NULL; k = k->hhNet.next){
        k->level = LVL_NIL;
    }
    
    /*La fuente es nivel 0*/
    node = set_lvl(dova->net, dova->source, 0);
    assert(node != NULL);
    queue_enqueue(q, node);
    lvl++;

    /*Actualizacion de distancias por BFS */
    while(!queue_isEmpty(q) && !IS_SET_FLAG(SINK_REACHED)){
        node = queue_head(q);
        /*Busqueda y actualizacion de niveles de nodos vecinos de 'node'*/
        set_lvlNbrs(dova, node, qNext, FWD, lvl);   /*Forwards*/
        set_lvlNbrs(dova, node, qNext, BWD, lvl);   /*Backwards*/
        /*Agrego 'node' al posible corte*/
        HASH_ADD(hhCut, dova->cut, name, sizeof(dova->cut->name), node);
        queue_dequeue(q);
        /*Si se terminaron los nodos de este nivel, se pasa al siguiente*/
        if(queue_isEmpty(q)){
            qAux = q;
            q = qNext;
            qNext = qAux;
            lvl++;
        }
    }
    /*Si se alcanzo 't'*/
    if(IS_SET_FLAG(SINK_REACHED)){
        HASH_CLEAR(hhCut, dova->cut);   /*No hay corte*/
    }else
        SET_FLAG(MAXFLOW);  /*El flujo es maximal => hay corte*/

    queue_destroy(q, NULL);
    queue_destroy(qNext, NULL);
    return IS_SET_FLAG(SINK_REACHED);
}

/** Hace una busqueda DFS-FF de un camino aumentante de menor longitud.
 * Solo se utilizan los nodos que tengan su distancia actualizada.
 * El ultimo nodo agregado al camino solo agrega a otro nodo si este ultimo 
 * tiene una distancia +1 que el, y si se puede aumentar (o disminuir) flujo 
 * entre ellos.
 * \param dova  El dova en el que se trabaja.
 * \pre \a dova Debe ser un DovahkiinP no nulo.
 * \return  1 si llega a \a t. \n
 *          0 caso contrario.
 */
int BusquedaCaminoAumentante(DovahkiinP dova){
    u64 s, t;
    Network *node = NULL;
    int t_reached = 0;
    
    assert(dova != NULL);
    s = dova->source;
    t = dova->sink;
   
    if (IS_SET_FLAG(PATHUSED)){
        /*preparacion de todas las cosas que uso para buscar un camino aumentante*/
        if (dova->path != NULL)
            stack_destroy(dova->path, NULL);
        dova->path = stack_create();
        HASH_FIND(hhNet, dova->net, &(s), sizeof(s), node);
        stack_push(dova->path, node);
        /*Busqueda*/
        while(!stack_isEmpty(dova->path) && (node->name != t)){
            node = network_nextElement(dova->net, node);
            if (node != NULL){
                stack_push(dova->path, node);
            }else{
                stack_pop(dova->path);
                node = (Network*)stack_top(dova->path); /* NULL si la pila esta vacia*/
            }
        }
    }
    
    if (dova->path != NULL && !stack_isEmpty(dova->path)){
        t_reached = ((Network*)stack_top(dova->path))->name == t;
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
u64 AumentarFlujo(DovahkiinP dova){
    u64 pflow = 0;          /*flujo a enviar por el camino aumentante*/ 
    Network *x = NULL;       /*nodo x de una arista xy*/
    Network *y = NULL;       /*nodo y de una arista xy*/
    
    assert(dova != NULL);
    
    if (!IS_SET_FLAG(PATHUSED)){
        pflow = get_pathFlow(dova);
        /*incremento del flujo sobre cada arista xy*/
        PATH_ITER(dova->path, x, y){
            nbrhd_increaseFlow(x->nbrs, y->name, pflow); /*por BWD disminuye!*/
        }
        dova->flow += pflow; 
        dova->pCounter++;
    }
    SET_FLAG(PATHUSED);
    return pflow;
}

/*Idem AumentarFlujo() pero tambien imprime el camino con el formato:
 * camino aumentante #:
 * t;x_r;...;x_1;s: <cantDelIncremento>
 * Donde # es el numero del camino aumentante, 
 * ";" se usa en caminos forward y ">" en backward.*/
u64 AumentarFlujoYTambienImprimirCamino(DovahkiinP dova){
    u64 pflow = 0;
    Network *x = NULL;
    Network *y = NULL;
    int dir = NONE;
    
    assert(dova != NULL);
    
    if (!IS_SET_FLAG(PATHUSED)){       
        pflow = AumentarFlujo(dova);
        /*printer*/ 
        printf("camino aumentante %"PRIu64":\nt", dova->pCounter );      
        PATH_ITER(dova->path, x, y){
            dir = nbrhd_getDir(x->nbrs, y->name); /*me fijo si son BWD/FWD*/
            assert(dir == FWD || dir == BWD);
            if(dir == FWD){
                if(x->name != dova->source)
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
void ImprimirFlujo(DovahkiinP dova){
    Network *x = NULL;
    Network *xTmp = NULL;
    u64 yName;
    u64 vflow = 0;
    int flag;
    
    assert(dova != NULL);
    
    if(IS_SET_FLAG(MAXFLOW))
        printf("Flujo maximal:\n");
    else
        printf("Flujo no maximal:\n");

    HASH_ITER(hhNet, dova->net, x, xTmp){
        flag = FST;
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
void ImprimirValorFlujo(DovahkiinP dova){
    if(IS_SET_FLAG(MAXFLOW))
        printf("Valor del flujo maximal: %"PRIu64"\n", dova->flow);
    else
        printf("Valor del flujo no maximal: %"PRIu64"\n", dova->flow);
}

/*Imprime un corte minimal y su capacidad con el formato:
Corte minimial: S = {s,x_1,...}
Capacidad: <Capacidad>*/

/*WARNING ESTE COMENTARIO ESTA MAL! (se esta calculando sumando los flujos)
 * Si bien la capacidad del corte minimal es igual al flujo maximal, se va a calcular el flujo
de S a su complemento. Eso es mas pesado pero sirve para propositos de debugging.
igual quedan programadas las dos formas y comentada una de ellas.*/   
void ImprimirCorte(DovahkiinP dova){
    Network *node = NULL;
    Network *yNode = NULL;
    u64 yName;
    u64 cflow = 0;
    int dir;
    assert(dova!=NULL);
    /*TODO hay que chequear que dova->cut no es nulo*/
    printf("Corte Minimal: S = {s");
    for(node=dova->cut; node != NULL; node=node->hhCut.next){/*Itero sobre los nodos*/
        dir = FST;
        while(nbrhd_getNext(node->nbrs, dir, FWD, &yName) != NONE ){/*Itero sobre los vecinos*/
            HASH_FIND(hhCut, dova->cut, &yName, sizeof(yName), yNode);
            if(yNode == NULL){
                /*printf("cflow=%"PRIu64", xy=%"PRIu64" -> %"PRIu64"\n", nbrhd_getFlow(node->nbrs, yName), node->name, yName);*/
                cflow+=nbrhd_getFlow(node->nbrs, yName);}
            dir = NXT;
        }
        if (node->name != dova->source)
            printf(",%"PRIu64, node->name);
    }
    printf("}\n");
    printf("Capacidad: %"PRIu64"\n", cflow);
}



/* 
 *          Funciones estaticas
*/

/*Busca el maximo valor de flujo que se puede enviar por un camino sin usar*/
u64 get_pathFlow(DovahkiinP dova){
    u64 pflow = u64_MAX;              /*path flow, valor de retorno*/
    u64 cap, flow;
    Network *x = NULL;       /*nodo x de una arista xy*/
    Network *y = NULL;       /*nodo y de una arista xy*/
    assert(dova != NULL);
    assert(!IS_SET_FLAG(PATHUSED));

    /*busqueda del flujo maximal sobre el camino aumentante*/
    PATH_ITER(dova->path, x, y){        
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
    node->level = LVL_NIL; 
    
    return node;
}

/*Destructor del dova*/
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
    int dir;
    u64 yName, flow, cap;//, eName,fName;
    bool breakW = false;
    
    assert(net != NULL && node != NULL);

    dir = nbrhd_getNext(node->nbrs, FST, UNK, &yName);

    while(dir != NONE && !breakW){
        HASH_FIND(hhNet, net, &yName, sizeof(yName), nextNode);
        if(nextNode->level != LVL_NIL && nextNode->level == (node->level+1)){ 
            flow = nbrhd_getFlow(node->nbrs, nextNode->name);        
            if(dir == FWD){
                cap = nbrhd_getCap(node->nbrs, nextNode->name);
                breakW = cap > flow;
            }else if(dir == BWD)
                breakW = flow > 0;
        }
        /* Si el actual vecino no es valido, se busca el siguiente*/
        if (dir == FWD && !breakW){
            dir = nbrhd_getNext(node->nbrs, NXT, dir, &yName);
            if (dir == NONE){   /*Se acabaron los FWD, sigo con los BWD*/
                dir = nbrhd_getNext(node->nbrs, FST, BWD, &yName);
            }
        }else if (dir == BWD && !breakW)
            dir = nbrhd_getNext(node->nbrs, NXT, dir, &yName);
    }
    
    if(!breakW && dir == NONE){
        /*no se encontro ningun vecino valido, anulo node para futuras busquedas*/
        node->level = LVL_NIL;
        nextNode = NULL;
    }    
    return nextNode;
}

/* Actualiza las distancias de los vecinos FWD y BWD de un nodo. 
 * Los vecinos que se actualizaron se agregan a la cola 'q'.
 * Precondicion: 
 */
static void set_lvlNbrs(DovahkiinP dova, Network *node, Queue q, int dir, int lvl){
    Network *yNode = NULL;
    bool canBeUsed = false;
    u64 yName, cap, flow;       /*nombre del vecino, capacidad y flujo con el*/

    assert(node != NULL && q != NULL);
    
    dir = nbrhd_getNext(node->nbrs, FST, dir, &yName);
    while(!IS_SET_FLAG(SINK_REACHED) && dir != NONE){
        flow = nbrhd_getFlow(node->nbrs, yName);
        cap = nbrhd_getCap(node->nbrs, yName);
        canBeUsed = (dir==FWD && cap > flow) || (dir==BWD && flow > 0);
       
        if (canBeUsed){                 /*busco el nodo y actualizo su nivel*/
            yNode = set_lvl(node, yName, lvl);
            if(yNode != NULL){
                queue_enqueue(q, yNode);
                if(yName == dova->sink)       /*llego a t*/
                    SET_FLAG(SINK_REACHED);
            }
        }
        dir = nbrhd_getNext(node->nbrs, NXT, dir, &yName);
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
    if(node->level == LVL_NIL)
        node->level = lvl;
    else
        node=NULL;
    return node;
}

