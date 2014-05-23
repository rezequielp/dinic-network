#ifndef LADO_H
#define LADO_H

typedef struct LadoSt * Lado;

/* Constructores
*/
Lado lado_new(u64 x, u64 y, u64 c);
void lado_destroy(Lado edge);


/*  Operaciones
*/
u64 lado_getY(Lado edge);
u64 lado_getX(Lado edge);
u64 lado_getCap(Lado edge);
#endif