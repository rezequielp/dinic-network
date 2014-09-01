#include <assert.h>
#include <stdlib.h>
#include "_queue.h"
#include <stdio.h>

/** \file _queue.c
La libreria _queue proporciona una serie de herramientas para almazenar cualquier tipo de elemento
en el orden FIFO(First In, First Out) primero en entrar primero en salir, correspondiente a lo que 
llamamos cotidianamente como una cola. La particularidad de esta cola es que no importa el tipo 
de elemento que ordene ni se sabe que elemento se ordena. Por este motivo no se puede 
destruir en el caso de querer sacar la cabeza de la cola o querer liberar la memoria de toda la estructura.
Por lo cual, estas dos funciones devuelven el elemento o un arreglo de los elementos que se desean eliminan
para que el llamador los elimine si lo considera pertinente.
*/
 
typedef struct ElementSt Element;

 /** Estructura que contiene un elemento Alpha y un puntero a la siguiente estructura. 
 El tipo cola esta formado por un "encadenamieto" de esta estructura.*/
struct ElementSt{
    void *elem;/**< Puntero al emento de esta estructura. Al ser del tipo void pointer puede apuntar a cualquier cosa.*/
    Element *next;/**<Puntero a la siguiente estructura del tipo ElementSt.*/
};

/**Estructura principal queue. Si bien es una cola, mantiene un puntero al primer elemento
 y al ultimo elemento de la cola solamente para propocitos de optimizacion y futuros upgrades.*/
struct QueueSt{
    Element *head;/**<Puntero al primer elemento de la cola.*/
    Element *tail;/**<Puntero al ultimo elemento de la cola.*/
    int size; /**<Cantidad de elementos en la cola.*/
};

/**Crea una cola sin elementos.
    \return Un puntero a la cola creada.*/
Queue queue_create(void){
    
    Queue Q = NULL;
    
    Q = (Queue) malloc (sizeof(struct QueueSt));
    if (Q != NULL){
        Q->head = NULL;
        Q->tail = NULL;
        Q->size = 0;
    }
    return Q;
}

/**Agrega un elemento del tipo Alpha al final de la cola.
    \param Q Cola a la cual hay que agregarle el elemento.
    \param q Elemento a ser agregado a la cola.
    \return  0 si se agrego.\n 1 caso contrario.*/
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

/**Quita la cabeza de la cola. Como precondicion la cola no
debe estar vacia ni ser nula.
    \param Q Cola a la cual se le va a quitar el ultimo elemento.
    \return Elemento que se le quita al a cola.*/
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

/**Muestra la cabeza de la cola. 
    \param Q Cola de la cual se le va a mostrar la cabeza.
    \return El elemento correspondiente a la cabeza de la cola.*/
void * queue_head(Queue Q){
    return(Q->head->elem);
}

/**Verifica que la cola este vacia.
    \param Q Cola sobre la cual se hará la verificación.
    \return True si Q esta vacia.\n
    False caso contrario.*/
int queue_isEmpty(Queue Q){
    return (Q->tail == NULL);
}

/**Destruye y libera la memoria de la cola Q preservando los elementos. Esta funcion no elimina los elementos 
que se agregaron a la cola. Es tarea del llamador destruirlos si se consideraba necesario. 
Esto es asi porque no se sabe cual es el tipo de las cosas encoladas y porque no es la 
funcion de queue_destroy.
    \param Q Cola a destruir.
    \param garbage  Puntero a un arreglo de punteros de elementos que se agregaron a la cola. Si se pasa 
    NULL, queue_destroy no guardara los elementos en ningun lado. El llamador debe tener cuidado de no perder
    la referencia a esta memoria para no generar memory leaks.
    \return Cantidad de elementos (del tipo ElementSt) que se eliminaron efectivamente de la cola.*/
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

/**Devuelve la cantidad de elementos de la cola.
        \param Q Cola en la cual se contabilizaran los elementos.
        \return Cantidad de elementos de la cola.*/
int queue_size (Queue Q){
    return (Q->size);
}

/** Permuta las colas.
 * \param fstQ Una cola.
 * \param sndQ La otra cola.
 */
void queue_swap (Queue *fstQ, Queue *sndQ){
    Queue qAux = *fstQ;    
    *fstQ = *sndQ;
    *sndQ = qAux;
}
