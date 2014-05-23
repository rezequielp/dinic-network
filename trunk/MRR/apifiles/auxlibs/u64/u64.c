#include "u64.h"

typedef struct u64St{
    int high;
    unsigned int low;
};
 /*TODO*/
 /* Hacer el caso de u64 negativos*/
/*TODO*/

 /*
  *  constructores
  */
u64 u64_new(){
    u64 n;
    n = (u64) malloc (sizeof(struct u64St))
    u64_setHL(k,0,0);
    return n;
}
u64 u64_copy(u64 n){
    u64 k;
    k = u64_new();
    u64_setHL(k, n->low, n->high);
    return k;
}

void u64_setLow(u64 k, int n){
    k->low = n;
}

void u64_setHigh(u64 k, int n){
    k->high = n;
}

void u64_setHL(u64 k, int low, int high){
    k->high = high;
    k->low = low;

}

int u64_getLow(u64 k){
    return k->low;
}
int u64_getHigh(u64 k){
    return k->high;
}


void u64_destroy(u64 n){
    free (n);
}

u64 u64_fromInt(int n){
    u64 k;
    k = u64_new();
    u64_setLow(k, n);
    return k;
}


/*Operaciones matematicas 
 */
 
u64 u64_add(u64 a, u64 b){
    int p,q,r;
    u64 k;
    k = u64_new();
    k->low = a->low + b->low;
    k->high = a->high + b->high;
    if(k->low < min(a,b))
        k->high++;
    return k;
}

u64 u64_substrac(u64 a, u64 b){
    int p,q,r;
    u64 k;
    k = u64_new();
    k->low = a->low + b->low;
    k->high = a->high + b->high;
    if(k->low > max(a,b))
        k->high--;
    return k;
}

/* TODO
// u64 u64_multiply(u64 a, u64 b){
    // k->low = a->low * b->low;
    // k->high = a->high * b->high;
    // //TODO calcular el carriage de low
    // return k;
// }

// u64 u64_divide(u64 a, u64 b){

// }
*/

int u64_cmp(u64 a, u64 b){
    int result = 0;
    if(u64_isMax(a, b))
        result =1;
    if(u64_isMin(a, b))
    result =-1;
    return result;
}    

u64 u64_max(u64 a, u64 b){
    u64 result = a;
    if(u64_isMax(b, a))
        result = b;
    return result;
}

u64 u64_min(u64 a, u64 b){
    u64 result = a;
    if(u64_isMin(b, a))
        result = b;
    return result;
}

bool u64_isMax(u64 a, u64 b){
    return ((b->high < a->high) || (a->high==b->high && b->low < a->low));
}

bool u64_isMin(u64 a, u64 b){
    return ((b->high > a->high) || (a->high==b->high && b->low > a->low));
}

bool u64_isEqual(u64 a, u64 b){
    return (a->low == b->low && a->high == b->high);
}

/* TODO
u64 u64_abs(u64 a){}
*/


char u64_toString(n, * cStr){
    char * lowAux[4];
    char * highAux[4];
    bstring bstr;
    
    snprintf(lowAux, 4, "%d", n->low);
    snprintf(highAux, 4, "%d", n->high);
    
    bcatcstr(bstr, highAux);
    bcatcstr(bstr, lowAux);
    cStr = bstr2cstr(bstr, NULL);
    bdestroy(bstr);
    return cStr;
}

u64 u64_fromBstr(char *bstr){
    
}




