#ifndef STACK_H
#define STACK_H

typedef struct StackSt *Stack;

/**Crea una pila vacia.
    \return Un puntero a la pila creada.*/
Stack stack_create(void);

/**Destruye y libera la memoria de la pila S preservando los elementos. Esta funcion no elimina los elementos 
que se agregaron en la pila. Es tarea del llamador destruirlos si se consideraba necesario. 
Esto es asi porque no se sabe cual es el tipo de las cosas appiladas y porque no es la 
funcion de stack_destroy.
    \param S La pila a destruir.
    \param garbage  Puntero a un arreglo de punteros de elementos que se agregaron a la pila. Si se pasa 
    NULL, stack_destroy no guardara los elementos en ningun lado. El llamador debe tener cuidado de no perder
    la referencia a esta memoria para no generar memory leaks.
    \return Cantidad de elementos (del tipo ElementSt) que se eliminaron efectivamente de la pila.*/
int stack_destroy(Stack S, void **garbage);

/**Agrega un elemento del tipo Alpha a la parte superior de la pila.
    \param S Pila a la cual hay que agregarle el elemento.
    \param elem Elemento a ser agregado a la pila.
    \return  0 si se agrego.\n 1 caso contrario.*/
int stack_push(Stack S, void *elem);

/**Quita el elemento superior de la pila. Como precondicion la pila no
debe estar vacia ni ser nula.
    \param S Pila a la cual se le va a quitar el elemento superior.
    \return Elemento que se le quita a la pila.*/
void * stack_pop(Stack S);

/**Muestra un elemento y corre el visor al siguiente, NULL si no existe.
    \param S Pila sobre la cual se itera.
    \return Elemento al cual esta apuntando el visor.
    \warning Esto se usa SOLAMENTE para iterar en el stack, si se elimina el elemento devuelto 
      queda un stack inconsistente.*/
void * stack_nextItem(Stack S);

/**Situal el visor en la parte superior de la pila.
    \param S Pila sobre la cual se reseteara el visor.*/
void stack_resetViewer(Stack S);

/**Verifica que la pila este vacia.
    \param S Pila sobre la cual se hará la verificación.
    \return True si S esta vacia.\n
    False caso contrario.*/
int stack_isEmpty(Stack S);


void *stack_top(Stack S);

/**Muestra el elemento superior de la pila. 
    \param S Pila de la cual se va a mostrar el elemento superior.
    \return El elemento correspondiente a la cabeza de la pila .*/
int stack_size(Stack S);
#endif