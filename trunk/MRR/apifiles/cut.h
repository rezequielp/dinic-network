#ifndef _CUT_H
#define _CUT_H

/* Este TAD es un arbol binario de busqueda por punteros a nodos, 
 * para tareas sobre cortes.
 */

typedef struct CutNodeSt * CutNode;

CutNode cutNode_create(u32 Ancestor, Edge edge, int direction);
void cutNode_destroy(CutNode cutNode);

/*GETS*/
u32 cutNode_getAncestor(CutNode cutNode);
Edge cutNode_getEdge(CutNode cutNode);
int cutNode_getDirection(CutNode cutNode);
u32 cutNode_getX(CutNode cutNode);/*TODO*/
#endif