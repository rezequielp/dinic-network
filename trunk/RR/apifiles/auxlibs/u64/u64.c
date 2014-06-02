#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "u64..h"

#define MAX_D 20    /*maxima cantidad de digitos de un u64*/

/*          Estructura
 */
struct u64St{
    short int num[MAX_D];   /*el nro de 64bit almacenado por digitos separados*/
    short int len;          /*el largo en digitos del nro*/
};

/* 
 *           Funciones
 */


/*Nuevo u64 con valor 0*/
u64 u64_new(){
    u64 n;
    n = (u64) malloc (sizeof(struct u64St));
    assert(n!=NULL);
    n->num[MAX_D-1] = 0;
    n->len = 1;
    return n;
}


/*Destruye un u64*/
void u64_destroy(u64 n){
    assert(n!=NULL);
    free (n);
}


/*conversor de bstring a u64*/
void u64_fromBstr(u64 n, bstring bstr){
    char * c = NULL;
    char cAux[1];
    int i, offset = MAX_D;
    
    assert(n != NULL && bstr != NULL);
    c = bstr2cstr(bstr, '\0');
    n->len = 0;
    offset = MAX_D - blength(bstr);

    for(i=offset; i<MAX_D; i++){
        strncpy(cAux, &c[i-offset], 1);
        n->num[i]= atoi(cAux);
        n->len++;
    }
    
    i=offset;
    while(n->num[i]==0 && i<MAX_D-1){
        n->len--;
        i++;
    } 
    bcstrfree(c);
}


/*al u64 'dst' le asigna el valor de 'src' */
void u64_assign(u64 dst, u64 src){
    int i;

    assert(dst!=NULL && src!=NULL);
    memcpy(dst, src, sizeof(struct u64St)); 
}


/*devuelve una copia de n*/
u64 u64_cpy(u64 n){
    u64 result = NULL;
    assert(n!=NULL);
    
    result = u64_new();
    memcpy(result, n, sizeof(struct u64St)); 
    
    return result;
}


/*conversor de u64 a bstring*/
void u64_toBstr(u64 n, bstring bstr){
    char c[2];
    int i=0;
    
    assert(n!=NULL && bstr!=NULL);
    bassigncstr(bstr, "");
    
    for(i=MAX_D-n->len; i<=MAX_D-1; i++){
        snprintf(c, 2, "%d", n->num[i]);
        bcatcstr(bstr, c);
    }
}


/*suma en 'a' el valor de 'b', error si hay overflow*/
void u64_add(u64 a, u64 b){
    int i = MAX_D;      /*iter u64*/
    int carry = 0;      /*el acarreo en cada suma por digitos*/
    int r = 0;          /*el resultado de sumar 2 digitos mas el acarreo*/
    u64 higher, smaller = NULL;    /*indicador del u64 mas largo y u64 mas corto*/

    assert(a!=NULL && b!=NULL);
    
    /*se identifica el u64 mas largo y el mas corto segun cant de digitos*/
    if(a->len > b->len){
        higher = a;
        smaller = b;
    }
    else{
        higher = b;
        smaller = a;
    }
    /*se suman los digitos hasta el largo del u64 mas corto*/
    for(i=MAX_D-1; i>=MAX_D-smaller->len; i--){
        r = higher->num[i] + smaller->num[i] + carry;
        a->num[i] = r % 10;
        carry = r / 10;
    }  
    /*se agregan los digitos restantes del u64 largo sumando carry*/
    for(; i>=(MAX_D-higher->len); i--){
        r = higher->num[i] + carry;
        a->num[i] = r % 10;
        carry = r / 10;
    }
    
    assert(carry != 1 || i>=0); /*overflow*/   
    
    /*se establece el largo de 'a'*/
    a->len = higher->len;
    if (carry == 1){    /*se alarga el nro 1 digito mas*/
        a->num[i] = 1;
        a->len++;
    }
}


