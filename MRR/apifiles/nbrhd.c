#include "network_node.h"
#include "../auxlibs/bstring/bstrlib.h"

/* Estructura de un nodo por forward*/
typedef struct FedgeSt{
    u64 y;          /* El nodo forward de 'x', se usa como key en la hash*/
    u64 cap;        /* La capacidad restante de envio de flujo*/
    u64 flow;       /* El flujo por forward que se esta enviando*/
    UT_hash_handler hh;
} Fedge;

/* Estructura de un nodo por backward*/
typedef struct BedgeSt{
    u64 y;          /* key */
    Fedge * x;      /* Puntero a la entrada 'x' de la fhash del nodo 'y'  */
    UT_hash_handler hh;
} Bedge;

/* Estructura de la vecindad de un nodo*/
struct NeighbourhoodSt{
    Fedge *fNbrs;   /* vecinos forward*/
    Bedge *bNbrs;   /* vecinos backward*/
};


/* Funciones del modulo*/

static Fedge fedge_create(u32 y, u32 cap);
static void fedge_destroy(Fedge fedge);
static u32 fedge_getFlow(Fedge fedge);
static void fedge_addFlow(Fedge fedge, u32 flow);
static u32 fedge_getCap(Fedge fedge);
static u32 fedge_getY(Fedge fedge); 

static Bedge bedge_create(u32 y, Edge * neighbor);
static void bedge_destroy(Bedge bedge);
static u32 bedge_getFlow(Bedge bedge);
static void bedge_addFlow(Bedge bedge, u32 flow);
static u32 bedge_getCap(Bedge bedge);
static u32 bedge_getY(Bedge bedge); 


Bedge bedge_create(u32 y, Edge * neighbor){
	Bedge bedge;
	
	bedge = (Bedge) malloc(sizeof(BedgeSt))
	if(bedge != NULL){
		bedge->y = neighbor;
	}
}

void bedge_create(Bedge bedge){
	free(bedge);
}




/* funciones sobre Edges*/
Edge neighbor_create(u32 x){
    Edge neighbor;
    
    neighbor = (Edge)malloc(sizeof(struct EdgeSt));
    if(neighbor != NULL){
        neighbor->x = x;
        neighbor->fAbb = abb_create();
        neighbor->bAbb = abb_create();
    }
    return neighbor;
}

void neighbor_destroy(Edge neighbor){
    Fedge * fedges;
    
    assert(neighbor != NULL);
    
    fSize = abb_size(neighbor->fAbb);
    fedges = (Fedge) malloc(fSize * sizeof(struct FedgeSt));
    if(fedges != NULL){
        fSize = abb_destroy(fAbb, fedges);
    }
    while (fSize > 0){
        free(fedges[fSize - 1]);
        fSize --;
    }
    abb_destroy(neighbor->bAbb, NULL);
    free(neighbor->bAbb);
    if (fSize == 0){
        free(neighbor->fAbb);
        free(neighbor);
        neighbor = NULL;
    }
    free(fedges);
}

void neighbor_set(Edge xEdge, Edge yEdge, u32 cap){
    Fedge yFedge; /*nodo 'y' como forward node de 'x' */
    Bedge xBedge; /*nodo 'x' como backward node de 'y' */
    u32 x;
    u32 y;
    
    assert(xEdge != NULL);
    assert(yEdge != NULL);
    
    /*Agregado de forward en xEdge*/
    y = neighbor_getX(yEdge);
    yFedge = fedge_create(y, cap, 0);
    abb_add(xEdge->fAbb, y, yFedge);
    
    /*Agregado de backward en xEdge*/
    x = neighbor_getX(xEdge);
    xBedge = bedge_create(yEdge);
    abb_add(neighbor->bAbb, x, bedge);
}

 /*GETS/SETS*/
u32 neighbor_getX(Edge neighbor){
    assert(neighbor != NULL);
    return (neighbor->x);
}

void neighbor_setFlow(Edge neighbor, u32 y, u32 flow, int direction){
    Abb tree;
    Fedge yedge;
    
    assert(neighbor != NULL);
    
    if(direction == FORWARD){
        fedge_setFlow(abb_search(neighbor->fAbb, y), flow);
    }else{
        bedge_setFlow(abb_search(neighbor->bAbb, y), flow);
    }
}

/*La capacidad solo se pide de lados forwards*/
u32 neighbor_getCap(Edge neighbor, u32 y){
    
    assert(neighbor != NULL)

    yedge = abb_search(neighbor->fAbb, y);
    return fedge_getCap(yedge);
}

u32 neighbor_getFlow(Edge neighbor , u32 y, int direction){
    Fedge yedge;
    
    assert(neighbor != NULL);
    
    if(direction == FORWARD){
        yedge = abb_search(neighbor->fAbb, y);
    }else{
        yedge = *(abb_search(neighbor->bAbb, y));/*lo apuntado por*/
    }
    return yedge->flow;
}


/*funciones sobre edges*/

Fedge fedge_create(u32 y, u32 cap){
	Fedge fedge;
	
	fedge = (Fedge) malloc(sizeof(FedgeSt));
	if(fedge != NULL){
		fedge->y = y;
		fedge->cap = cap;
		fedge->flow = 0;
	}
	return fedge;
}
void fedge_create(Fedge fedge){
	free(fedge);
}
u64 fedge_getFlow(Fedge fedge){
	return(fedge->flow);
}
void fedge_addFlow(Fedge fedge, u32 flow){
	assert(fedge != NULL);
	
	fedge->flow += flow;
	
	assert(fedge->flow <= fedge_getCap(fedge));
}
u32 fedge_getCap(Fedge fedge){
	return (fedge->cap);
}
u32 fedge_getY(Fedge fedge){
	return(fedge->y);
}

Bedge bedge_create(u32 y, Edge * neighbor){
	Bedge bedge;
	
	bedge = (Bedge) malloc(sizeof(BedgeSt))
	if(bedge != NULL){
		bedge->y = neighbor;
	}
}
void bedge_create(Bedge bedge){
	free(bedge);
}
u32 bedge_getFlow(Bedge bedge){
	return(*(fedge)->flow);
}
void bedge_addFlow(Bedge bedge, u32 flow){
	assert(fedge != NULL && *(bedge) != NULL);
	
	*(fedge)->flow += flow;
	
	assert(*(fedge)->flow <= bedge_getCap(bedge));
}
u32 bedge_getCap(Bedge bedge){
	return (*(bedge)->cap);
}
u32 bedge_getY(Bedge bedge){
	return(*(bedge)->y);
}









