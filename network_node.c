#include "network_node.h"
#inlcude "../auxlibs/bstring/bstrlib.h"

/* Estructura de un nodo por forward*/
typedef struct FNodeSt{
    u32 y;      /* El nodo forward de 'x'*/
    u32 flow;   /* El flujo por forward que se esta enviando*/
    u32 cap;    /* La capacidad restante de envio de flujo*/
}FNode;

/* Estructura de un nodo por backward*/
typedef struct BNodeSt{
    FNode * y;    /* Puntero a la direccion de memoria del nodo 'y' en el arbol*/
}BNode;

/* Estructura de un nodo*/
struct edgeNodeSt{
    u32 x;      /* El nodo en cuestion*/
    Abb *fAbb;  /* Los nodos vecinos por forward*/
    Abb *bAbb;  /* Los nodos vecinos por backward*/
};

typedef struct networkNodeSt{
	edgeNode * edge;
	networkNode * left;
	networkNode * rigth;
}networkNode;

typedef struct edgeNode * bArr;


/* 
 *						FUNCIONES SOBRE NODOS
*/

networkNode networkNode_create(){
	networkNode net;
	node = (networkNode) malloc (sizeof(networkNodeSt));
	node->edge = void edgeNode_create():
	node->left = Leaf;
	node->right = Leaf;
}

/*setea los valores del nodo*/

void networkNode_destroy(networkNode netNode){
	edgeNode_destroy(netNode->edge);
	free(netNode)
}


bool networkNode_compare(networkNode node1, networkNode node2){
	return(edge_getX(node1) == edge_getX(node2))
}

/* 
 *						FUNCIONES SOBRE EDGES
*/

edgeNode edgeNode_create(){
    edgeNode edge;
    
    edge = (edgeNode)malloc(sizeof(struct edgeNodeSt));
    if(edge != NULL){
        edge->fSize = 0;
        edge->bSize = 0;
        edge->fArr = NULL;
        edge->bArr = NULL;
    }
    return edge;
}

void edgeNode_destroy(edgeNode edge){
    
    assert(edge != NULL);    
    while(edge->fSize >= 0){
        free(edge->fArr[edge->fSize-1]);
        edge->fSize -= 1;
    }
    free(edge->fArr);
    free(edge->bArr);
    free(edge);
    edge = NULL;
}

 /*GETS/SETS*/
u32 edge_getX(edgeNode edge){
	return edge->x;
}

void edgeNode_set(edgeNode edge, u32 x, u32 y, u32 cap){
    assert (edge != NULL);
    
}
void edge_setFlow(edgeNode edge,  ){
    
}

u32 edge_getX(edgeNode edge);
u32 edge_getY(edgeNode edge);
u32 edge_getCap(edgeNode edge);
u32 edge_getFlow(edgeNode edge);