#include "cut.h"

struct cutNodeSt{
    u32 ancestor;
    Edge edge;
    int direction;
};

CutNode cutNode_create(u32 ancestor, Edge edge, int direction){
    CutNode cutNode = NULL;
    
    assert(edge != NULL);

    cutNode = (CutNode) malloc(sizeof(cutNodeSt));
    if(cutNode != NULL){
        cutNode->ancestor = ancestor;
        cutNode->edge = edge;
        cutNode->direction = direction;
    }
    return cutNode;
}

void cutNode_destroy(CutNode cutNode){
    assert(cutNode != NULL);
    free(cutNode);
}

/*GETS*/
u32 cutNode_getAncestor(CutNode cutNode){
    assert(cutNode != NULL);
    return cutNode->ancestor ;
}

Edge cutNode_getEdge(CutNode cutNode){
    assert(cutNode != NULL);
    return cutNode->edge;
}

int cutNode_getDirection(CutNode cutNode){
    assert(cutNode != NULL);
    return cutNode->direction;
}
