#ifndef QUEUE_H
#define QUEUE_H

/** \file _queue.h
 * La librería _queue proporciona una serie de herramientas para almacenar 
 * cualquier tipo de elemento en el orden FIFO(First In, First Out), primero en
 * entrar primero en salir, correspondiente a lo que llamamos cotidianamente 
 * como una cola. La particularidad de esta cola es que no importa el tipo de 
 * elemento que ordene ni se sabe que elemento se ordena. Por este motivo no se 
 * puede destruir en el caso de querer sacar la cabeza de la cola o querer 
 * liberar la memoria de toda la estructura. Por lo cual, estas dos funciones 
 * devuelven el elemento o un arreglo de los elementos que se desean eliminan
 * para que el llamador los elimine si lo considera pertinente.
 */

/** \struct Queue
 * Puntero a una cola.
 */
typedef struct QueueSt *Queue;

/* Crea una cola sin elementos.
 * return: Un puntero a una cola vacía.
 */
Queue queue_create (void);

/* Agrega un elemento del tipo Alpha al final de la cola.
 * Q Cola a la cual hay que agregarle el elemento.
 * elem Elemento a ser agregado a la cola.
 * pre: Q no es una cola nula.
 * return:  1 si se agrego. 
 *          0 caso contrario.
 */
int queue_enqueue(Queue Q, void *elem);

/* Quita el ultimo elemento de la cola. 
 * Q Cola a la cual se le va a quitar el ultimo elemento.
 * pre: Q no debe estar vacía ni ser nula.
 * return: Elemento que se le quita a la cola.
 */
void * queue_dequeue (Queue Q);

/* Muestra la cabeza de la cola.
 * Q Cola de la cual se le va a mostrar la cabeza.
 * pre: Q no es una cola nula.
 * return: El elemento correspondiente a la cabeza de la cola.
 */
void *queue_head (Queue Q);

/* Verifica que la cola este vacía.
 * Q Cola sobre la cual se hará la verificación.
 * pre: Q no es una cola nula.
 * return:  True si Q esta vacía.
 *          False caso contrario.
 */
int queue_isEmpty (Queue Q);

/* Destruye y libera la memoria de la cola Q preservando los elementos. 
 * Esta función no elimina los elementos que se agregaron a la cola. Es tarea 
 * del llamador destruirlos si se consideraba necesario. Esto es así porque no 
 * se sabe cual es el tipo de las cosas encoladas y porque no es una tarea 
 * de queue_destroy.
 * Q Cola a destruir.
 * garbage  Puntero a un arreglo de punteros de elementos que se 
 * agregaron a la cola. Si es NULL, queue_destroy no guardara los elementos en 
 * ningún lado. El llamador debe tener cuidado de no perder la referencia a esta
 * memoria para no generar memory leaks.
 * pre: Q no es una cola nula.
 * return: Cantidad de elementos que se eliminaron efectivamente de la cola.*/
int queue_destroy (Queue Q, void ** garbage);

/* Devuelve la cantidad de elementos de la cola.
 * Q Cola en la cual se contabilizaran los elementos.
 * pre: Q no es una cola nula.
 * return: Cantidad de elementos de la cola.
 */
int queue_size (Queue Q);

/* Permutación entre las colas.
 * Q1 Una cola.
 * Q2 La otra cola.
 */
void queue_swap (Queue *Q1, Queue *Q2);

#endif
