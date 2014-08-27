#include "nbrhd.h"
#include "_uthash.h"
#include <stdlib.h>
#include <assert.h>

/** \file nbrhd.c
 *  Las estructuras Nbrhd, Fedge, Bedge y todas sus funciones se definen aquí.
 */

/** Estructura de una artista en sentido forward.
 * Relacion xy (y es nodo forward de x).
 */
typedef struct FedgeSt{
    u64 y;                  /**< Nodo forward de \a x. Es key de la hash.*/
    u64 cap;                /**< La capacidad restante de envio de flujo.*/
    u64 flow;               /**< El flujo que se esta enviando.*/
    UT_hash_handle hhfNbrs; /**< La tabla hash.*/
} Fedge;

/** Estructura de una arista en sentido backward.
 * Relacion yx (y es nodo backward de x).
 * Ej: x=3; y=2; Lado(yx)=23. 2 es backward de 3, 3 es forward de 2, 
 * por lo que existe una estructura Fedge para 3 relacionada al nodo 2. 
 * Esta estructura es a la que apunta \a *x.
 */
typedef struct BedgeSt{
    u64 y;              /**< Nodo backward de \a x. Es key de la hash.*/
    Fedge *x;           /**< Puntero a la entrada \a x por forward del nodo 'y'.*/
    UT_hash_handle hhbNbrs; /**< La tabla hash.*/
} Bedge;

/** Estructura Nbhd de la vecindad de un nodo \a x.*/
struct NeighbourhoodSt{
    Fedge *fNbrs;   /**< vecinos forward. Tabla hash de todos los vecinos forward de \a x*/
    Bedge *bNbrs;   /**< vecinos backward. Tabla hash de todos los vecinos backward de \a x*/
};

/*                      Funciones del modulo
 */
static void *findNbr(Nbrhd nbrs, u64 y, int* dir);
static Fedge *fedge_create(u64 y, u64 c);
static Bedge *bedge_create(u64 y, Fedge *fNbr);
static void fedge_destroy(Fedge *fNbrs);
static void bedge_destroy(Bedge *bNbrs);


/** Constructor de un nuevo Nbrhd.
 * \return un \a Nbrhd vacío.
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
 */
void nbrhd_destroy(Nbrhd nbrs){
    assert(nbrs != NULL);
    /*destruyo todos los vecinos forward*/
    if (nbrs->fNbrs != NULL)
        fedge_destroy(nbrs->fNbrs);
    /*destruyo todos los vecinos backward*/
    if (nbrs->bNbrs != NULL)
        bedge_destroy(nbrs->bNbrs);    
    free(nbrs);
    nbrs = NULL;
}


/** Genera el vinculo entre \a x e \a y (edge) convirtiendolos en vecinos.
 * la relacion es \a xy: \a y vecino forward de \a x, \a x vecino backward de \a y
 * \pre x, y, edge != NULL
 * \param x Vecindario del nodo x.
 * \param y Vecindario del nodo y.
 * \param edge Lado \a xy con su capacidad y flujo.
 */
void nbrhd_addEdge(Nbrhd x, Nbrhd y, Lado edge){
    Fedge *fNbr = NULL;     /* vecino forward*/
    Bedge *bNbr = NULL;     /* vecino backward*/
    u64 yName;              /* nombre del nodo y*/
    
    assert(x!=NULL && y!=NULL && edge!=LadoNulo);
    
    /*Se busca si ya existe una entrada para el lado 'xy'*/
    yName = lado_getY(edge);
    HASH_FIND(hhfNbrs, x->fNbrs, &(yName), sizeof(x->fNbrs->y), fNbr);
    /*Si no existe, creo las estructuras correspondientes*/
    if(fNbr == NULL){
        /*Creo el vecino 'y' forward y lo agrego en x->fNbrs*/
        fNbr = fedge_create(yName, lado_getCap(edge));
        HASH_ADD(hhfNbrs, x->fNbrs, y, sizeof(x->fNbrs->y), fNbr);
        
        /*Creo el vecino 'x' backward y lo agrego en y->bNbrs*/
        bNbr = bedge_create(lado_getX(edge), fNbr);
        HASH_ADD(hhbNbrs, y->bNbrs, y, sizeof(y->bNbrs->y), bNbr);
    /*Si existe, es un caso de lados paralelos*/
    }else{
        fNbr->cap += lado_getCap(edge);
    }
        
}

