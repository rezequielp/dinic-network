#include <assert.h>
#include <stdlib.h> 
#include "Stack.h"

struct StackSt{
	Element *top;
    Element *bottom;
	int size;
};

/* Estructura que contiene un elemento y un puntero a la siguiente estructura */
typedef struct ElementSt{
    void *elem;
    Element *next;
}Element;


Stack stack_create (void){
	Stack S = NULL;
    S = (Stack) malloc(sizeof(struct StackSt));
    if (S != NULL){
        S->top = NULL;
        S->bottom = NULL;
        S->size = 0;
    }
	return S;
}

int stack_destroy(Stack S, void *garbage){
    int result = -1;
    int i = 0;
    int sSize;
    void * elem;
    
    assert(S != NULL);
    
    sSize = stack_size(Q);
    while(!stack_isEmpty(S)){
        elem = stack_pop(S);
        if(garbage != NULL){
            garbage[i] = elem;
        }
        i ++;
    }
    if(sSize == i){
        result = i;
        free(S);
        S = NULL;
    }
    return result;
}

int stack_push(Stack S, void *elem){
	Element *new = NULL;
    result = 1; /*0 = esta todo OK*/
    
	new = (Element *) malloc(sizeof(struct ElementSt));
	if(new != NULL){
        new->elem = elem;
        new->next = S->top;
        S->top = new;
        S->size += 1;
        if (S->bottom == NULL){
            S->bottom = new;
        }
        result = 0;
    }
	return result;
}

int stack_bpush (Stack S, void *elem){
    Element *new = NULL;
    result = 1;
    
    assert(S != NULL);
    if (S->bottom == NULL){
        result = stack_push(S, elem);
    }else{
        new = (Element *) malloc(sizeof(struct ElementSt));
        if (new != NULL){
            new->elem = elem;
            new->next = NULL;
            S->bottom->next = new;
            S->bottom = new;
            S->size += 1;
            result = 0;
        }
    }
    return result;
}

void * stack_pop(Stack S){
	Element *aux = NULL;
    void * elem = NULL;
    assert(S != NULL && !stack_isEmpty(S));
    
	aux = S->top;
    if (aux->next != NULL){
        S->top = S->top->next;
    }else{ /*caso ultimo elemento*/
        S->top = NULL;
        S->bottom = NULL;
    }
    elem = aux->elem;
    free(aux);
    S->size -= 1; 
	return elem;
}

int stack_isEmpty(Stack S){
    assert(S != NULL);
	return (S->size == 0);
}

void *stack_top(Stack S){
    assert( S != NULL);
    return(S->top->elem);
}

int stack_size(Stack S){
    assert( S != NULL);
    return (S->size);
}
/* CAPAZ NO SE USE (OBSERVACION: OJO CON POP Y DESTROY POR 
 *TEMA DE MEMORIA)*/
int stack_revert(Stack S1, Stack S2 ){
    Element *aux = NULL;
    int result = 1;
    
    assert((S1 != NULL) && (S2 != NULL));
    assert(stack_isEmpty(S2))

    aux = stack_top(S1);
    while ((stack_size(S1) != stack_size(S2)) && (result != 0)){
        result = stack_push(S2, aux);
        aux = aux->next;
    }
    return result;
}