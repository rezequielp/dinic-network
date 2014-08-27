#include <assert.h>
#include <stdlib.h> 
#include "_stack.h"
#include <stdio.h>

/** \file _stack.c
La libreria _stack proporciona una serie de herramientas para almazenar cualquier tipo de elemento
en el orden LIFO(Last In, First Out) ultimo en entrar primero en salir, correspondiente a lo que 
llamamos cotidianamente como una pila. La particularidad de esta pila es que no importa el tipo 
de elemento que ordene ni se sabe que elemento se ordena. Por este motivo no se puede 
destruir en el caso de querer sacar el elemento superior o querer liberar la memoria de toda la estructura.
Por lo cual, estas dos funciones devuelven el elemento o un arreglo de los elementos que se desean eliminar 
para que el llamador los elimine si lo considera pertinente.
*/


static int stack_revert (Stack S1, Stack S2 );

/** Estructura que contiene un elemento Alpha y un puntero a la siguiente estructura. 
 El tipo pila esta formado por un "apilamiento" de esta estructura.*/
typedef struct ElementSt{
    void * elem;/**<Puntero al emento de esta estructura. Al ser del tipo void pointer puede apuntar a cualquier cosa.*/
    struct ElementSt *next;/**<Puntero a la siguiente estructura del tipo ElementSt.*/
} Element;

/**Estructura principal de la pila. 
    \warning Sobre una pila no se pueden correr dos iteraciones simultaneas ya que cada una moveria el visor iter 
     accediendo a los elementos alternadamente. Esta caracteristica es exclusiva. En futuros upgrades se evitara que
     se puedan correr diferentes iteraciones o se haran visores multilples para que si se pueda.*/
struct StackSt{
    Element *top;/**<Puntero al elemento superior de la pila.*/
    Element *iter;/**<Puntero al elemento de la iteracion. Tambien llamado visor.*/
    int size; /**<Cantidad de elementos de la pila.*/
};

/**Crea una pila vacia.
    \return Un puntero a la pila creada.*/
Stack stack_create (void){
	Stack S = NULL;
    S = (Stack) malloc(sizeof(struct StackSt));
    if (S != NULL){
        S->top = NULL;
        S->iter = NULL;
        S->size = 0;
    }
	return S;
}

/**Destruye y libera la memoria de la pila S preservando los elementos. Esta funcion no elimina los elementos 
que se agregaron en la pila. Es tarea del llamador destruirlos si se consideraba necesario. 
Esto es asi porque no se sabe cual es el tipo de las cosas appiladas y porque no es la 
funcion de stack_destroy.
    \param S La pila a destruir.
    \param garbage  Puntero a un arreglo de punteros de elementos que se agregaron a la pila. Si se pasa 
    NULL, stack_destroy no guardara los elementos en ningun lado. El llamador debe tener cuidado de no perder
    la referencia a esta memoria para no generar memory leaks.
    \return Cantidad de elementos (del tipo ElementSt) que se eliminaron efectivamente de la pila.*/
int stack_destroy(Stack S, void ** garbage){
    int result = -1;
    int i = 0;
    int sSize;
    
    assert(S != NULL);
    
    sSize = stack_size(S);
    while(!stack_isEmpty(S)){
        if(garbage != NULL){
            garbage[i] = stack_top(S);
        }
        stack_pop(S);
        i ++;
    }
    if(sSize == i){
        result = i;
        free(S);
        S = NULL;
    }
    return result;
}

/**Agrega un elemento del tipo Alpha a la parte superior de la pila.
    \param s Pila a la cual hay que agregarle el elemento.
    \param elem Elemento a ser agregado a la pila.
    \return  0 si se agrego.\n 1 caso contrario.*/
int stack_push(Stack S, void *elem){
	Element *new = NULL;
    int result = 1; /*0 = esta todo OK*/
    
	new = (Element *) malloc(sizeof(struct ElementSt));
	if(new != NULL){
        new->elem = elem;
        new->next = S->top;
        S->top = new;
        S->size += 1;
        result = 0;
    }
	return result;
}

/**Quita el elemento superior de la pila. Como precondicion la pila no
debe estar vacia ni ser nula.
    \param S Pila a la cual se le va a quitar el elemento superior.
    \return Elemento que se le quita a la pila.*/
void * stack_pop(Stack S){
	Element *aux = NULL;
    void *elem = NULL;
    assert(S != NULL && !stack_isEmpty(S));
    
	aux = S->top;
    S->top = S->top->next;
    elem = aux->elem;
    free(aux);
    S->size --; 
	return elem;
}

/**Verifica que la pila este vacia.
    \param S Pila sobre la cual se hará la verificación.
    \return True si S esta vacia.\n
    False caso contrario.*/
int stack_isEmpty(Stack S){
    assert(S != NULL);
	return (S->size == 0);
}

/**Muestra el elemento superior de la pila. 
    \param S Pila de la cual se va a mostrar el elemento superior.
    \return El elemento correspondiente a la cabeza de la pila .*/
void *stack_top(Stack S){
    void * result = NULL;
    assert( S != NULL);
    if (S->top != NULL)
        result = S->top->elem;
    return result;
}

/**Devuelve la cantidad de elementos de la pila.
        \param S Pila en la cual se contabilizaran los elementos.
        \return Cantidad de elementos de la pila.*/
int stack_size(Stack S){
    assert( S != NULL);
    return (S->size);
}

/**Devuelve en S2 la pila S1 revertida
Fuincion que invierte el orden de la pila utilizando una nueva pila.
    \warning Es altamente ineficiente en cuanto a la memoria que ocupa.
    Por lo pronto se encuentra deshabilitada.*/
int stack_revert(Stack S1, Stack S2 ){
    Element *aux = NULL;
    int result = 1;
    
    assert((S1 != NULL) && (S2 != NULL));
    assert(stack_isEmpty(S2));

    aux = stack_top(S1);
    while ((stack_size(S1) != stack_size(S2)) && (result != 0)){
        result = stack_push(S2, aux);
        aux = aux->next;
    }
    return result;
}


/**muestra un elemento y corre el visor al siguiente, NULL si no existe.
    \param S Pila sobre la cual se itera.
    \return Elemento al cual esta apuntando el visor.*/
void* stack_nextItem(Stack S){
    void * elem = NULL;
    assert(S!=NULL);
    
    if (S->iter != NULL){
        elem = S->iter->elem;
        S->iter = S->iter->next;
    }
    return elem;
}


/**Situal el visor en la parte superior de la pila.
    \param S Pila sobre la cual se reseteara el visor.*/
void stack_resetViewer(Stack S){
    assert(S!=NULL);
    S->iter = S->top;
}