/** \note Tener en cuenta la documentacion sobre las opciones de los parametros
 * y retorno. Verlo como una iteracion sobre una tabla en la que empiezo por 
 * el primer(\a FST) elemento, o bien por el siguiente del ultimo consultado(\a NXT).
 * 
 * Busca el vecino siguiente en la direccion \a dir y si existe almacena el 
 * nombre en \a y. Si \a dir=UNK entonces primero intenta por \a FWD, y si
 * no hay (o no existen) entonces intenta por \a BWD.
 * \param nbrs  El vecindario de \a x. 
 * \param flag  Buscar desde el inicio de la tabla o desde el ultimo encontrado.
 * \param dir   La dirección en la que se pretende buscar.
 * \param y    Variable en la que se almacena el nombre del vecino encontrado.
 * \pre nbrs!=NULL, flag=FST|NXT, dir=FWD|BWD|UNK, y!=NULL
 * \return \a dir, en la que se encuentra el vecino.\n
 *         \a NONE, si ya no hay mas vecinos que devolver.
 */
int nbrhd_getNext(Nbrhd nbrs, int flag, int dir, u64 *y){
    static Fedge *fNbr = NULL;  /*Puntero al ultimo vecino forward consultado*/
    static Bedge *bNbr = NULL;  /*Puntero al ultimo vecino backward consultado*/
    int result = NONE;          /*valor de retorno*/
    
    assert(nbrs != NULL && y != NULL);
    assert(flag == FST || flag == NXT);
    assert(dir == FWD || dir == BWD || dir == UNK);

    /*Se pide un vecino desde el inicio de la tabla*/
    if(flag == FST){
        /*Si no se pide por backward y existen vecinos por forward*/
        if(dir != BWD && nbrs->fNbrs != NULL){
                /*Se asigna el primero de la tabla por forward*/
                fNbr = nbrs->fNbrs;
                *y = fNbr->y;
                result = FWD;
        /*Si no se pide por forward y existen vecinos por backward*/
        }else if(dir != FWD && nbrs->bNbrs != NULL){
                /*Se asigna el primero de la tabla por backward*/
                bNbr = nbrs->bNbrs;
                *y = bNbr->y;
                result = BWD;
        /*Observar que si dir=UNK por el orden se prueba primero por forward*/
        }
    /*Se pide el siguiente vecino de uno anterior*/
    }else{
        /*Si se pide por forward y anteriormente se retorno un vecino forward*/
        if(dir == FWD && fNbr != NULL){
            /*Se asigna el sig de la tabla, NULL si el anterior fue el ultimo*/
            fNbr = fNbr->hhfNbrs.next;
            if(fNbr != NULL){
                *y = fNbr->y;
                result = FWD;
            }
        /*Si se pide por backward y anteriormente se retorno un vecino backward*/
        }else if(dir == BWD && bNbr != NULL){
            /*Se asigna el sig de la tabla, NULL si el anterior fue el ultimo*/
            bNbr = bNbr->hhbNbrs.next;
            if(bNbr != NULL){
                *y = bNbr->y;
                result = BWD;
            }
        }
        /*Observar que pedir NXT en direccion UNK no es un caso a tener en cuenta*/
    }
    return result;
}

/** Se aumenta el flujo para con el vecino \a y por \a vf cantidad. 
 * Si \a y es un vecino BWD, el valor del flujo se disminuye por \a vf cantidad.
 * \param nbrs  El vecindario de \a x. 
 * \param y     El nombre del vecino.
 * \param vf    El valor de flujo.
 * \pre \a y es vecino de \a x. \a vf > 0
 * \return Valor del nuevo flujo que se esta enviando entre \a x e \a y. */
