#include "nbrhd.h"
#include "__uthash.h"
#include <stdlib.h>
#include <assert.h>

/** \file nbrhd.c
 * Las estructuras Nbrhd, Fedge, Bedge y todas sus funciones se definen aquí.
 * \note
 * Para los comentarios y documentación, tener en cuenta la notación enunciada
 * en nbrhd.h
 */


/** Estructura de una artista en sentido forward.
 * Relacion 'xy' (y es nodo forward de x).
 */
typedef struct FedgeSt{
    u64 y;                  /**<Nodo forward de 'x'. Es key de la hash.*/
    u64 cap;                /**<La capacidad de envio de flujo.*/
    u64 flow;               /**<El flujo que se esta enviando.*/
    UT_hash_handle hhfNbrs; /**<La tabla hash.*/
} Fedge;

/** Estructura de una arista en sentido backward.
 * Relacion 'yx' (y es nodo backward de x).
 * Ej: x=3; y=2; Lado 'yx'=23. 2 es backward de 3, y 3 es forward de 2, 
 * por lo que existe para el nodo 2 (como ancestro), una estructura Fedge para 3 
 * en su vecindad. Esta estructura es a la que apunta *x.
 */
typedef struct BedgeSt{
    u64 y;              /**<Nodo backward de 'x'. Es key de la hash.*/
    Fedge *x;           /**<Referencia al nodo 'y' como forward del nodo 'x'*/
    UT_hash_handle hhbNbrs; /**< La tabla hash.*/
} Bedge;

/** Estructura Nbhd de la vecindad de un nodo 'x'.*/
struct NeighbourhoodSt{
    Fedge *fNbrs;       /**<Vecinos forward. Hash de vecinos forward de 'x'*/
    Bedge *bNbrs;       /**<Vecinos backward. Hash de vecinos backward de 'x'*/
};

/*Funciones estaticas */
static void *findNbr(Nbrhd nbrs, u64 y, short int dir);
static Fedge *fedge_create(u64 y, u64 c);
static Bedge *bedge_create(u64 y, Fedge *fNbr);
static void fedge_destroy(Fedge *fNbrs);
static void bedge_destroy(Bedge *bNbrs);


/** Constructor de un nuevo Nbrhd.
 * \return un \p Nbrhd vacío.
 */
Nbrhd nbrhd_create(void){
    Nbrhd nbrs = NULL;  /*nuevo Nbrhd*/
    /*asigno memoria*/
    nbrs = (Nbrhd) malloc(sizeof(struct NeighbourhoodSt));
    assert(nbrs != NULL);
    /*inicializo campos*/
    nbrs->fNbrs = NULL;
    nbrs->bNbrs = NULL;
    
    return nbrs;
}

/** Destructor de un Nbrhd.
 * \pre \p Nbrhd no es nulo.
 */
void nbrhd_destroy(Nbrhd nbrs){
    assert(nbrs != NULL);
    /*Destruyo todos los vecinos forward*/
    if (nbrs->fNbrs != NULL)
        fedge_destroy(nbrs->fNbrs);
    /*Destruyo todos los vecinos backward*/
    if (nbrs->bNbrs != NULL)
        bedge_destroy(nbrs->bNbrs);    
    free(nbrs);
    nbrs = NULL;
}

/** Genera el vinculo entre 'x' e 'y' (edge) convirtiendolos en vecinos.
 * La relacion es 'xy': 'y' vecino forward de 'x'; 'x' vecino backward de 'y'
 * \pre 'x', 'y', 'edge' no son nulos.
 * \param x Vecindario del nodo 'x'.
 * \param y Vecindario del nodo 'y'.
 * \param edge Lado 'xy' con su capacidad y flujo.
 */
