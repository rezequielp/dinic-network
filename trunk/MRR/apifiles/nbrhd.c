#include "auxlibs/bstring/bstrlib.h"
#include "nbrhd.h"
#include "auxlibs/bstring/uthash.h"
#include <stdlib.h>
#include <assert.h>

/* Estructura de un nodo por forward*/
typedef struct FedgeSt{
    u64 y;          /* El nodo forward de 'x', se usa como key en la hash*/
    u64 cap;        /* La capacidad restante de envio de flujo*/
    u64 flow;       /* El flujo por forward que se esta enviando*/
    UT_hash_handler hhfNbrs;
} Fedge;

/* Estructura de un nodo por backward*/
typedef struct BedgeSt{
    u64 y;          /* key */
    Fedge * x;      /* Puntero a la entrada 'x' de la fhash del nodo 'y'  */
    UT_hash_handler hhbNbrs;
} Bedge;

/* Estructura de la vecindad de un nodo*/
struct NeighbourhoodSt{
    Fedge *fNbrs;   /* vecinos forward*/
    Bedge *bNbrs;   /* vecinos backward*/
};


/* 
 *                      Funciones del modulo
 */

static void *findNbr(Nbrhd nbrs, u64 y, int dir);
static void fedge_destroy(Fedge fNbrs);
static void bedge_destroy(Bedge bNbrs);
static Fedge fedge_create(u64 y, u64 c);
static Bedge bedge_create(u64 y, Fedge fNbr);


