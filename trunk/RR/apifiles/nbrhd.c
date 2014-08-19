#include "nbrhd.h"
#include "auxlibs/uthash/uthash.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h> /*TODO borrar*/

/* Estructura de un nodo por forward*/
typedef struct FedgeSt{
    u64 y;          /* El nodo forward de 'x', se usa como key en la hash*/
    u64 cap;        /* La capacidad restante de envio de flujo*/
    u64 flow;       /* El flujo por forward que se esta enviando*/
    UT_hash_handle hhfNbrs;
} Fedge;

/* Estructura de un nodo por backward*/
typedef struct BedgeSt{
    u64 y;          /* key */
    Fedge *x;      /* Puntero a la entrada 'x' de la fhash del nodo 'y'  */
    UT_hash_handle hhbNbrs;
} Bedge;

/* Estructura de la vecindad de un nodo*/
struct NeighbourhoodSt{
    Fedge *fNbrs;   /* vecinos forward*/
    Bedge *bNbrs;   /* vecinos backward*/
};
/*
 *  CONSTANTES     
 */
/*#define NON_STARTED  0
#define STARTED      1
#define FINISHED    -1*/
/* 
 *                      Funciones del modulo
 */

static void *findNbr(Nbrhd nbrs, u64 y, int* dir);
static void fedge_destroy(Fedge *fNbrs);
static void bedge_destroy(Bedge *bNbrs);
static Fedge *fedge_create(u64 y, u64 c);
static Bedge *bedge_create(u64 y, Fedge *fNbr);


/*Constructor de un nuevo Nbrhd*/
Nbrhd nbrhd_create(void){
    Nbrhd nbrs = NULL;
    
    nbrs = (Nbrhd) malloc(sizeof(struct NeighbourhoodSt));
    assert(nbrs != NULL);
    
    nbrs->fNbrs = NULL;
    nbrs->bNbrs = NULL;
    
    return nbrs;
}


/*Destructor de un Nbrhd*/
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


/* Genera el vinculo entre 'x' e 'y' (edge) convirtiendolos en vecinos,
 * la relacion es xy: 'y' vecino forward de 'x', 'x' vecino backward de 'y'
 * Precondicion: x, y, edge != NULL
 */
void nbrhd_addEdge(Nbrhd x, Nbrhd y, Lado edge){
    Fedge *fNbr = NULL;  /* vecino forward*/
    Bedge *bNbr = NULL;  /* vecino backward*/
    u64 yName;          /* nombre del nodo y*/
    
    assert(x!=NULL && y!=NULL && edge!=LadoNulo);
    
    yName = lado_getY(edge);
    HASH_FIND(hhfNbrs, x->fNbrs, &(yName), sizeof(x->fNbrs->y), fNbr);
    if(fNbr == NULL){
        /*Creo el vecino 'y' forward y lo agrego en x->fNbrs*/
        fNbr = fedge_create(yName, lado_getCap(edge));
        HASH_ADD(hhfNbrs, x->fNbrs, y, sizeof(x->fNbrs->y), fNbr);
        
        /*Creo el vecino 'x' backward y lo agrego en y->bNbrs*/
        bNbr = bedge_create(lado_getX(edge), fNbr);
        HASH_ADD(hhbNbrs, y->bNbrs, y, sizeof(y->bNbrs->y), bNbr);    
    }else{
        fNbr->cap += lado_getCap(edge);     /*caso lado paralelo*/
    }
        
    }

/* NOTE Tener en cuenta la documentacion sobre las opciones de los parametros
 * y retorno. Verlo como una iteracion sobre una lista en la que empiezo por 
 * el primer(FST) elemento, o bien por el siguiente del ultimo consultado(NXT)
 * 
 * Busca el vecino siguiente en la direccion 'dir' y si existe 
 * almacena el nombre en 'y'. Si dir=UNK entonces primero intenta por FWD, y si
 * no hay (o no existen) entonces intenta por BWD.
 * Precondicion: nbrs!=NULL, flag=FST|NXT, dir=FWD|BWD|UNK, y!=NULL
 * Retorno(r):  r = 'dir', en la que se encuentra el vecino
 *              r = NONE, ya no hay mas vecinos que devolver
 */