void nbrhd_addEdge(Nbrhd x, Nbrhd y, Lado edge){
    Fedge *fNbr = NULL;     /*Vecino forward*/
    Bedge *bNbr = NULL;     /*Vecino backward*/
    u64 yName;              /*Nombre del nodo 'y'*/
    
    assert(x!=NULL && y!=NULL && edge!=LadoNulo);
    
    /*Se busca si ya existe una entrada para el lado 'xy'*/
    yName = lado_getY(edge);
    HASH_FIND(hhfNbrs, x->fNbrs, &(yName), sizeof(x->fNbrs->y), fNbr);
    /*Si no existe, creo las estructuras correspondientes*/
    if(fNbr == NULL){
        /*Creo a 'y' como vecino forward de 'x'*/
        fNbr = fedge_create(yName, lado_getCap(edge));
        HASH_ADD(hhfNbrs, x->fNbrs, y, sizeof(x->fNbrs->y), fNbr);
        
        /*Creo a 'x' como vecino backward de 'y'*/
        bNbr = bedge_create(lado_getX(edge), fNbr);
        HASH_ADD(hhbNbrs, y->bNbrs, y, sizeof(y->bNbrs->y), bNbr);
    /*Si existe, es un caso de lados paralelos*/
    }else{
        fNbr->cap += lado_getCap(edge);
    }
        
}

/** Busca el siguiente vecino forward.
 * La peticion de busqueda puede ser por el primer nodo (FST) de la tabla, 
 * o bien por el siguiente(NXT) del ultimo pedido. 
 * Si existe almacena el nombre en 'y'.
 * 
 * \note Tener en cuenta la documentacion sobre las opciones de los parametros.
 * Verlo como un iterador de consultas a una tabla.
 * 
 * \param nbrs  El vecindario del nodo ancestro 'x'. 
 * \param rqst  Si se pide el primero 'FST' o un siguiente 'NXT'.
 * \param y     Variable en la que se almacena el nombre del vecino encontrado.
 * \pre 'nbrs' e 'y' no son nulos y 'rqst' es una opcion valida (FST o NXT)
 * \return 1 Si se encontro y se almaceno en 'y' un vecino.\n
 *         0 Caso contrario.
 */
int nbrhd_getFwd(Nbrhd nbrs, int rqst, u64 *y){
    static Fedge *fNbr = NULL;  /*Puntero al ultimo vecino forward consultado*/
    int result = 0;             /*Valor de retorno*/
    
    assert(nbrs != NULL && y != NULL);
    assert(rqst == FST || rqst == NXT);
    
    if(nbrs->fNbrs != NULL){
        /*Se asigna el primero de la tabla*/
        if(rqst == FST){
            fNbr = nbrs->fNbrs;
            *y = fNbr->y;
            result = 1;
        /*Se pide el siguiente. El ultimo pedido esta referenciado en 'fNbr'*/
        }else{
            if(fNbr != NULL){
                /*Se asigna el sig de la tabla, NULL si el anterior fue el ultimo*/
                fNbr = fNbr->hhfNbrs.next;
                if(fNbr != NULL){
                    *y = fNbr->y;
                    result = 1;
                }
            }
        }
    }
    return result;   
}

/** Busca el siguiente vecino backward.
 * La peticion de busqueda puede ser por el primer nodo (FST) de la tabla, 
 * o bien por el siguiente(NXT) del ultimo pedido. 
 * Si existe almacena el nombre en 'y'.
 * 
 * \note Tener en cuenta la documentacion sobre las opciones de los parametros.
 * Verlo como un iterador de consultas a una tabla.
 * 
 * \param nbrs  El vecindario del nodo ancestro 'x'. 
 * \param rqst  Si se pide el primero 'FST' o un siguiente 'NXT'.
 * \param y     Variable en la que se almacena el nombre del vecino encontrado.
 * \pre 'nbrs' e 'y' no son nulos y 'rqst' es una opcion valida (FST o NXT)
 * \return 1 Si se encontro y se almaceno en 'y' un vecino.\n
 *         0 Caso contrario.
 */
int nbrhd_getBwd(Nbrhd nbrs, int rqst, u64 *y){
    static Bedge *bNbr = NULL;  /*Puntero al ultimo vecino backward consultado*/
    int result = 0;             /*Valor de retorno*/
    
    assert(nbrs != NULL && y != NULL);
    assert(rqst == FST || rqst == NXT);
    
    if(nbrs->bNbrs != NULL){
        /*Se asigna el primero de la tabla*/
        if(rqst == FST){
            bNbr = nbrs->bNbrs;
            *y = bNbr->y;
            result = 1;
        /*Se pide el siguiente. El ultimo pedido esta referenciado en 'bNbr'*/
        }else{
            if(bNbr != NULL){
                /*Se asigna el sig de la tabla, NULL si el anterior fue el ultimo*/
                bNbr = bNbr->hhbNbrs.next;
                if(bNbr != NULL){
                    *y = bNbr->y;
                    result = 1;
                }
            }
        }
    }
    return result;   
}

