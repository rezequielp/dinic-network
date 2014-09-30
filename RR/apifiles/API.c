#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "__uthash.h"
#include "_stack.h"
#include "_queue.h"
#include "__bstrlib.h"
#include "_lexer.h"
#include "nbrhd.h"
#include "API.h"
#include "parser_lado.h"


#define LVL_NIL -1      /**<Valor nulo de distancia para los nodos.*/
#define DIR_NIL -1      /**<Valor nulo de dirección en el path para los nodos.*/    

/* Macro: Flags de permisos y estados.*/
#define SINK_REACHED    0b00010000      /**<Se llego a t.*/
#define MAXFLOW         0b00001000      /**<Es flujo maximal. Implica corte
                                            minimal.*/
#define SOURCE          0b00000100      /**<Fuente fijada.*/
#define SINK            0b00000010      /**<Resumidero fijado.*/
#define PATHUSED        0b00000001      /**<Camino usado para aumentar flujo.*/

/* Macro: Manejo de flags.*/
#define SET_FLAG(f) dova->flags |= f    /**<Activa(1) el bit de la flag f.*/
#define UNSET_FLAG(f) dova->flags &= ~f /**<Desactiva(0) el bit de la flag f.*/
#define CLEAR_FLAG() 0b00000001         /**<Inicializa todas las flags (reset).*/
#define IS_SET_FLAG(f) (dova->flags & f) > 0 /**<¿El bit de la flag f es 1?.*/


/* Macro: Iterador sobre un path.*/
/** Itera sobre los elementos de un path.*/
#define PATH_ITER(path, x, y)                                               \
    stack_resetViewer(path);                                               \
    y = stack_nextItem(path);                                               \
    x = stack_nextItem(path);                                               \
    for(; (x)!=NULL; (y)=(x), (x)=stack_nextItem(path))

/* Estructuras */
/** Estructura de un netwrok con la información de los nodos.
 * Contiene la información de un nodo: su nombre, quiénes son sus vecinos, 
 * su nivel de distancia en la búsqueda de caminos aumentantes y la dirección
 * con la que fue agregado por su ancestro en el camino aumentante actual. 
 * Estos nodos están registrados en una tabla hash: La del network que 
 * conforman, y en el corte minimal si es que forman parte.
 */
typedef struct NetworkSt{
    u64 name;                   /**<Hash key - nombre del nodo.*/
    Nbrhd nbrs;                 /**<Hash value - vecinos del nodo.*/
    int lvl;                    /**<Nivel de distancia del nodo.*/
    short int pDir;             /**<Dirección con su ancestro en el path.*/
    UT_hash_handle hhNet,hhCut; /**<Hace esta estructura hashable.*/
} Network;

/** Estructura de un Dovahkiin con los datos que se necesitan almacenar para 
 * ejecutar Dinic.
 * Contiene toda la información en el nivel más alto de ejecución, como el
 * acceso al network, el valor de flujo y corte minimal calculados, qué nodo 
 * es la fuente y cuál es resumidero. El último camino aumentante encontrado 
 * sin usar y la cantidad ya utilizados. También se almacenan las flags de 
 * estados que se necesiten en el transcurso de ejecución de DINIC.
 */
struct DovahkiinSt{
    Network *net;   /**<Network de los nodos para acceder a las aristas.*/
    u64 flow;       /**<Valor del flujo del dova.*/
    u64 src;        /**<Nodo fijado como fuente (s).*/
    u64 snk;        /**<Nodo fijado como resumidero (t).*/
    Network *cut;   /**<Corte minimal.*/
    Stack path;     /**<Camino de nodos, de s a t.*/
    u64 pCounter;   /**<Contador para la cantidad de caminos.*/
    int flags;      /**<Flags de estado, explicados en la sección define.*/
};


