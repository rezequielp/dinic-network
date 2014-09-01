#ifndef STACK_H
#define STACK_H

/** \file _stack.h
 * La libreria _stack proporciona una serie de herramientas para almacenar 
 * cualquier tipo de elemento en el orden LIFO(Last In, First Out), ultimo en 
 * entrar primero en salir, correspondiente a lo que llamamos cotidianamente 
 * como una pila. La particularidad de esta pila es que no importa el tipo de 
 * elemento que ordene ni se sabe que elemento se ordena. Por este motivo no se 
 * puede destruir en el caso de querer sacar el elemento superior o querer 
 * liberar la memoria de toda la estructura. Por lo cual, estas dos funciones 
 * devuelven el elemento o un arreglo de los elementos que se desean eliminar 
 * para que el llamador los elimine si lo considera pertinente.
*/

/** \struct Stack
 * Puntero a una pila.
 */
typedef struct StackSt *Stack;

/* Crea una pila vacia.
 * return: Un puntero a la pila creada.
*/
Stack stack_create(void);

/* Destruye y libera la memoria de la pila S preservando los elementos. 
 * Esta funcion no elimina los elementos que se agregaron en la pila. Es tarea 
 * del llamador destruirlos si se consideraba necesario. Esto es asi porque 
 * no se sabe cual es el tipo de las cosas apiladas y porque no es una tarea 
 * de stack_destroy.
 * S La pila a destruir.
 * garbage  Puntero a un arreglo de punteros de elementos que se 
 * agregaron a la pila. Si es NULL, stack_destroy no guardara los elementos en 
 * ningun lado. El llamador debe tener cuidado de no perder la referencia a esta
 * memoria para no generar memory leaks.
 * pre: La pila S no es nula.
 * return: Cantidad de elementos que se eliminaron efectivamente de la pila.
 */
int stack_destroy(Stack S, void **garbage);

/* Agrega un elemento del tipo Alpha a la parte superior de la pila.
 * S Pila a la cual hay que agregarle el elemento.
 * elem Elemento a ser agregado a la pila.
 * pre: La pila S no es nula.
 * return:  1 si se agrego. 
 *          0 caso contrario.
 */
int stack_push(Stack S, void *elem);

/* Quita el elemento superior de la pila. 
 * S Pila a la cual se le va a quitar el elemento superior.
 * pre: La pila S no debe estar vacia ni ser nula.
 * return: Elemento que se le quita a la pila.
 */
void * stack_pop(Stack S);

/* Verifica que la pila este vacia.
 * S Pila sobre la cual se hará la verificación.
 * pre: La pila S no es nula.
 * return:  True si S esta vacia.
            False caso contrario.
*/
int stack_isEmpty(Stack S);

/* Muestra el elemento superior de la pila.
 * S Pila de la cual se va a mostrar el elemento superior.
 * pre: La pila S no es nula.
 * return: El elemento correspondiente a la cabeza de la pila.
 */
void *stack_top(Stack S);

/* Devuelve la cantidad de elementos de la pila.
 * S Pila en la cual se contabilizaran los elementos.
 * pre: La pila S no es nula.
 * return: Cantidad de elementos de la pila.
 */
int stack_size(Stack S);

/* Devuelve en S2 la pila S1 revertida.
 * Funcion que invierte el orden de la pila utilizando una nueva pila. El 
 * llamador debe proporcionar las pilas y se encarga de liberarlas.
 * WARNING Es altamente ineficiente en cuanto a la memoria que ocupa.
 * pre: Las pilas no son nulas y S2 esta vacia.
 * return:  1 si no hubo error.
 *          0 caso contrario.
 */
int stack_revert(Stack S1, Stack S2 );

/* Muestra un elemento y corre el visor al siguiente, NULL si no existe.
 * S Pila sobre la cual se itera.
 * pre: La pila S no es nula.
 * return: Elemento al cual esta apuntando el visor.
 * WARNING Esto se usa SOLAMENTE para iterar en el stack, si se elimina el 
 * elemento devuelto queda un stack inconsistente.*/
void * stack_nextItem(Stack S);

/* Situa el visor en la parte superior de la pila.
 * S Pila sobre la cual se reseteara el visor.
 * pre: La pila S no es nula.
 */
void stack_resetViewer(Stack S);

#endif