
/* Estructura de un nodo por forward*/
typedef struct FNodeSt{
    u32 y;      /* El nodo forward de 'x'*/
    u32 cap;    /* La capacidad restante de envio de flujo*/
    u32 flow;   /* El flujo por forward que se esta enviando*/
}FNode;

/* Estructura de un nodo por backward*/
typedef struct BNodeSt{
    FNode * y;    /* Puntero a la direccion de memoria del nodo 'y' en el arbol*/
}BNode;


typedef LadoSt {
       *hashtable forward 
       *hashtable backward 
}lado
