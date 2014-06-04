#include <assert.h>
#include <stdlib.h>
#include "queue.h"
#include <stdio.h>

/* Estructura doblemente enlazada, tiene una referencia al primer elemento 
 * y otra al ultimo */

typedef struct ElementSt Element;
/* Estructura que contiene un elemento y un puntero a la siguiente estructura */
struct ElementSt{
	void *elem;
	Element *next;
};

struct queueSt{
	Element *head;
    Element *tail;
	int size;
};

Queue queue_create(void){
	
	Queue Q = NULL;
	
	Q = (Queue) malloc (sizeof(struct queueSt));
	if (Q != NULL){
        Q->head = NULL;
        Q->tail = NULL;
        Q->size = 0;
    }
	return Q;
}

int queue_enqueue(Queue Q, void * q){
	
	Element *new = NULL;
    int result = 1;
	
	new = (Element*) malloc (sizeof(struct ElementSt));
	if (new != NULL){
        new->elem = q;
        new->next = NULL;
        if(queue_isEmpty(Q)){
            Q->head = new;
            Q->tail = Q->head;
        }else{
            Q->tail->next = new;
            Q->tail = new;
        }
        Q->size = (Q->size + 1);
        result = 0;
    }
	return result;
}

void * queue_dequeue(Queue Q){
	Element * aux = NULL;
	void * elem = NULL;
    assert(Q != NULL && !queue_isEmpty(Q));
    
	aux = Q->head;
	if(aux->next != NULL){
		Q->head = Q->head->next;
	}else{
        Q->head = NULL;
        Q->tail = NULL;
    }
    elem = aux->elem;
    free(aux);
	Q->size -= 1;
	return elem;
}

void * queue_head(Queue Q){
	return(Q->head->elem);
}

int queue_isEmpty(Queue Q){
	return (Q->tail == NULL);
}

int queue_destroy (Queue Q, void ** garbage){
    int result = -1;
    int i = 0;
    int qSize;
    void * elem;
    
    assert(Q != NULL);
    
    qSize = queue_size(Q);
    while(!queue_isEmpty(Q)){
        elem = queue_dequeue(Q);
        if(garbage != NULL){
            garbage[i] = elem;
        }
        i ++;
    }
    if(qSize == i){
        result = i;
        free(Q);
        Q = NULL;
    }
	return result;
}

int queue_size (Queue Q){
	return (Q->size);
}

void main(){
    Queue q;
    q=queue_create();
    int  a=1;
    int  b=2;
    int  c=3;
    int  d=4;
    int  e=4;
    int aux, *aux2;
    
    queue_enqueue(q,&a);
    queue_enqueue(q,&b);
    queue_enqueue(q,&c);
    queue_enqueue(q,&d);
    queue_enqueue(q,&e);
    printf("tamaño: %i\n", queue_size(q));
    while(!queue_isEmpty(q)){
        
        aux2 =  queue_head(q);
        aux = * aux2;
        printf("el num es %i\n", aux);
        queue_dequeue(q);
    }
    queue_destroy(q,NULL);
}


