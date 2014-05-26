/*Version Beta1.1*/
#ifndef U64_H
#define U64_H

#include "../bstring/bstrlib.h"
#include <stdbool.h>


typedef struct u64St *u64;

/* creacion y copia 
 */
u64 u64_new();  /*Nuevo u64 con valor 0*/

void u64_destroy(u64 n);    /*Destruye un u64*/

void u64_fromBstr(u64 n, bstring bstr); /*conversor de bstring a u64*/

void u64_assign(u64 dst, u64 src);  /*a 'dst' le asigna el valor de 'src' */

u64 u64_cpy(u64 n);     /*devuelve una copia de n*/

void u64_toBstr(u64 n, bstring bstr);   /*conversor de u64 a bstring*/


/* operadores aritmeticos
 */
void u64_add(u64 a, u64 b); /*suma en 'a' el valor de 'b', error si hay overflow*/
void u64_sub(u64 a, u64 b); /*resta en 'a' el valor de 'b', error si b>a */


/* impresion
 */
void u64_print(u64 n);  /*print de un u64 por stdout (estaria bueno programarlo 
                            con las funciones de print de bstring*/

  
/* comparadores
 */
bool u64_isMax(u64 a, u64 b);   /*True si a > b o a=b*/
bool u64_isMin(u64 a, u64 b);   /*True si a < b o a=b*/
bool u64_isEqual(u64 a, u64 b); /*True si a = b*/
int u64_cmp(u64 a, u64 b);      /* -1 si a<b, 0 si a=b, 1 si a>b */
u64 u64_max(u64 a, u64 b);      /*el maximo entre a y b*/
u64 u64_min(u64 a, u64 b);      /*el minimo entre a y b*/

#endif