/** Se aumenta el flujo para con el vecino 'y' por 'vf' cantidad. 
 * Si 'y' es un vecino BWD, el valor del flujo se disminuye por 'vf' cantidad.
 * \param nbrs  El vecindario del nodo ancestro 'x'. 
 * \param y     El nombre del vecino.
 * \param vf    El valor de flujo.
 * \pre 'y' es vecino de 'x'. 'vf' > 0
 * \return Valor del nuevo flujo que se esta enviando entre 'x' e 'y'.*/
u64 nbrhd_increaseFlow(Nbrhd nbrs, u64 y, short int dir, u64 vf){
    Fedge *fNbr = NULL; /*Para manipular el lado si 'dir'=FWD*/
    Bedge *bNbr = NULL; /*Para manipular el lado si 'dir'=BWD*/
    void *nbr = NULL;   /*El vecino*/

    assert(nbrs != NULL);
    assert(dir == FWD || dir == BWD);
    assert(vf > 0);  
    
    nbr = findNbr(nbrs, y, dir); /*Se busca*/
    if(dir == FWD){    /*Es FWD, aumento el flujo*/
        fNbr = (Fedge*)nbr;
        fNbr->flow += vf;
        assert(fNbr->flow <= fNbr->cap);
    }else{  /*Es BWD, reduzco el flujo en esta arista*/
        bNbr = (Bedge*)nbr;
        fNbr = bNbr->x;
        assert(fNbr->flow >= vf);
        fNbr->flow -= vf;
        
    }

    return fNbr->flow;
}

/** Devuelve la capacidad del lado que relaciona al nodo ancestro 'x' con el 
 * vecino 'y'.
 * Como pueden haber loops hay que especificar si se esta tratando del lado 'xy'
 * o 'yx'.
 * \param nbrs  El vecindario del nodo ancestro 'x'. 
 * \param y     El nombre del vecino.
 * \param dir   Direccion que se encuentra el vecino (lado FWD o BWD)
 * \pre 'y' es vecino de 'x'. 
 * \return  La capacidad sobre este lado.
 */
u64 nbrhd_getCap(Nbrhd nbrs, u64 y, short int dir){
    void *nbr = NULL;   /*El vecino*/
    
    assert(nbrs != NULL);
    assert(dir == FWD || dir == BWD);

    nbr = findNbr(nbrs, y, dir); /*Se busca*/
    if(dir == BWD)  /*Es backward, se maneja distinto*/
        nbr = ((Bedge*)nbr)->x;
    
    return ((Fedge*)nbr)->cap;
}

/** Devuelve el valor del flujo del lado que relaciona al nodo ancestro 'x' con 
 * el vecino 'y'.
 * Como pueden haber loops hay que especificar si se esta tratando del lado 'xy'
 * o 'yx'.
 * \param nbrs  El vecindario de 'x'. 
 * \param y     El nombre del vecino.
 * \param dir   Direccion que se encuentra el vecino (lado FWD o BWD)
 * \pre 'y' es vecino de 'x'. 
 * \return  El valor del flujo sobre este lado.
 */
u64 nbrhd_getFlow(Nbrhd nbrs, u64 y, short int dir){
    void *nbr = NULL;   /*El vecino*/
    
    assert(nbrs != NULL);
    assert(dir == FWD || dir == BWD);

    nbr = findNbr(nbrs, y, dir); /*Se busca*/
    if(dir == BWD)  /*Es backward, se maneja distinto*/
        nbr = ((Bedge*)nbr)->x;
    
    return ((Fedge*)nbr)->flow;
}

/*
 *          Funciones locales
 */

