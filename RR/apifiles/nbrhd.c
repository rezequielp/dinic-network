#include "nbrhd.h"
#include "auxlibs/uthash/uthash.h
#include <stdlib.h>
#include <assert.h>

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
 *                      Funciones del modulo
 */

static void *findNbr(Nbrhd nbrs, u64 y, int* dir);
static void fedge_destroy(Fedge *fNbrs);
static void bedge_destroy(Bedge *bNbrs);
static Fedge *fedge_create(u64 y, u64 c);
static Bedge *bedge_create(u64 y, Fedge *fNbr);


/*Constructor de un nuevo Nbrhd*/
Nbrhd nbrhd_create(){
    Nbrhd nbrs=NULL;
    
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
    Fedge_destroy(nbrs->fNbrs);
    /*destruyo todos los vecinos backward*/
    Bedge_destroy(nbrs->bNbrs);    
    free(nbrs);
}


/* Genera el vinculo entre 'x' e 'y' (edge) convirtiendolos en vecinos,
 * la relacion es xy: 'y' vecino forward de 'x', 'x' vecino backward de 'y'
 * Precondicion: x, y, edge != NULL
 */
void nbrhd_addEdge(Nbrhd x, Nbrhd y, Lado edge){
    Fedge *fNbr = NULL;  /* vecino forward*/
    Bedge *bNbr = NULL;  /* vecino backward*/
    
    assert(x!=NULL && y!=NULL && edge!=NULL);
    /*Creo el vecino 'y' forward y lo agrego en x->fNbrs*/
    fNbr = fedge_create(lado_getY(edge), lado_getCap(edge));
    HASH_ADD(hhfNbrs, x->fNbrs, y, sizeof(x->fNbrs->y), fNbr);
    
    /*Creo el vecino 'x' backward y lo agrego en y->bNbrs*/
    bNbr = bedge_create(lado_getX(edge), fNbr);
    HASH_ADD(hhbNbrs, y->bNbrs, y, sizeof(y->bNbrs->y), bNbr);    
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
    static Fedge *fNbr;     /*ptr al ultimo vecino forward consultado*/
    static Bedge *bNbr;     /*ptr al ultimo vecino backward consultado*/
    int result = NONE;      /*valor de retorno*/

    assert(nbrs != NULL && y != NULL);
    assert(flag == FST || flag == NXT);
    assert(dir == FWD || dir == BWD || dir == UNK);
    
    if (flag == NXT) /*me posiciono en el sig de la tabla de uno ya consultado*/
        if(dir != BWD && fNbr != NULL)
            fNbr = fNbr->hhfNbrs.next;
        if(dir != FWD && bNbr != NULL || fNbr == NULL )
            bNbr = bNbr->hhbNbrs.next;
    else    /*el primer vecino que se encuentre en alguna direccion*/
        if (dir != BWD)
            fNbr = nbrs->fNbrs;
        if (dir != FWD || fNbr == NULL){
            bNbr = nbrs->bNbrs;
            dir = BWD;
        }
        
    if (fNbr != NULL || bNbr != NULL){    /*se encontro un siguiente*/
        if (dir != BWD){
            *y = fNbr->y;
            result = FWD;
        }else{
            *y = bNbr->y;
            result = BWD;
        }
    }
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
    
    assert(nbrs != NULL && vf > 0);    
    nbr = findNbr(nbrs, y, &dir);
    
    if (dir == FWD){        /* es FWD, aumento el flujo*/
        fNbr = (Fedge*)nbr;
        fNbr->flow += vf;
        assert(fNbr->flow <= fNbr->cap);
    }else{                  /* es BWD, disminuyo el flujo*/
        bNbr = (Bedge*)nbr;
        fNbr = bNbr->x;
        fNbr->flow -= vf;
        assert(fNbr->flow >= 0);
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
    void *nbr = NULL;   /*el vecino*/
    int dir = UNK;        /*direccion en la que se encuentra el vecino*/
    
    assert(nbrs != NULL);
    findNbr(nbrs, y, &dir);
    
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
Fedge *fedge_create(u64 y, u64 c){
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
Bedge *bedge_create(u64 y, Fedge *fNbr){
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
}