int nbrhd_getNext(Nbrhd nbrs, int flag, int dir, u64 *y){
    static Fedge *fNbr = NULL;     /*ptr al ultimo vecino forward consultado*/
    static Bedge *bNbr = NULL;     /*ptr al ultimo vecino backward consultado*/
    /*static fNbrStatus = NON_STARTDED;
    static bNbrStatus = NON_STARTDED;*/
    int result = NONE;      /*valor de retorno*/
    
    assert(nbrs != NULL && y != NULL);
    assert(flag == FST || flag == NXT);
    assert(dir == FWD || dir == BWD || dir == UNK);

    if(flag == FST){
        if(dir != BWD && nbrs->fNbrs != NULL){
                fNbr = nbrs->fNbrs;
                *y = fNbr->y;
                result = FWD;
        }else if(dir != FWD && nbrs->bNbrs != NULL){
                bNbr = nbrs->bNbrs;
                *y = bNbr->y;
                result = BWD;
        }
    }else{
        if(dir == FWD && fNbr != NULL){
            fNbr = fNbr->hhfNbrs.next;
            if(fNbr != NULL){
                *y = fNbr->y;
                result = FWD;
            }
        }else if(dir == BWD && bNbr != NULL){
            bNbr = bNbr->hhbNbrs.next;
            if(bNbr != NULL){
                *y = bNbr->y;
                result = BWD;
            }
        }
    }



/*    if(flag == FST){
        if (dir == UNK){
            if(nbrs->fNbrs != NULL)
                dir = FWD;
            else if(nbrs->bNbrs != NULL)
                dir = BWD;
        }
        if(dir == FWD && nbrs->fNbrs != NULL){
                fNbr = nbrs->fNbrs;
                *y = fNbr->y;
                result = FWD;
                fNbrStatus = STARTED;
        }else if(dir == BWD && nbrs->bNbrs != NULL){
                bNbr = nbrs->bNbrs;
                *y = bNbr->y;
                result = BWD;
                bNbrStatus = STARTED;
        }
    }else{
        if(dir == FWD){
            if(fNbrStatus == STARTED){
                fNbr = fNbr->hhfNbrs.next;
                if(fNbr != NULL){
                    *y = fNbr->y;
                    result = FWD;
                }else
                    fNbrStatus = FINISHED;
            }
        }else if(dir == BWD){
            if(bNbrStatus == STARTED){
                bNbr = bNbr->hhbNbrs.next;
                if(bNbr != NULL){
                    *y = bNbr->y;
                    result = BWD;
                }else
                    bNbrStatus = FINISHED;
            }
        }else{ 
            if(fNbrStatus == STARTED){
                fNbr = fNbr->hhfNbrs.next;
                if(fNbr != NULL){
                    *y = fNbr->y;
                    result = FWD;
                }else
                    fNbrStatus = FINISHED;
            }
           if(fNbrStatus = FINISHED){
                if(bNbrStatus == STARTED){
                    bNbr = bNbr->hhbNbrs.next;
                    if(bNbr != NULL){
                        *y = bNbr->y;
                        result = BWD;
                    }else
                        bNbrStatus = FINISHED;
                }else if (bNbrStatus == NOT_STARTED && nbrs->bNbrs != NULL){
                    bNbr = nbrs->bNbrs;
                    *y = bNbr->y;
                    result = BWD;
                    bNbrStatus = STARTED
                }
            }
        }
    }*/
    return result;
}

/* Se aumenta el flujo para con el vecino 'y' por 'vf' cantidad. 
 * Si 'y' es un vecino BWD, el valor del flujo se disminuye por 'vf' cantidad
 * Precondicion: 'y' es vecino. vf > 0
 * Retorno: valor del nuevo flujo que se esta enviando */
