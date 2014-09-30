#include <assert.h>
#include <stdlib.h>
#include "_queue.h"
#include <stdio.h>

/** Estructura que encapsula un elemento que se encola. 
 * Contiene un elemento Alpha y un puntero a la siguiente estructura. El tipo 
 * cola esta formado por un "encadenamieto" de esta estructura.
 */
typedef struct QElemSt{
    void *elem;             /**<Puntero al elemento de esta estructura.*/
    struct QElemSt *next;   /**<Puntero a la siguiente estructura.*/
}QElem;

/**Estructura principal de una queue. 
 * Si bien es una cola, mantiene un puntero al primer elemento y al ultimo 
 * elemento de la cola solamente para propósitos de optimización y futuros 
 * upgrades.
 */
struct QueueSt{
    QElem *head;  /**<Puntero al primer elemento de la cola.*/
    QElem *tail;  /**<Puntero al ultimo elemento de la cola.*/
    int size;     /**<Cantidad de elementos en la cola.*/
};


/** Crea una cola sin elementos.
 * \return Un puntero a una cola vacía.
 */
Queue queue_create(void){
    Queue Q = NULL; /*La nueva cola*/
    
    Q = (Queue) malloc (sizeof(struct QueueSt));
    if (Q != NULL){
        Q->head = NULL;
        Q->tail = NULL;
        Q->size = 0;
    }
    return Q;
}

/** Agrega un elemento del tipo Alpha al final de la cola.
 * \param Q Cola a la cual hay que agregarle el elemento.
 * \param q Elemento a ser agregado a la cola.
 * \pre Q no es una cola nula.
 * \return  1 si se agregó.\n 
 *          0 caso contrario.
 */
int queue_enqueue(Queue Q, void * q){
    QElem *new = NULL;  /*El nuevo elemento a apilar*/
    int result = 0;     /*1 = esta todo OK*/
    
    new = (QElem*) malloc (sizeof(struct QElemSt));
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
        Q->size ++;
        result = 1;
    }
    return result;
}

/** Quita el ultimo elemento de la cola. 
 * \param Q Cola a la cual se le va a quitar el ultimo elemento.
 * \pre Q no debe estar vacía ni ser nula.
 * \return Elemento que se le quita a la cola.
 */
void * queue_dequeue(Queue Q){
    QElem *aux = NULL;  /*Puntero auxiliar para no perder referencias*/
    void *elem = NULL;  /*El elemento de la estructura encolada*/
    
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
    Q->size --;
    return elem;
}

/** Muestra la cabeza de la cola.
 * \param Q Cola de la cual se le va a mostrar la cabeza.
 * \pre Q no es una cola nula.
 * \return El elemento correspondiente a la cabeza de la cola.
 */
void * queue_head(Queue Q){
    void * result = NULL;  /*El elemento cabecera que retorno*/
    assert(Q!=NULL );
    if (Q->head != NULL)
        result = Q->head->elem;
    return result;
    
}

/** Verifica que la cola este vacía.
 * \param Q Cola sobre la cual se hará la verificación.
 * \pre Q no es una cola nula.
 * \return  True si Q esta vacía.\n
 *          False caso contrario.
 */
int queue_isEmpty(Queue Q){
    assert(Q != NULL);
    return (Q->tail == NULL);
}

/** Destruye y libera la memoria de la cola Q preservando los elementos. 
 * Esta función no elimina los elementos que se agregaron a la cola. Es tarea 
 * del llamador destruirlos si se consideraba necesario. Esto es así porque no 
 * se sabe cual es el tipo de las cosas encoladas y porque no es una tarea 
 * de queue_destroy.
 * \param Q Cola a destruir.
 * \param garbage  Puntero a un arreglo de punteros de elementos que se 
 * agregaron a la cola. Si es NULL, queue_destroy no guardara los elementos en 
 * ningún lado. El llamador debe tener cuidado de no perder la referencia a esta
 * memoria para no generar memory leaks.
 * \pre Q no es una cola nula.
 * \return Cantidad de elementos que se eliminaron efectivamente de la cola.*/
int queue_destroy (Queue Q, void ** garbage){
    int result = -1;    /*Retorno de la operacion*/
    int i = 0;          /*Contador de elementos que despilo*/
    int qSize;          /*Tamaño de la cola*/
    void *elem;         /*El elemento que se descola*/
    
    assert(Q != NULL);
    
    qSize = queue_size(Q);
    /*Hasta vaciar la cola, despilo los elementos*/
    while(!queue_isEmpty(Q)){
        /*Si quiere referencia a los elementos, se los dejo en garbage*/
        elem = queue_dequeue(Q);
        if(garbage != NULL){
            garbage[i] = elem;
        }
        i ++;
    }
    /*Si descole todos los elementos, ya puedo destruir la cola*/
    if(qSize == i){
        result = i;
        free(Q);
        Q = NULL;
    }
    return result;
}

/** Devuelve la cantidad de elementos de la cola.
 * \param Q Cola en la cual se contabilizaran los elementos.
 * \pre Q no es una cola nula.
 * \return Cantidad de elementos de la cola.
 */
int queue_size (Queue Q){
    assert(Q != NULL);
    return (Q->size);
}

/** Permutación entre las colas.
 * \param Q1 Una cola.
 * \param Q2 La otra cola.
 */
void queue_swap (Queue *Q1, Queue *Q2){
    Queue qAux = *Q1;    
    *Q1 = *Q2;
    *Q2 = qAux;
}
