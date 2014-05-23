/*Version Beta0.1*/
#ifndef U64_H
#define U64_H

/*TODO evaluar si son unsigned o no.*/

typedef struct u64St *u64;
/*
 * Macros para facilitar la escitura de las funciones de u64
 */
 #define U64_a+b    u64_add(a, b)
 #define U64_a-b    u64_substract(u64_a, u64_b)
 #define U64_a*b    u64_multiply(u64_a, u64_b)
 #define U64_a/b    u64_divide(u64_a, u64_b)
 #define U64_a?b    u64_cmp(u64 a, u64 b)
 #define U64_amb    u64_min(u64 a, u64 b)
 #define U64_aMb    u64_max(u64 a, u64 b)
 #define U64_a<b    u64_isMin(u64 a, u64 b)
 #define U64_a>b    u64_isMax(u64 a, u64 b)
 #define U64_a==b   u64_isEqual(u64 a, u64 b)
 
 /*
  *  constructores
  */
  /*crea un u64 sin datos*/
  
  /*TODO cambiar los int por u32 por el bit de signo*/
u64 u64_new();
/*devuelve una copia del u64*/
u64 u64_copy(u64 n);
/*setean la parte low y la high del u64*/
void u64_setLow(u64 k,int n);
void u64_setHigh(u64 k,int n);
int u64_getLow(u64 k);
int u64_getHigh(u64 k);
void u64_setHL(u64 k, int low, int high);
void u64_destroy(u64 n);

/*
 *   Operaciones matematicas
 */
u64 u64_add(u64 a, u64 b);          /*crea un nuevo u64*/
u64 u64_substrac(u64 a, u64 b);      /*crea un nuevo u64*/
//u64 u64_multiply(u64 a, u64 b);
//u64 u64_divide(u64 a, u64 b);
void u64_cmp(u64 a, u64 b);   /*retorna 1 si a>b, 0 si a==b y -1 si a<b*/
u64 u64_max(u64 a, u64 b);      /*regresa el puntero del mayor*/
u64 u64_min(u64 a, u64 b);      /*regresa el puntero del mayor*/
bool u64_isMax(u64 a, u64 b);  
bool u64_isMin(u64 a, u64 b);
bool u64_isEqual(u64 a, u64 b);
// u64 u64_abs(u64 a);

/*
 *    Miscellaneous functions 
 */
void u64_toBstr(u64 n, bstring bstr);
u64 u64_fromStr(bstring bstr, u64 n);


#endif