u64 nbrhd_increaseFlow(Nbrhd nbrs, u64 y, u64 vf){
    Fedge *fNbr = NULL;
    Bedge *bNbr = NULL;
    void *nbr = NULL;
    int dir = UNK;      /*direccion en la que se encuentra el vecino*/

    assert(nbrs != NULL);
    assert(vf > 0);    
    nbr = findNbr(nbrs, y, &dir);
    
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


/*devuelve la capacidad con el vecino y
 Precondicion: 'y' es vecino */
u64 nbrhd_getCap(Nbrhd nbrs, u64 y){
    void *nbr = NULL;   /*el vecino*/
    int dir = UNK;        /*direccion en la que se encuentra el vecino*/
    
    assert(nbrs != NULL);
    nbr = findNbr(nbrs, y, &dir);
    
    if (dir == BWD)
        nbr = ((Bedge*)nbr)->x;
    
    return ((Fedge*)nbr)->cap;
}


/*devuelve el valor del flujo con el vecino 'y'
 Precondicion: 'y' es vecino */
u64 nbrhd_getFlow(Nbrhd nbrs, u64 y){
    void *nbr = NULL;   /*el vecino*/
    int dir = UNK;        /*direccion en la que se encuentra el vecino*/
    
    assert(nbrs != NULL);
    nbr = findNbr(nbrs, y, &dir);
    
    if (dir == BWD)
        nbr = ((Bedge*)nbr)->x;
    
    return ((Fedge*)nbr)->flow;
}

/*devuelve la direccion (FWD o BWD) en la que se encuentra el vecino 'y'
 Precondicion: 'y' es vecino */
int nbrhd_getDir(Nbrhd nbrs, u64 y){
    int dir = UNK;      /*direccion en la que se encuentra el vecino*/
    
    assert(nbrs != NULL);
    findNbr(nbrs, y, &dir);
    assert(dir != UNK);
    return dir;
}


/*
 *          Funciones locales
 */

/* BUSQUEDA INDIVUDAL, por si no funciona la general
Fedge nbrhd_findFnbr(Fedge fNbrs, u64 y){
    Fedge fnbr = NULL;
    
    assert(fNbrs != NULL && y != NULL);
    
    HASH_FIND(fNbrs->hhfNbrs, fNbrs, &(y), sizeof(y), fnbr);
    
    return fnbr;
}

Bedge nbrhd_findBnbr(Bedge bNbrs, u64 y){
    Bedge bnbr = NULL;
    
    assert(bNbrs != NULL && y != NULL);
    
    HASH_FIND(bNbrs->hhbNbrs, bNbrs, &(y), sizeof(y), bnbr);
    
    return bnbr;
}
*/
/* ATTENTION Arriba esta hecho por separado, por si esto no funciona
 * 
 * Devuelve el vecino 'y' con la direccion en la que se encontro en 'dir'. 
 * Si 'dir' != UNK, busca unicamente en esa direccion; 
 * caso contrario, busca primero por forward y luego por backward.
 * Precondicion: 'y' es vecino */
static void *findNbr(Nbrhd nbrs, u64 y, int* dir){
    Fedge *fNbr = NULL;
    Bedge *bNbr = NULL;
    void *result = NULL;
    
    assert(nbrs != NULL);
    
    if (*dir != BWD)         /*busqueda por vecinos forward (dir == UNK|FWD)*/
        HASH_FIND(hhfNbrs, nbrs->fNbrs, &(y), sizeof(y), fNbr);
        /* caso dir == FWD, no puede ser fnbr == NULL */
        assert(*dir != FWD || fNbr != NULL);
    
    if (fNbr != NULL){      /* se encontro en forward*/
        result = fNbr;
        *dir = FWD;
    }else{                  /*busqueda por vecinos backward*/
        HASH_FIND(hhbNbrs, nbrs->bNbrs, &(y), sizeof(y), bNbr);
        assert(bNbr != NULL);   /* 'y' no es vecino, error */
        result = bNbr;
        *dir = BWD;
    }
        
    return result;
}

/* Construye un Fedge a partir del nombre del vecino (y) y la capacidad
que se tiene con el (c). El valor del flujo se inicia en 0.*/
static Fedge *fedge_create(u64 y, u64 c){
    Fedge *fNbr = NULL;     /* vecino forward*/
    
    fNbr = (Fedge *) malloc(sizeof(struct FedgeSt));
    assert(fNbr != NULL);
    fNbr->y = y;        
    fNbr->cap = c;
    fNbr->flow = 0;
    
    return fNbr;
}


/* Construye un Bedge a partir del nombre del vecino (y) y el vinculo
a los datos por forward respecto a el*/
static Bedge *bedge_create(u64 y, Fedge *fNbr){
    Bedge *bNbr = NULL;     /* vecino backward*/
    
    bNbr = (Bedge*) malloc(sizeof(struct BedgeSt));
    assert(bNbr != NULL);
    bNbr->y = y;          
    bNbr->x = fNbr;
    
    return bNbr;
}


/* Destructor de vecinos forwards */
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


/* Destructor de vecinos backwards */
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