/** Busca un vecino con nombre 'y', en la dirección especificada.
 * \param nbrs  El vecindario de 'x'.
 * \param y     El nombre del vecino.
 * \pre 'y' es vecino de 'x'.
 * \return  La representación del lado dependiendo de la dirección 
 *          en la que se encuentra 'y': \n
 *          Si es forward, se retorna un puntero a un Fedge. \n
 *          Si es backward, se retorna un puntero a un Bedge.
 */
static void *findNbr(Nbrhd nbrs, u64 y, short int dir){
    Fedge *fNbr = NULL;     /*Para manipular el lado si 'dir'=FWD*/
    Bedge *bNbr = NULL;     /*Para manipular el lado si 'dir'=BWD*/
    void *result = NULL;    /*El lado de retorno, fNbr o bNbr*/
    
    assert(nbrs != NULL);
    assert(dir == FWD || dir == BWD);
    if(dir == FWD){  /*Busqueda por vecinos forward*/
        HASH_FIND(hhfNbrs, nbrs->fNbrs, &(y), sizeof(y), fNbr);
        assert(fNbr != NULL);    /*Debe existir*/
        result = fNbr;
    }
    else{                  /*Busqueda por vecinos backward*/
        HASH_FIND(hhbNbrs, nbrs->bNbrs, &(y), sizeof(y), bNbr);
        assert(bNbr != NULL);   /*Debe existir*/
        result = bNbr;
    }
        
    return result;
}

/** Construye un nuevo vecino forward (Fedge) para el nodo ancestro 'x'. 
 * El valor del flujo se inicia en 0.
 * \param y Nombre del nuevo vecino forward.
 * \param c Capacidad de envio de flujo a este vecino.
 * \return Puntero a la estructura que representa el nuevo vecino forward.\n
 *         El llamador se encarga de liberarlo.
*/
static Fedge *fedge_create(u64 y, u64 c){
    Fedge *fNbr = NULL;     /*Vecino forward*/
    
    fNbr = (Fedge *) malloc(sizeof(struct FedgeSt));
    assert(fNbr != NULL);
    fNbr->y = y;        
    fNbr->cap = c;
    fNbr->flow = 0;
    
    return fNbr;
}


/** Construye un nuevo vecino backward para el nodo ancestro 'x'. 
 * Se vincula a los datos forward respecto a él.
 * \param y Nombre del nuevo vecino backward.
 * \param fNbr Puntero al lado en donde el nodo 'y' es forward del nodo 'x'.
 * \return Puntero a la estructura que representa el nuevo vecino backward.\n
 *          El llamador se encarga de liberarlo.
*/
static Bedge *bedge_create(u64 y, Fedge *fNbr){
    Bedge *bNbr = NULL;     /*Vecino backward*/
    
    bNbr = (Bedge*) malloc(sizeof(struct BedgeSt));
    assert(bNbr != NULL);
    bNbr->y = y;          
    bNbr->x = fNbr;
    
    return bNbr;
}


/** Destructor de vecinos forwards.
 * \param fNbrs Puntero a la hash de vecinos forward
 * \pre \p fNbrs no es nulo.
 */
static void fedge_destroy(Fedge *fNbrs){
    Fedge *felem = NULL;    /*El i-esimo elemento de fNbrs*/
    Fedge *feTmp = NULL;    /*El i-esimo+1 elemento de fNbrs*/
    
    assert(fNbrs != NULL);
    HASH_ITER(hhfNbrs, fNbrs, felem, feTmp){
        HASH_DELETE(hhfNbrs, fNbrs, felem);
        free(felem);
    }    
    free(fNbrs);
    fNbrs = NULL;
}


/** Destructor de vecinos backwards.
 * \param bNbrs Puntero a la hash de vecinos forward
 * \pre \p bNbrs no es nulo.
 */
static void bedge_destroy(Bedge *bNbrs){
    Bedge *belem = NULL;    /*El i-esimo elemento de bNbrs*/
    Bedge *beTmp = NULL;    /*El i-esimo+1 elemento de bNbrs*/
    
    assert(bNbrs != NULL);
    HASH_ITER(hhbNbrs, bNbrs, belem, beTmp){
        HASH_DELETE(hhbNbrs, bNbrs, belem);
        free(belem);
    }
    free(bNbrs);
    bNbrs = NULL;
}
