
/* Estructura de un nodo*/
typedef struct EdgeSt * Edge;

Edge edge_create(u32);
void edge_destroy(Edge edge);

/*ver*/
void edge_set(Edge xEdge, Edge yEdge, u32 cap);

void edge_setFlow(Edge edge, u32 y, u32 flow, int direction;

u32 edge_getX(Edge edge);
u32 edge_getCap(Edge edge, u32 y);
u32 edge_getFlow(Edge edge , u32 y, int direction);