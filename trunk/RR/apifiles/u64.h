#ifndef U64_H
#define U64_H

/** Define los macros de formato standard de C para imprimir valores u64*/  
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

/* Descomentar uno u el otro dependiendo el tipo de int a usar. */
/** Entero de 64 bits sin signo.*/
typedef uint64_t u64;
/*typedef uint_fast64_t u64;*/

/*Usar el macro PRIu64 para poder imprimir el valor de u64
 * ejemplo: printf("%" PRIu64 "\n", my_u64);
*/

/* Maximo valor de u64*/
#define u64_MAX UINT64_MAX /**< Máximo valor posible para u64*/

/* Macros utiles*/
/**Retorna el maximo entre 2 números u64.*/
#define u64_max(a, b) ((a) > (b) ? (a) : (b))
/**Retorna el minimo entre 2 números u64.*/
#define u64_min(a, b) ((a) < (b) ? (a) : (b))

#endif
