#ifndef QUEUE_H
#define QUEUE_H

typedef struct QueueSt *Queue;
typedef struct ElementSt Element;

/**Crea una cola sin elementos.
    \return Un puntero a una cola vacia.*/
Queue queue_create (void);

/**Agrega un elemento del tipo Alpha al final de la cola.
    \param Q Cola a la cual hay que agregarle el elemento.
    \param q Elemento a ser agregado a la cola.
    \return  0 si se agrego.\n 1 caso contrario.*/
int queue_enqueue(Queue Q, void *elem);

/**Quita el ultimo elemento de la cola. Como precondicion la cola no
debe estar vacia ni ser nula.
    \param Q Cola a la cual se le va a quitar el ultimo elemento.
    \return Elemento que se le quita al a cola.*/
void * queue_dequeue (Queue Q);

/**Muestra la cabeza de la cola. 
    \param Q Cola de la cual se le va a mostrar la cabeza.
    \return El elemento correspondiente a la cabeza de la cola.*/
void *queue_head (Queue Q);

/**Verifica que la cola este vacia.
    \param Q Cola sobre la cual se hará la verificación.
    \return True si Q esta vacia.\n
    False caso contrario.*/
int queue_isEmpty (Queue Q);

/**Destruye y libera la memoria de la cola Q preservando los elementos. Esta funcion no elimina los elementos 
que se agregaron a la cola. Es tarea del llamador destruirlos si se consideraba necesario. 
Esto es asi porque no se sabe cual es el tipo de las cosas encoladas y porque no es la 
funcion de queue_destroy.
    \param Q Cola a destruir.
    \param garbage  Puntero a un arreglo de punteros de elementos que se agregaron a la cola. Si se pasa 
    NULL, queue_destroy no guardara los elementos en ningun lado. El llamador debe tener cuidado de no perder
    la referencia a esta memoria para no generar memory leaks.
    \return Cantidad de elementos (del tipo ElementSt) que se eliminaron efectivamente de la cola.*/
int queue_destroy (Queue Q, void ** garbage);

/**Devuelve la cantidad de elementos de la cola.
        \param Q Cola en la cual se contabilizaran los elementos.
        \return Cantidad de elementos de la cola.*/
int queue_size (Queue Q);

/** Permuta las colas.
 * \param fstQ Una cola.
 * \param sndQ La otra cola.
 */
void queue_swap (Queue *fstQ, Queue *sndQ);


#endif