/* Funciones estáticas */
static u64 get_pathFlow(DovahkiinP dova);
static Network *network_create(u64 n);
static void network_destroy(Network *net);
static Network *network_nextNode(Network * net, Network *x, short int *dir);
static void set_lvlNbrs(DovahkiinP dova, Network *x, Queue q);

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
    dova->src = 0;
    dova->snk = 0;
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
    Network *node = NULL;   /*El i-esimo nodo en la hash del network*/
    Network *ref = NULL;    /*Lo necesita HASH_ITER para no perder referencias*/
    assert(dova!=NULL);
    /*Primero hay que destruir las entradas de la hash del corte.
    Si se destruyen primero los nodos entonces pierdo referencias*/
    if (dova->cut != NULL)
        HASH_CLEAR(hhCut, dova->cut);
    /*En este punto ya se puede destruir el network y todos los nodos.
    Esto también libera la hash del corte.*/
    if (dova->net != NULL)
        HASH_ITER(hhNet, dova->net, node, ref){
            /*Elimina la referencia en la hash del network*/
            HASH_DELETE(hhNet, dova->net, node);
            network_destroy(node);
        }
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
 * \pre \p dova debe ser un DovahkiinP no nulo.
 */
void FijarFuente(DovahkiinP dova, u64 s){
    assert(dova != NULL);
    dova->src = s;
    SET_FLAG(SOURCE);
}

/** Establece un nodo como resumidero del network.
 * \param dova  El dova en el que se trabaja.
 * \param t     El nombre del nodo.
 * \pre \p dova debe ser un DovahkiinP no nulo.
 */
void FijarResumidero(DovahkiinP dova, u64 t){
    assert(dova != NULL);
    dova->snk = t;
    SET_FLAG(SINK);
}

/** Imprime por la salida estándar el nombre del nodo que es fuente.
 * Imprime por pantalla:\n
 * Fuente: s \n
 * Donde s es el nodo que estamos considerando como fuente. Este es el único 
 * caso donde la fuente se imprimirá con su nombre real y no con la letra s. 
 * \param dova  El dova en el que se trabaja.
 * \pre \p dova debe ser un DovahkiinP no nulo.
 * \return  -1 si la fuente no esta fijada.\n
 *          0 caso contrario e imprime por pantalla.
 */
int ImprimirFuente(DovahkiinP dova){
    int result = -1;    /*Resultado de la operación.*/
    
    assert(dova != NULL);
    /*Imprimo solo si la fuente fue fijada*/
    if(IS_SET_FLAG(SOURCE)){
        printf("Fuente: %"PRIu64"\n", dova->src);
        result=0;
    }
    return result;
}

/** Imprime por la salida estándar el nombre del nodo que es resumidero.
 * Imprime por pantalla:\n
 * Resumidero: t \n
 * Donde t es el nodo que estamos considerando como resumidero. Este es el único 
 * caso donde el resumidero se imprimirá con su nombre real y no con la letra t. 
 * \param dova  El dova en el que se trabaja.
 * \pre \p dova debe ser un DovahkiinP no nulo.
 * \return  -1 si el resumidero no esta fijado.\n
 *          0 caso contrario e imprime por pantalla.
 */
int ImprimirResumidero(DovahkiinP dova){
    int result = -1;    /*Resultado de la operación.*/
    
    assert(dova != NULL);
    /*Imprimo solo si el resumidero fue fijado*/
    if(IS_SET_FLAG(SINK)){
        printf("Resumidero: %"PRIu64"\n", dova->snk);
        result=0;
    }
    return result;
}

/** Lee un lado desde la entrada estándar.
 * Lee una linea desde Standar Input que representa un lado y
 * devuelve el elemento de tipo Lado que lo representa si la linea es valida, 
 * sino devuelve el elemento LadoNulo. \n
 * Cada linea es de la forma x y c, siendo todos u64 representando el lado xy 
 * de capacidad c. 
 * \return  Un lado legal con los datos leídos.\n
 *          LadoNulo si la linea leída no es valida.
 */