u64 nbrhd_increaseFlow(Nbrhd nbrs, u64 y, u64 vf){
    Fedge *fNbr = NULL; /*para manipular el lado si 'dir'=FWD*/
    Bedge *bNbr = NULL; /*para manipular el lado si 'dir'=BWD*/
    void *nbr = NULL;   /*el vecino*/
    int dir = UNK;      /*direccion en la que se encuentra el vecino*/

    assert(nbrs != NULL);
    assert(vf > 0);    
    nbr = findNbr(nbrs, y, &dir); /*se busca*/
    
    if (dir == FWD){        /* es FWD, aumento el flujo*/
        fNbr = (Fedge*)nbr;
        fNbr->flow += vf;
        assert(fNbr->flow <= fNbr->cap);
    }else{                  /* es BWD, disminuyo el flujo*/
        bNbr = (Bedge*)nbr;
        fNbr = bNbr->x;
        assert(fNbr->flow >= vf);
        fNbr->flow -= vf;
        
    }

    return fNbr->flow;
}


/** Devuelve la capacidad con el vecino \a y del lado \a xy.
 * \param nbrs  El vecindario de \a x. 
 * \param y     El nombre del vecino.
 * \pre \a y es vecino de \a x. 
 * \return  La capacidad del lado \a xy.
 */
u64 nbrhd_getCap(Nbrhd nbrs, u64 y){
    void *nbr = NULL;   /*el vecino*/
    int dir = UNK;      /*direccion en la que se encuentra el vecino*/
    
    assert(nbrs != NULL);
    nbr = findNbr(nbrs, y, &dir);   /*se busca*/
    assert(nbr != NULL && (dir != UNK && dir != NONE));  /*debe conocerse*/
    if (dir == BWD)
        /*es backward, se maneja distinto*/
        nbr = ((Bedge*)nbr)->x;
    
    return ((Fedge*)nbr)->cap;
}


/** Devuelve el valor del flujo con el vecino \a y del lado \a xy.
 * \param nbrs  El vecindario de \a x. 
 * \param y     El nombre del vecino.
 * \pre \a y es vecino de \a x. 
 * \return  El valor de flujo del lado \a xy.
 */
u64 nbrhd_getFlow(Nbrhd nbrs, u64 y){
    void *nbr = NULL;   /*el vecino*/
    int dir = UNK;      /*direccion en la que se encuentra el vecino*/
    
    assert(nbrs != NULL);
    nbr = findNbr(nbrs, y, &dir); /*se busca*/
    assert(nbr != NULL && (dir != UNK && dir != NONE));  /*debe conocerse*/
    if (dir == BWD)
        /*es backward, se maneja distinto*/
        nbr = ((Bedge*)nbr)->x;
    
    return ((Fedge*)nbr)->flow;
}

/** Devuelve la dirección en la que se encuentra el vecino \a y respecto a \a x.
 * \param nbrs  El vecindario de \a x.
 * \param y     El nombre del vecino.
 * \pre \a y es vecino de \a x. 
 * \return  La dirección entre el nodo \a x e \a y.
 */
int nbrhd_getDir(Nbrhd nbrs, u64 y){
    int dir = UNK;      /*direccion en la que se encuentra el vecino*/  
    
    assert(nbrs != NULL);
    findNbr(nbrs, y, &dir); /*se busca*/
    assert(dir != UNK && dir != NONE); /*debe conocerse*/
    return dir;
}


/*
 *          Funciones locales
 */

/** Busca un vecino con nombre \a y, en alguna dirección dada. 
 * Si \a dir!=UNK, busca unicamente en esa direccion; caso contrario, 
 * busca primero por forward y luego por backward.
 * \param nbrs  El vecindario de \a x.
 * \param y     El nombre del vecino.
 * \pre \a y es vecino de \a x.
 * \return  La representación del lado dependiendo de la dirección 
 *          en la que se encuentra \a y. \n
 *          Si es forward, se retorna un puntero a un Fedge. \n
 *          Si es backward, se retorna un puntero a un Bedge.
 */
