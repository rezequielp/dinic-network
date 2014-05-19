
/* Estructura de un nodo*/
typedef struct EdgeSt * Edge;

Edge edge_create(u32);
void edge_destroy(Edge edge);

/*TODO*/
FNode edge_getFNext(u64 x);
BNode edge_getBNext(u64 x);

u64 edge_getFName(FNode fnode);
u64 edge_getBName(BNode bnode);



void edge_set(Edge xEdge, Edge yEdge, u32 cap);

void edge_setFlow(Edge edge, u32 y, u32 flow, int direction;

u64 edge_getX(Edge edge);
u64 edge_getCap(Edge edge, u32 y);
u64 edge_getFFlow(FNode fnode);
u64 edge_getBFlow(BNode bnode);