/*resta en 'a' el valor de 'b', error si b>a */
void u64_sub(u64 a, u64 b){
    int i = MAX_D;      /*iter u64*/
    int carry = 0;      /*el acarreo en cada suma por digitos*/
    int r = 0;          /*el resultado de sumar 2 digitos mas el acarreo*/

    assert(a!=NULL && b!=NULL);
    assert(u64_isMax(a, b));    /*b > a, no se puede*/
    
    /*se restan los digitos hasta el largo de b*/
    for(i=MAX_D-1; i>=MAX_D-b->len; i--){
        r = a->num[i] - b->num[i] - carry;
        carry = (r<0);
        a->num[i] = r + (10*carry);
    } 
    /*se agregan los digitos restantes de 'a' resolviendo posible carry*/
    for(; i>=MAX_D-a->len; i--){
        r = a->num[i] - carry;
        carry = (r<0);
        a->num[i] = r+(10*carry);
    }
    /*resuelvo el nuevo largo de 'a'*/
    i++;
    while(a->num[i] == 0 && i<MAX_D-1){
        a->len--;
        i++;
    }
}


/*print de un u64 por stdout*/
void u64_print(u64 n){
    bstring bAux = NULL;
    char * str = NULL;
    
    assert(n!=NULL);
    
    bAux = bfromcstr("");
    u64_toBstr(n, bAux);
    
    str = bstr2cstr(bAux, '\0');
    printf("%s: %id\n", str, blength(bAux));
    
    bdestroy(bAux);
    bcstrfree(str);
}


/*True si a > b o a=b*/
bool u64_isMax(u64 a, u64 b){
    bool r = false;
    int i;
    assert(a!=NULL && b!=NULL);

    if (a->len > b->len){   /*reviso por longitud*/
        r = true;
    }else if (a->len == b->len){
        i = MAX_D-a->len;
        /*hasta que sean distintos o llegue al final del nro*/
        while (i <= MAX_D-1 && a->num[i] == b->num[i]){ 
            i++;
        }
        if(i == MAX_D || a->num[i] > b->num[i]) /*son iguales o a > b*/
            r = true;
    }
    return r;
}


/*True si a < b o a=b*/
bool u64_isMin(u64 a, u64 b){
    bool r = false;
    int i;
    assert(a!=NULL && b!=NULL);

    if (a->len < b->len){   /*reviso por longitud*/
        r = true;
    }else if (a->len == b->len){
        i = MAX_D-a->len;
        /*hasta que sean distintos o llegue al final del nro*/
        while (i <= MAX_D-1 && a->num[i] == b->num[i]){
            i++;
        }
        if(i == MAX_D || a->num[i] < b->num[i]) /*son iguales o a < b*/
            r = true;
    }
    return r;
}


/*True si a = b*/
bool u64_isEqual(u64 a, u64 b){
    bool r = false;
    int i;
    assert(a!=NULL && b!=NULL);

    if (a->len == b->len){  /*reviso por longitud*/
        i = MAX_D-a->len;
        /*hasta que sean distintos o llegue al final del nro*/
        while (i <= MAX_D-1 && a->num[i] == b->num[i]){
            i++;
        }
        if(i == MAX_D)  /*son iguales*/
            r = true;
    }
    return r;
}


/* -1 si a<b, 0 si a=b, 1 si a>b */
int u64_cmp(u64 a, u64 b){
    int result = -1;
    assert(a!=NULL && b!=NULL);
    
    if(u64_isEqual(a, b))
        result = 0;
    else if(u64_isMax(a, b))
        result = 1;
    return result;
} 


/*el maximo entre a y b*/
u64 u64_max(u64 a, u64 b){
    u64 result;
    assert(a!=NULL && b!=NULL);

    result = a;
    if(u64_isMax(b, a))
        result = b;

    return result;
}


/*el minimo entre a y b*/
u64 u64_min(u64 a, u64 b){
    u64 result;
    assert(a!=NULL && b!=NULL);

    result = a;
    if(u64_isMin(b, a))
        result = b;

    return result;
}