static void *findNbr(Nbrhd nbrs, u64 y, int* dir){
    Fedge *fNbr = NULL;     /*para manipular el lado si 'dir'=FWD*/
    Bedge *bNbr = NULL;     /*para manipular el lado si 'dir'=BWD*/
    void *result = NULL;    /*el lado de retorno, fNbr o bNbr*/
    
    assert(nbrs != NULL);   
    if (*dir != BWD)         /*busqueda por vecinos forward (dir == UNK|FWD)*/
        HASH_FIND(hhfNbrs, nbrs->fNbrs, &(y), sizeof(y), fNbr);
        /* caso dir == FWD, no puede ser fnbr == NULL */
        assert(*dir != FWD || fNbr != NULL);
    
    if (fNbr != NULL){      /*se encontro en forward*/
        result = fNbr;
        *dir = FWD;
    }else{                  /*busqueda por vecinos backward*/
        HASH_FIND(hhbNbrs, nbrs->bNbrs, &(y), sizeof(y), bNbr);
        assert(bNbr != NULL);   /*'y' no es vecino, error */
        result = bNbr;
        *dir = BWD;
    }
        
    return result;
}

/** Construye un nuevo Fedge para el nodo \a x. 
 * El valor del flujo se inicia en 0.
 * \param y Nombre del vecino que se relaciona.
 * \param c Capacidad con el vecino.
 * \return Puntero al nuevo Fedge inicializado. 
 *          El llamador se encarga de liberarlo.
*/
static Fedge *fedge_create(u64 y, u64 c){
    Fedge *fNbr = NULL;     /* vecino forward*/
    
    fNbr = (Fedge *) malloc(sizeof(struct FedgeSt));
    assert(fNbr != NULL);
    fNbr->y = y;        
    fNbr->cap = c;
    fNbr->flow = 0;
    
    return fNbr;
}


/** Construye un nuevo Bedge para el nodo \a x. 
 * Se vincula a los datos forward respecto a él.
 * \param y Nombre del vecino que se relaciona.
 * \param fNbr Puntero al Fedge en donde \a x es forward de \a y.
 * \return Puntero al nuevo Bedge inicializado. 
 *          El llamador se encarga de liberarlo.
*/
static Bedge *bedge_create(u64 y, Fedge *fNbr){
    Bedge *bNbr = NULL;     /* vecino backward*/
    
    bNbr = (Bedge*) malloc(sizeof(struct BedgeSt));
    assert(bNbr != NULL);
    bNbr->y = y;          
    bNbr->x = fNbr;
    
    return bNbr;
}


/** Destructor de vecinos forwards.
 * \param fNbrs Puntero a la hash de vecinos forward
 * \pre \a fNbrs!=NULL
 */
static void fedge_destroy(Fedge *fNbrs){
    Fedge *felem = NULL;    /*el i-esimo elemento de fNbrs*/
    Fedge *feTmp = NULL;    /*el i-esimo+1 elemento de fNbrs*/
    
    assert(fNbrs != NULL);
    HASH_ITER(hhfNbrs, fNbrs, felem, feTmp){
        HASH_DELETE(hhfNbrs, fNbrs, felem);
        free(felem);
    }    
    free(fNbrs);
    fNbrs = NULL;
}


/** Destructor de vecinos backwards 
 * \param bNbrs Puntero a la hash de vecinos forward
 * \pre \a bNbrs!=NULL
 */
static void bedge_destroy(Bedge *bNbrs){
    Bedge *belem = NULL;    /*el i-esimo elemento de bNbrs*/
    Bedge *beTmp = NULL;    /*el i-esimo+1 elemento de bNbrs*/
    
    assert(bNbrs != NULL);
    HASH_ITER(hhbNbrs, bNbrs, belem, beTmp){
        HASH_DELETE(hhbNbrs, bNbrs, belem);
        free(belem);
    }
    free(bNbrs);
    bNbrs = NULL;
}