Lado LeerUnLado(void){
    Lado edge = LadoNulo;   /*El lado con los datos leídos*/
    Lexer *input;           /*Analizador léxico por lineas de un archivo*/
    int clean = PARSER_OK;  /*Indica si no se encontró basura al parsear*/
   
    /*Construyo el lexer sobre la entrada estándar*/
    input = lexer_new(stdin);
    if (input != NULL){
        /*Leo un lado mientras no llegue a un fin de archivo o algún error*/
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
 * \pre \p dova Debe ser un DovahkiinP no nulo.
 * \return  1 si no hubo problemas.\n
 *          0 caso contrario.
 */
int CargarUnLado(DovahkiinP dova, Lado edge){
    Network *x = NULL;  /*Nodo 'x'*/
    Network *y = NULL;  /*Nodo 'y'*/
    u64 xName, yName;   /*Nombres del nodo 'x' e 'y'*/
    int result = 0;     /*Retorno*/
    
    assert(dova != NULL);
    
    if (edge != LadoNulo){
        xName = lado_getX(edge);
        /*Cargo el nodo 'x', si todavía no existe en el network*/
        HASH_FIND(hhNet, dova->net, &(xName), sizeof(xName), x);
        if (x == NULL){
            x = network_create(xName);
            HASH_ADD(hhNet, dova->net, name, sizeof(dova->net->name), x);
        }
        
        yName = lado_getY(edge);
        /*Cargo el nodo 'y', si todavía no existe en el network*/
        HASH_FIND(hhNet, dova->net, &(yName), sizeof(yName), y);
        if (y == NULL){
            y = network_create(yName);
            HASH_ADD(hhNet, dova->net, name, sizeof(dova->net->name), y);
        }
        /*Se establecen como vecinos. Arista 'xy'*/
        nbrhd_addEdge(x->nbrs, y->nbrs, edge);
        result = 1;
        lado_destroy(edge); /*Destruyo el lado, ya no nos sirve*/
    }
    
    return result;
}

/** Preprocesa el DovahkiinP para empezar a buscar caminos aumentantes. 
 * Aquí se debe chequear y preparar todo lo que sea necesario para comenzar
 * a buscar caminos aumentantes. \n
 * Por el momento solo hace falta chequear que estén seteados s y t, 
 * y que estos nodos existen en el network.
 * \param dova  El dova en el que se trabaja.
 * \pre \p dova Debe ser un DovahkiinP no nulo.
 * \return  1 si los preparativos fueron exitosos.\n
 *          0 caso contrario.
 */
int Prepararse(DovahkiinP dova){
    int status = 0;       /*Estado de los preparativos. Retorno*/
    Network *s = NULL;    /*nodo fuente 's' en el network*/
    Network *t = NULL;    /*nodo resumidero 't' en el network*/
    
    assert(dova != NULL);
    /*Si la fuente y el resumidero están fijados entonces chequeo que existen
     en el network*/
    if(IS_SET_FLAG(SINK) && IS_SET_FLAG(SOURCE)){
        /*Se buscan en el network*/
        HASH_FIND(hhNet, dova->net, &(dova->src), sizeof(dova->src), s);
        HASH_FIND(hhNet, dova->net, &(dova->snk), sizeof(dova->snk), t);
        if (s != NULL && t != NULL) 
            status = 1; /*Ambos existen, todo OK*/
    }
    return status;
}

/** Actualiza las distancias haciendo una búsqueda BFS-FF.
 * Se reinician todas las distancias a nulo y se comienzan a actualizar a partir
 * del nodo fuente 's' utilizando BFS-FF, hasta encontrarse con el
 * nodo resumidero 't'. \n
 * Si se alcanza 't' entonces implica un camino aumentante. Caso contrario, el
 * flujo actual es maximal y los nodos que fueron actualizados conforman el
 * corte minimal.
 * \param dova  El dova en el que se trabaja.
 * \pre \p dova Debe ser un DovahkiinP no nulo.
 * \return  1 si existe un camino aumentante entre 's' y 't'. \n
 *          0 caso contrario.
 */
int ActualizarDistancias(DovahkiinP dova){
    Queue q, qNext;         /*Colas para el manejo de los niveles. 
                            q = nodos del nivel actual; qNext = del siguiente*/
    Network *node = NULL;   /*Nodo actual de 'q' en el cual se itera*/
    Network *k = NULL;      /*Iterador para reseteo de las distancias*/

    assert(dova != NULL);
    
    /*Preparación de las cosas que voy a usar*/
    UNSET_FLAG(SINK_REACHED);
    HASH_CLEAR(hhCut, dova->cut);
    q = queue_create();
    qNext = queue_create();
    
    /* Reset de distancias por llamadas anteriores*/
    for(k = dova->net; k != NULL; k = k->hhNet.next){
        k->lvl = LVL_NIL;
    }    
    /*La fuente es nivel 0*/
    HASH_FIND(hhNet, dova->net, &(dova->src), sizeof(dova->src), node);
    assert(node != NULL);
    node->lvl = 0;
    queue_enqueue(q, node);

    /*Actualización de distancias por BFS */
    while(!queue_isEmpty(q) && !IS_SET_FLAG(SINK_REACHED)){
        node = queue_head(q);
        /*Actualización de niveles de distancia de los nodos vecinos de 'node'*/
        set_lvlNbrs(dova, node, qNext);
        /*Agrego 'node' al corte*/
        HASH_ADD(hhCut, dova->cut, name, sizeof(dova->cut->name), node);
        queue_dequeue(q);
        /*Si se terminaron los nodos de este nivel, se pasa al siguiente*/
        if(queue_isEmpty(q))
            queue_swap(&q, &qNext);
    }
    /*Si se alcanzo 't'*/
    if(IS_SET_FLAG(SINK_REACHED)){
        HASH_CLEAR(hhCut, dova->cut);   /*EL corte no es minimal, no me sirve*/
    }else
        SET_FLAG(MAXFLOW);  /*El flujo es maximal => hay corte minimal*/

    queue_destroy(q, NULL);
    queue_destroy(qNext, NULL);
    return IS_SET_FLAG(SINK_REACHED);
}

/** Hace una búsqueda DFS-FF de un camino aumentante de menor longitud.
 * Solo se utilizan los nodos que tengan su distancia actualizada.
 * El ultimo nodo agregado al camino solo agrega a otro nodo si este ultimo 
 * tiene una distancia +1 que el, y si se puede aumentar (o disminuir) flujo 
 * entre ellos.
 * El nodo es anulado si no tiene un siguiente a quien enviar flujo.
 * \note El camino al ser el resultado de un DFS es una pila y se usa el nodo 
 * top para avanzar hacia 't'. Si este nodo no tiene vecinos por los que se 
 * pueda avanzar entonces se quita de la pila y se bloquea para que no se vuelva
 * a usar. Se continua intentando avanzar de esta manera hasta llegar a 't' o
 * hasta que no queden mas nodos en la pila.
 * \param dova  El dova en el que se trabaja.
 * \pre \p dova Debe ser un DovahkiinP no nulo.
 * \return  1 si llega a 't'. \n
 *          0 caso contrario.
 */
int BusquedaCaminoAumentante(DovahkiinP dova){
    Network *x = NULL;      /*Ultimo nodo agregado al path (ancestro)*/
    Network *y = NULL;      /*Nodo candidato a agregarse al path*/
    int t_reached = 0;      /*Indica si se alcanzó 't'*/
    short int dir;          /*Dirección de un nodo con su ancestro en el path*/
    /*Network *k = NULL;*/  /*Iterador para reseteo de direcciones*/
    
    assert(dova != NULL);
    /* Reset de direcciones(solo sirve por motivos de debuggin y aumenta el 
     *tiempo de procesamiento)*/
    /*for(k = dova->net; k != NULL; k = k->hhNet.next){
        k->pDir = DIR_NIL;
    }*/
    
    /*No se busca un nuevo camino aumentante si uno anterior encontrado todavía
     no se uso para aumentar flujo*/
    if (IS_SET_FLAG(PATHUSED)){
        if (dova->path != NULL)
            stack_destroy(dova->path, NULL);    /*Destruyo el path viejo*/
        dova->path = stack_create();
        /*Busco a 's' y lo agrego al path (notar 'x' = 's')*/
        HASH_FIND(hhNet, dova->net, &(dova->src), sizeof(dova->src), x); 
        stack_push(dova->path, x);
        while(!stack_isEmpty(dova->path) && (x->name != dova->snk)){
            /*Busco el siguiente nodo que cualifica para el camino aumentante*/
            y = network_nextNode(dova->net, x, &dir);
            if(y != NULL){
                /*Se agrega al camino con la dirección que es agregado*/
                y->pDir = dir;
                stack_push(dova->path, y);
                x = y;
            }else{  
                /*No se puede avanzar. Bloqueo y pruebo con el próximo top.*/
                x->lvl = LVL_NIL;
                stack_pop(dova->path);
                x = (Network*)stack_top(dova->path); 
            }
        }
    }
    /*Si se encontró camino aumentante es que llego a 't'*/
    if (dova->path != NULL && !stack_isEmpty(dova->path)){
        t_reached = ((Network*)stack_top(dova->path))->name == dova->snk;
        if (t_reached) /*El path se puede usar para aumentar flujo*/
            UNSET_FLAG(PATHUSED); 
    }
    return t_reached;
}

/** Aumenta el flujo del network.
 * Actualiza el flujo en el network de \p dova sobre el camino aumentante 
 * encontrado. La actualización es por el máximo aumento de flujo que se pueda 
 * enviar por ese camino, teniendo en cuenta flujos anteriores.
 * \param dova  El dova en el que se trabaja.
 * \pre \p dova Debe ser un DovahkiinP no nulo.\n
 *      Se busco y encontró un camino aumentante que todavía no se ha usado para
 *      actualizar el flujo.
 * \return  Valor por el cual se aumenta el flujo, si no hubo errores.
 *          0 si hubo error o no se cumple la precondición de camino aumentante.
 */
u64 AumentarFlujo(DovahkiinP dova){
    u64 pflow = 0;      /*Flujo a enviar por el camino aumentante*/ 
    Network *x = NULL;  /*Nodo 'x' de una arista 'xy'. Ancestro de 'y'*/
    Network *y = NULL;  /*Nodo 'y' de una arista 'xy'*/
    
    assert(dova != NULL);
    /*Precondicion de que el camino no se uso para aumentar flujo*/
    if (!IS_SET_FLAG(PATHUSED)){
        pflow = get_pathFlow(dova);
        /*Incremento del flujo sobre cada arista 'xy'.*/
        PATH_ITER(dova->path, x, y){
            /*por BWD se reduce el flujo sobre esta arista*/
            nbrhd_increaseFlow(x->nbrs, y->name, y->pDir, pflow);
        }
        dova->flow += pflow; 
        dova->pCounter++;
    }
    SET_FLAG(PATHUSED);
    return pflow;
}

/** Idem AumentarFlujo() pero también imprime el camino por el Estandar Output.
 * Imprime el camino con el formato: \n
 * \verbatim camino aumentante #:
   t;x_r;...;x_1;s: <cantDelIncremento>
   \endverbatim
 * Donde '#' es el numero del camino aumentante, ';' se usa en caminos forward 
 * y '>' en backward.
 * \param dova  El dova en el que se trabaja.
 * \pre \p dova Debe ser un DovahkiinP no nulo.\n
 * \return  Valor por el cual se aumenta el flujo, si no hubo errores.
 *          0 si hubo error o no se cumple la precondición de camino aumentante.
 */
u64 AumentarFlujoYTambienImprimirCamino(DovahkiinP dova){
    u64 pflow = 0;      /*Flujo a enviar por el camino aumentante*/ 
    Network *x = NULL;  /*Nodo 'x' de una arista 'xy'. Ancestro de 'y'*/
    Network *y = NULL;  /*Nodo 'y' de una arista 'xy'*/
    
    assert(dova != NULL);
    /*precondición de que el camino no se uso para aumentar flujo*/
    if (!IS_SET_FLAG(PATHUSED)){
        /*Aumento el flujo*/
        pflow = AumentarFlujo(dova);
        /*Imprimo el camino*/ 
        printf("camino aumentante %"PRIu64":\nt", dova->pCounter );      
        PATH_ITER(dova->path, x, y){
            if(y->pDir == FWD){
                if(x->name != dova->src)
                    printf(";%"PRIu64, x->name);
                else
                    printf(";s");
            }else
                printf(">%"PRIu64, x->name);
        }
        printf(": <%"PRIu64">\n", pflow);
    }
    SET_FLAG(PATHUSED);
    return pflow;
}

/** Imprime por Estandar Output el Flujo hasta el momento. 
 * Imprime con el formato: \n
 * \verbatim Flujo �:
   Lado x_1,y_2: <FlujoDelLado>
   \endverbatim
 * Donde � es "maximal" si el flujo es maximal o "no maximal" caso contrario.
 * \param dova  El dova en el que se trabaja.
 * \pre \p dova Debe ser un DovahkiinP no nulo.
 */
void ImprimirFlujo(DovahkiinP dova){
    Network *x = NULL;      /*Nodo actual de la iteración.*/
    Network *ref = NULL;    /*Lo necesita HASH_ITER para no perder referencias*/
    u64 yName;              /*Nombre del nodo 'y', vecino del nodo 'x'*/
    u64 vflow = 0;          /*Valor del flujo del lado a imprimir*/
    int rqst;               /*Manejo de rqst para la búsqueda de nodos vecinos*/
    
    assert(dova != NULL);
    
    if(IS_SET_FLAG(MAXFLOW))
        printf("Flujo maximal:\n");
    else
        printf("Flujo no maximal:\n");
    /*Itero sobre cada nodo del network.*/
    HASH_ITER(hhNet, dova->net, x, ref){
        rqst = FST;
        /*Imprimo todos los lados 'xy' hasta acabar los vecinos*/
        while(nbrhd_getFwd(x->nbrs, rqst, &yName)){
            vflow = nbrhd_getFlow(x->nbrs, yName, FWD);
            printf("Lado %"PRIu64",%"PRIu64": %"PRIu64"\n",x->name, yName, 
                   vflow);
            rqst = NXT;
        }
    }
    printf("\n");
}

/** Imprime por Estandar Output el valor del Flujo calculado hasta el momento. 
 * Imprime con el formato: \n
 * \verbatim Valor del flujo �: <ValorDelFlujo> \endverbatim
 * Donde � es "maximal" si el flujo es maximal o "no maximal" caso contrario.
 * \param dova  El dova en el que se trabaja.
 * \pre \p dova Debe ser un DovahkiinP no nulo.
 */
void ImprimirValorFlujo(DovahkiinP dova){
    assert(dova != NULL);
    if(IS_SET_FLAG(MAXFLOW))
        printf("Valor del flujo maximal: %"PRIu64"\n", dova->flow);
    else
        printf("Valor del flujo no maximal: %"PRIu64"\n", dova->flow);
}

/** Imprime por Estandar Output un corte minimal y su capacidad.
 * La capacidad del corte minimal es igual a la suma de todas las capicades
 * de las aristas que van desde el corte a su complemento. Este valor debe
 * ser igual al valor del flujo maximal. Aquí no se compara dicha igualdad, pero
 * si se calcula la capacidad del corte como se enuncia.
 * Imprime con el formato:
 * \verbatim Corte Minimial: S = {s,x_1,...}
   Capacidad: <Capacidad>
   \endverbatim
 * Donde � es "maximal" si el flujo es maximal o "no maximal" caso contrario.
 * \param dova  El dova en el que se trabaja.
 * \pre \p dova Debe ser un DovahkiinP no nulo. \n
        Debe existir corte minimal. ie, debe haberse llegado a flujo maximal.
 */  
void ImprimirCorte(DovahkiinP dova){
    Network *x = NULL;      /*Nodo que pertenece al corte*/
    Network *y = NULL;      /*Nodo vecino de 'x'*/
    Network *ref = NULL;    /*Lo necesita HASH_ITER para no perder referencias*/
    u64 yName;              /*Nombre del nodo 'y'*/
    u64 capCut = 0;         /*Capacidad del corte*/
    int rqst;               /*Para establecer pedidos de busqueda de vecinos*/
    
    assert(dova!=NULL);
    assert(dova->cut!=NULL && IS_SET_FLAG(MAXFLOW));
    
    printf("Corte Minimal: S = {s");
    /*Itero sobre cada nodo del corte.*/
    HASH_ITER(hhCut, dova->cut, x, ref){
        rqst = FST;
        /*Itero sobre los vecinos de 'x'*/
        while(nbrhd_getFwd(x->nbrs, rqst, &yName)){
            /*Me fijo si el vecino 'y' esta en el corte*/
            HASH_FIND(hhCut, dova->cut, &yName, sizeof(yName), y);
            if(y == NULL) /*Esta en el complemento. Sumo la capacidad*/
                capCut += nbrhd_getCap(x->nbrs, yName, FWD);
            rqst = NXT;
        }
        if(x->name != dova->src)
            printf(",%"PRIu64, x->name);
    }
    printf("}\n");
    printf("Capacidad: %"PRIu64"\n", capCut);
}



/* 
 *          Funciones estáticas
*/

/** Busca el máximo valor de flujo que se puede enviar por un camino sin usar.
 * \param dova  El dova en el que se trabaja.
 * \pre \p dova Debe ser un DovahkiinP no nulo. \n.
 *      El camino aumentante todavía no se uso para aumentar flujo.
 * \return  Valor por el cual se aumenta el flujo.
 */
static u64 get_pathFlow(DovahkiinP dova){
    u64 pflow = u64_MAX;    /*valor de flujo de retorno*/
    u64 cap, flow;          /*capacidad y valor de flujo*/
    Network *x = NULL;      /*nodo 'x'*/
    Network *y = NULL;      /*nodo 'y'*/
    
    assert(dova != NULL);
    assert(!IS_SET_FLAG(PATHUSED));
    /*Itero en el camino. 'x' es ancestro de 'y'*/
    PATH_ITER(dova->path, x, y){
        flow = nbrhd_getFlow(x->nbrs, y->name, y->pDir);
        /*El calculo depende de la dirección en la que fue agregado 'y'*/
        if(y->pDir == FWD){
            /*'y' es vecino forward.*/
            cap = nbrhd_getCap(x->nbrs, y->name, FWD);
            pflow = u64_min(pflow, cap - flow);
        }else
            /*'y' es vecino backward.*/
            pflow = u64_min(pflow, flow);
    }
    return pflow;
}

/** Creador de un nuevo Network.
 * \param n El nombre del nodo.
 * \return Un Network con nombre de nodo 'n'.
 */
static Network *network_create(u64 n){
    Network *node;
    
    node = (Network*) malloc(sizeof(Network));
    assert(node != NULL);
    
    node->name = n;
    node->nbrs = nbrhd_create();
    node->lvl = LVL_NIL;
    node->pDir = DIR_NIL;
    
    return node;
}

/** Destructor de un Network.
 * \pre El Network no es nulo.
 * \param net El Network a destruir.
 */
static void network_destroy(Network *net){
    assert(net!=NULL);
    nbrhd_destroy(net->nbrs);
    free(net);
}

/** Busca el siguiente nodo que cumple las condiciones de envío de flujo.
 * En relación con un nodo, se busca un siguiente que sea vecino de este y que
 * entre ellos haya posibilidad de enviar flujo. La búsqueda es prioritaria por
 * forward, si no encuentra ninguno entonces intenta por backward. \n
 * \param net El Network que los nodos pertenecen.
 * \param x El nodo ancestro.
 * \param dir Puntero donde almacenar la dirección que se encuentra el vecino (FWD o BWD).
 * \pre El network y el nodo no son nulos.
 * \return  Puntero al nodo encontrado. \n
 *          NULL si no hay un siguiente que cualifique.
*/
static Network *network_nextNode(Network *net, Network *x, short int *dir){
    Network *y = NULL;    /*El nodo candidato a ser el siguiente. Retorno*/
    int getNbr;           /*Resultado de los nbrhd_getX()*/
    u64 yName;            /*Nombre del nodo 'y'*/
    u64 flow, cap;        /*Flujo y capacidad entre 'x' e 'y'*/
    bool breakW = false;  /*Termina la búsqueda iterativa*/
    
    assert(net != NULL && x != NULL);
    /*Inicio con el primer vecino que encuentre*/
    getNbr = nbrhd_getFwd(x->nbrs, FST, &yName);
    if(!getNbr){
        getNbr = nbrhd_getBwd(x->nbrs, FST, &yName);
        *dir = BWD;
    }else
        *dir = FWD;
        
    while(getNbr && !breakW){
        HASH_FIND(hhNet, net, &yName, sizeof(yName), y);
        assert(y!=NULL);
        /*Compruebo si su distancia es mayor por 1 unidad*/
        if(y->lvl != LVL_NIL && y->lvl == (x->lvl+1)){ 
            flow = nbrhd_getFlow(x->nbrs, yName, *dir);
            cap = nbrhd_getCap(x->nbrs, yName, *dir);
            /*Compruebo el envio de flujo. True = Cumple lo requerido! salgo*/
            breakW = (*dir == FWD && cap > flow) || (*dir == BWD && flow > 0);
        }
        /*Si no es valido, se descarta y busca otro (prioridad por FWD)*/
        if(*dir == FWD && !breakW){
            getNbr = nbrhd_getFwd(x->nbrs, NXT, &yName);
            if(!getNbr){   /*Se acabaron los FWD, sigo con los BWD*/
                getNbr = nbrhd_getBwd(x->nbrs, FST, &yName);
                *dir = BWD;
            }
        }else if(*dir == BWD && !breakW)
            getNbr = nbrhd_getBwd(x->nbrs, NXT, &yName);
    }

    if(!breakW && !getNbr)
        y = NULL;   /*Ningún vecino cualifica*/
    
    return y;
}

/** Actualiza los niveles de distancias de los vecinos forward y luego los
 * backward de un nodo.
 * Solo se actualizan vecinos que todavía no tienen su nivel fijado (ie, su 
 * nivel es nulo) y por el que se pueda enviar flujo. Tendrán un nivel de 
 * distancia +1 respecto a su ancestro, y se agregan a una cola que el llamador 
 * debe proporcionar y se encarga de liberar.
 * \param dova  El dova en el que se trabaja.
 * \param x     Nodo ancestro.
 * \param upd   Cola en la que se agregan vecinos que se actualicen.
 * \pre 'dova', 'x' y 'upd' No deben ser nulos.
 */
static void set_lvlNbrs(DovahkiinP dova, Network *x, Queue upd){
    Network *y = NULL;      /*Nodo 'y' vecino de 'x' por actualizar*/
    int getNbr;             /*Resultado de los nbrhd_getX()*/
    int dir = FWD;          /*Dirección en la que se encuentra el vecino*/
    u64 yName, cap, flow;   /*Nombre del nodo 'y', capacidad y flujo con 'x'*/
    
    assert(dova != NULL);
    assert(x != NULL && upd != NULL);
    
    /*Inicio con el primer vecino que encuentre*/
    getNbr = nbrhd_getFwd(x->nbrs, FST, &yName);
    if(!getNbr){
        getNbr = nbrhd_getBwd(x->nbrs, FST, &yName);
        dir = BWD;
    }
    /*No paro hasta que alcance 't' o se me acaben los vecinos*/
    while(getNbr && !IS_SET_FLAG(SINK_REACHED)){
        flow = nbrhd_getFlow(x->nbrs, yName, dir);
        cap = nbrhd_getCap(x->nbrs, yName, dir);
        /*Según su dirección, compruebo el envío de flujo*/
        if((dir == FWD && cap > flow) || (dir == BWD && flow > 0)){
            /*Busco el nodo y actualizo su nivel (si todavía no lo hizo)*/
            HASH_FIND(hhNet, dova->net, &yName, sizeof(yName), y);
            assert(y != NULL);
            if(y->lvl == LVL_NIL){
                y->lvl = x->lvl + 1;
                queue_enqueue(upd, y);
                if(yName == dova->snk)  /*Se llego a t*/
                    SET_FLAG(SINK_REACHED);
            }
        }
        /*Busco el siguiente vecino*/
        if(dir == FWD && !IS_SET_FLAG(SINK_REACHED)){
            getNbr = nbrhd_getFwd(x->nbrs, NXT, &yName);
            if(!getNbr){   /*Se acabaron los FWD, sigo con los BWD*/
                getNbr = nbrhd_getBwd(x->nbrs, FST, &yName);
                dir = BWD;
            }
        }else if(dir == BWD && !IS_SET_FLAG(SINK_REACHED))
            getNbr = nbrhd_getBwd(x->nbrs, NXT, &yName);
    }
}
