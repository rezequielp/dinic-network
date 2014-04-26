#include "API.h"

typedef struct NetworkSt{
    u64 node;                   /* key */
    Edge edge;                  /* value */
    u64 level;                  /* nivel de distancia */
    UT_hash_handle hh;          /* makes this structure hashable */
} Network;

typedef struct NetworkYFlujo{
    Network *net;       /* Network de los nodos para acceder a las aristas */
    u64 flow;           /* Valor del flujo del network */
    u64 source;         /* Vertice fijado como fuente */
    u64 trash;          /* Vertice fijado como resumidero */
    Abb cut;            /* TODO ver por que era abb y no pila o cola*/ /*Un abb de punteros a nodos para el corte*/
    queue path;         /* Camino en forma de un flujo de caracteres */
    u64 tempFlow;       /* Cant de flujo a aumentar (cap min del camino actual) */
} DovahkiinSt;