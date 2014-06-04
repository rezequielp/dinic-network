#include "auxlibs/bstring/bstrlib.h"
#include "nbrhd.h"
#include "auxlibs/uthash//uthash.h"
#include <stdlib.h>
#include <assert.h>

/* Estructura de un nodo por forward*/
typedef struct FedgeSt{
    u64 y;          /* El nodo forward de 'x', se usa como key en la hash*/
    u64 cap;        /* La capacidad restante de envio de flujo*/
    u64 flow;       /* El flujo por forward que se esta enviando*/
    UT_hash_handler hhfNbrs;
} *Fedge;

/* Estructura de un nodo por backward*/
typedef struct BedgeSt{
    u64 y;          /* key */
    Fedge * x;      /* Puntero a la entrada 'x' de la fhash del nodo 'y'  */
    UT_hash_handler hhbNbrs;
} *Bedge;

/* Estructura de la vecindad de un nodo*/
struct NeighbourhoodSt{
    Fedge *fNbrs;   /* vecinos forward*/
    Bedge *bNbrs;   /* vecinos backward*/
};


#define UNK 0    /*direccion desconocida*/

/* 
 *                      Funciones del modulo
 */

static (void *) findNbr(Nbrhd nbrs, u64 y, int dir);
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
que se tiene con el (c). El valor del flujo se inicia en 0.*/
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
    

/* NOTE Tener en cuenta la documentacion sobre las opciones de los parametros
 * y retorno. Verlo como una iteracion sobre una lista en la que empiezo por 
 * el primer(FST) elemento, o bien por el siguiente del ultimo consultado(NXT)
 * 
 * Busca el vecino siguiente en la direccion 'dir' (FWD o BWD) y si existe 
 * almacena el nombre en 'y'
 * Precondicion: nbrs!=NULL, flag=FST|NXT, dir=FWD|BWD, y!=NULL
 * Retorno(r):  r = 'dir', si se encontro un vecino
 *              r = NONE, ya no hay mas vecinos en esa direccion
 */
int nbrhd_getNext(Nbrhd nbrs, int flag, int dir, u64 *y){
    void *nbrY = NULL;
    static u64 nxtNbr;
    int result = NONE;

    assert(nbrs != NULL && y!=NULL);
    assert(flag == FST || flag == NXT);
    assert(dir == FWD || dir == BWD || flag == FWD||BWD);

    if (dir & FWD > 0){
        if (flag == NXT){   /*el siguiente vecino forward*/
            nbrY = findNbr(nbrs, nxtNbr, dir);
            if (nbrY->hhfNbrs.next != NULL){    /*existe un siguiente*/
                nxtNbr = nbrY->hhfNbrs.next->y;
                *y = nxtNbr;
                result = FWD;
            }
        }else{   /*primer vecino forward*/
            if (nbrs->fNbrs != NULL){
                nxtNbr = nbrs->fNbrs->y;
                *y = nxtNbr;
                result = FWD;
            }
        }
    }
    if(dir & BWD > 0 && result == NONE){
        if (flag == NXT){   /*el sig vecino vecino backward*/
            nbrY = findNbr(nbrs, nxtNbr, dir);
            if (nbrY->hhbNbrs.next != NULL){    /*existe un siguiente*/
                nxtNbr = nbrY->hhbNbrs.next->y;
                *y = nxtNbr;
                result = BWD;
            }
        }else{   /*primer vecino backward*/
            if (nbrs->bNbrs != NULL){
                nxtNbr = nbrs->bNbrs->y;
                *y = nxtNbr;
                result = BWD;
            }
        }
    }
    return result;
}


/* Se aumenta el flujo para con el vecino 'y' por 'vf' cantidad. 
 * Si 'y' es un vecino BWD, el valor del flujo se disminuye por 'vf' cantidad
 * Precondicion: 'y' es vecino. vf > 0
 * Retorno: valor del nuevo flujo que se esta enviando */
u64 nbrhd_increaseFlow(Nbrhd nbrs, u64 y, u64 vf){
    void *nbr = NULL;   /*el vecino*/
    int dir = UNK;      /*direccion en la que se encuentra el vecino*/
    
    assert(nbrs != NULL && vf > 0);
    
    nbr = findNbr(nbrs, y, dir);
    
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
 Precondicion: 'y' es vecino */
u64 nbrhd_getCap(Nbrhd nbrs, u64 y){
    void *nbr = NULL;   /*el vecino*/
    int dir = UNK;        /*direccion en la que se encuentra el vecino*/
    
    assert(nbrs != NULL);

    nbr = findNbr(nbrs, y, dir);
    
    if (dir == BWD)
        nbr = nbr->x;
    
    return nbr->cap;
}


/*devuelve el valor del flujo con el vecino 'y'
 Precondicion: 'y' es vecino */
u64 nbrhd_getFlow(Nbrhd nbrs, u64 y){
    void *nbr = NULL;   /*el vecino*/
    u64 result;
    int dir = UNK;        /*direccion en la que se encuentra el vecino*/
    
    assert(nbrs != NULL);

    nbr = findNbr(nbrs, y, dir);
    
    if (dir == BWD)
        result = nbr->x->flow;
    else
        result = nbr->flow;
    
    return result;
}

/*devuelve la direccion (FWD o BWD) en la que se encuentra el vecino 'y'
 Precondicion: 'y' es vecino */
int nbrhd_getDir(Nbrhd nbrs, u64 y); 
    void *nbr = NULL;   /*el vecino*/
    int *dir = &UNK;        /*direccion en la que se encuentra el vecino*/
    
    assert(nbrs != NULL);

    findNbr(nbrs, y, dir);
    
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
 * Si 'dir' != UNK, busca unicamente en esa direccion; 
 * caso contrario, busca primero por forward y luego por backward.
 * Precondicion: 'y' es vecino */
void *findNbr(Nbrhd nbrs, u64 y, int* dir){
    Fedge fnbr = NULL;
    Bedge bnbr = NULL;
    void *result = NULL;
    
    assert(nbrs != NULL);
    
    if (dir != BWD)         /*busqueda por vecinos forward (dir == UNK|FWD)*/
        HASH_FIND(nbrs->fNbrs->hhfNbrs, nbrs->fNbrs, &(y), sizeof(y), fnbr);
        /* caso dir == FWD, no puede ser fnbr == NULL */
        assert(dir != FWD || fnbr != NULL);
    
    if (fnbr != NULL){      /* se encontro en forward*/
        result = fnbr;
        dir = FWD;
    }else{                  /*busqueda por vecinos backward*/
        HASH_FIND(nbrs->bNbrs->hhfNbrs, nbrs->bNbrs, &(y), sizeof(y), bnbr);
        assert(bnbr != NULL);   /* 'y' no es vecino, error */
        result = &bnbr;
        dir = BWD;
    }
        
    return result;
}