/*Constructor de un nuevo Nbrhd*/
Nbrhd nbrhd_create(){
    Nbrhd nbrs;
    
    nbrs = (Nbrhd) malloc(sizeof (struct NeighbourhoodSt));
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


/* Destructor de un Fedge*/
void fedge_destroy(Fedge fNbrs){
    Fedge felem = NULL;    /*el i-esimo elemento de fNbrs*/
    Fedge feTmp = NULL;    /*el i-esimo+1 elemento de fNbrs*/
    
    assert(fNbrs!=NULL);
    HASH_ITER(fNbrs->hhfNbrs, fNbrs, felem, feTmp){
        HASH_DEL(fNbrs, felem);
        free(felem);
    }
    
    free(fNbrs);
}


/* Destructor de un Bedge*/
void bedge_destroy(Bedge bNbrs){
    Fedge belem = NULL;    /*el i-esimo elemento de bNbrs*/
    Fedge beTmp = NULL;    /*el i-esimo+1 elemento de bNbrs*/
    
    assert(bNbrs!=NULL);
    HASH_ITER(bNbrs->hhbNbrs, bNbrs, belem, beTmp){
        HASH_DEL(bNbrs, belem);
        free(belem);
    }
    
    free(bNbrs);
}


/* Genera el vinculo entre 'x' e 'y' (edge) convirtiendolos en vecinos,
 * la relacion es xy: 'y' vecino forward de 'x', 'x' vecino backward de 'y'
 * Precondicion: x, y, edge != NULL
 */
void nbrhd_addEdge(Nbrhd x, Nbrhd y, Lado edge){

    Bedge *bNbr = NULL;     /* vecino backward*/
    void *hTable = NULL;    /* la tabla hash en la que agrego el vecino*/
    
    /*Creo un vecino forward*/
    fedge_create(lado_getY(edge), lado_getCap(edge));
    /*lo agrego en la tabla de vecinos forward de 'x'*/ 
    hTable = x->fNbrs;
    HASH_ADD(hTable->hhfNbrs, hTable, hTable->y, sizeof(hTable->y), fNbr);
    
    /*Creo un vecino backward*/
    bedge_create(lado_getX(edge), fNbr);
    /*lo agrego en la tabla de vecinos backward de 'y'*/
    hTable = y->bNbrs;
    HASH_ADD(hTable->hhbNbrs, hTable, hTable->y, sizeof(hTable->y), bNbr);    
}


/* Construye un Fedge a partir del nombre del vecino (y) y la capacidad
que se tiene con el (c)*/
Fedge fedge_create(u64 y, u64 c){
    Fedge *fNbr = NULL;     /* vecino forward*/
    
    assert(y!=NULL && c!=NULL);

    fNbr = (Fedge*) malloc(sizeof(Fedge));
    assert(fNbr != NULL);
    fNbr->y = y;          
    fNbr->cap = c;        
    fNbr->flow = 0;
    
    return fNbr;
}


/* Construye un Bedge a partir del nombre del vecino (y) y el vinculo
a los datos por forward respecto a el*/
Bedge bedge_create(u64 y, Fedge fNbr){
    Bedge *bNbr = NULL;     /* vecino backward*/
    
    assert(y!=NULL && fNbr!=NULL);
    
    bNbr = (Bedge*) malloc(sizeof(Bedge))
    assert(bNbr != NULL);
    bNbr->y = y;          
    bNbr->x = fNbr;
    
    return bNbr;
}    
    

/* Busca el vecino que sigue despues de 'y' en la direccion 'dir' (FWD o BWD)
 * Retorno: y = NULL, u64 nombre del primer vecino en esa direccion
 *          y != NULL, u64 nombre del sig vecino desp de 'y' en esa direccion
 *          NULL, si no existen mas vecinos en esa direccion */
u64 nbrhd_getNext(Nbrhd nbrs, u64 y, int dir){
    void *nbrY = NULL;
    u64 nxtNbr = NULL;
    
    assert(nbrs != NULL);
    assert(dir == FWD || dir == BWD);
    
    if (dir == FWD)
        if (y != NULL){                     /*el sig vecino despues de y*/
            nbrY = findNbr(nbrs, y, dir);
            assert(nbrY!=NULL);
            nxtNbr = nbrY->hhfNbrs.next->y;
        }else                               /*primer vecino forward*/
            if (nbrs->fNbrs != NULL)
                nxtNbr = nbrs->fNbrs->y;
    else
        if (y != NULL){                     /*el sig vecino despues de y*/
            nbrY = findNbr(nbrs, y, dir);
            assert(nbrY!=NULL);
            nxtNbr = nbrY->hhbNbrs.next->y;
        }else                               /*primer vecino backward*/
            if (nbrs->bNbrs != NULL)
                nxtNbr = nbrs->bNbrs->y;

    return nxtNbr;
}


/* Se aumenta el flujo para con el vecino 'y' por 'vf' cantidad. 
 * Si 'y' es un vecino BWD, el valor del flujo se disminuye por 'vf' cantidad
 * Precondicion: y != NULL y es vecino del nodo padre. vf > 0
 * Retorno: valor del nuevo flujo que se esta enviando */
u64 nbrhd_increaseFlow(Nbrhd nbrs, u64 y, u64 vf){
    void *nbr = NULL;   /*el vecino*/
    int dir = 0;        /*direccion en la que se encuentra el vecino*/
    u64 gf = 0;         /*flujo actual enviandose. Retorno*/
    
    assert(nbrs != NULL && y != NULL && vf > 0);
    
    nbr = findNbr(nbrs, y, dir);
    assert(nbr != NULL);
    
    if (dir == FWD){        /* es FWD, aumento el flujo*/
        nbr->flow += vf;
        assert(nbr->flow <= nbr->cap);
    }else{                  /* es BWD, disminuyo el flujo*/
        nbr = nbr->x;
        nbr->flow -= vf;
        assert(nbr->flow >= 0);
    }

    return nbr->flow;
}


/*devuelve la capacidad con el vecino y
 Precondicion: y != NULL y es vecino del nodo padre */
u64 nbrhd_getCap(Nbrhd nbrs, u64 y){
    void *nbr = NULL;   /*el vecino*/
    int dir = 0;        /*direccion en la que se encuentra el vecino*/
    
    assert(nbrs != NULL && y != NULL);

    nbr = findNbr(nbrs, y, dir);
    assert(nbr != NULL);
    
    if (dir == BWD)
        nbr = nbr->x;
    
    return nbr->cap;
}


/*devuelve el valor del flujo con el vecino 'y'
 Precondicion: y != NULL y es vecino del nodo padre */
u64 nbrhd_getFlow(Nbrhd nbrs, u64 y){
    void *nbr = NULL;   /*el vecino*/
    int dir = 0;        /*direccion en la que se encuentra el vecino*/
    
    assert(nbrs != NULL && y != NULL);

    nbr = findNbr(nbrs, y, dir);
    assert(nbr != NULL);
    
    if (dir == BWD)
        nbr = nbr->x;
    
    return nbr->flow;
}

/*devuelve la direccion (FWD o BWD) en la que se encuentra el vecino 'y'
 Precondicion: y != NULL y es vecino del nodo padre */
int nbrhd_getDir(Nbrhd nbrs, u64 y); 
    void *nbr = NULL;   /*el vecino*/
    int dir = 0;        /*direccion en la que se encuentra el vecino*/
    
    assert(nbrs != NULL && y != NULL);

    nbr = findNbr(nbrs, y, dir);
    assert(nbr != NULL);
    
    return dir;
}







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
/* ATTENTION Arriba esta hecho separadamente, por si esto no funciona
 * 
 * Devuelve el vecino 'y' con la direccion en la que se encontro en 'dir'. 
 * Si 'dir' != 0, busca unicamente en esa direccion; 
 * caso contrario, busca primero por forward y luego por backward.
 * Precondicion: y != NULL y es vecino del nodo padre */
void *findNbr(Nbrhd nbrs, u64 y, int dir){
    Fedge fnbr = NULL;
    Bedge bnbr = NULL;
    void *result = NULL;
    
    assert(Nbrs != NULL && y != NULL);
    
    if (dir != BWD)         /*busqueda por vecinos forward*/
        HASH_FIND(nbrs->fNbrs->hhfNbrs, nbrs->fNbrs, &(y), sizeof(y), fnbr);
        /* caso dir == FWD, no puede ser fnbr == NULL */
        assert(dir != FWD || fnbr != NULL);
    
    if (fnbr != NULL){      /* se encontro en forward*/
        result = fnbr;
        dir = FWD;
    }else{                  /*busqueda por vecinos backward*/
        HASH_FIND(nbrs->bNbrs->hhfNbrs, nbrs->bNbrs, &(y), sizeof(y), bnbr);
        assert(bnbr != NULL);   /* 'y' no es vecino, error */
        result = bnbr;
        dir = BWD;
    }
        
    return result;
}

