#include <assert.h>
#include <stdlib.h> 
#include "_stack.h"
#include <stdio.h>

/** \file _stack.c
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


/** Estructura que encapsula un elemento que se apila. 
 * Contiene un elemento Alpha y un puntero a la siguiente estructura. 
 * El tipo pila esta formado por un "apilamiento" de esta estructura.*/
typedef struct SElemSt{
    void * elem;            /**<Puntero al elemento de esta estructura.*/
    struct SElemSt *next; /**<Puntero a la siguiente estructura.*/
}SElem;

/** Estructura principal de una pila.
 * \warning Sobre una pila no se pueden correr dos iteraciones simultaneas ya 
 * que cada una moveria el visor 'iter' accediendo a los elementos 
 * alternadamente. Esta caracteristica es exclusiva. En futuros upgrades se 
 * evitara que se puedan correr diferentes iteraciones o se haran visores 
 * multilples para que si se pueda.
 */   
struct StackSt{
    SElem *top;   /**<Puntero al elemento superior de la pila.*/
    SElem *iter;  /**<Puntero al elemento de la iteracion. Visor.*/
    int size;       /**<Cantidad de elementos de la pila.*/
};


/** Crea una pila vacia.
 * \return Un puntero a la pila creada.
*/
Stack stack_create (void){
    Stack S = NULL;     /*La nueva pilas*/
    
    S = (Stack) malloc(sizeof(struct StackSt));
    if (S != NULL){
        S->top = NULL;
        S->iter = NULL;
        S->size = 0;
    }
    return S;
}

/** Destruye y libera la memoria de la pila S preservando los elementos. 
 * Esta funcion no elimina los elementos que se agregaron en la pila. Es tarea 
 * del llamador destruirlos si se consideraba necesario. Esto es asi porque 
 * no se sabe cual es el tipo de las cosas apiladas y porque no es una tarea 
 * de stack_destroy.
 * \param S La pila a destruir.
 * \param garbage  Puntero a un arreglo de punteros de elementos que se 
 * agregaron a la pila. Si es NULL, stack_destroy no guardara los elementos en 
 * ningun lado. El llamador debe tener cuidado de no perder la referencia a esta
 * memoria para no generar memory leaks.
 * \pre La pila S no es nula.
 * \return Cantidad de elementos que se eliminaron efectivamente de la pila.
 */
int stack_destroy(Stack S, void ** garbage){
    int result = -1;    /*Retorno de la operacion*/
    int i = 0;          /*Contador de elementos que despilo*/
    int sSize;          /*Tamaño de la pila*/
    void *elem;         /*El elemento que se despila*/
    
    assert(S != NULL);
    
    sSize = stack_size(S);
    /*Hasta vaciar la pila, despilo los elementos*/
    while(!stack_isEmpty(S)){
        /*Si quiere referencia a los elementos, se los dejo en garbage*/
        elem = stack_pop(S);
        if(garbage != NULL){
            garbage[i] = elem;
        }
        i++;
    }
    /*Si despile todos los elementos, ya puedo destruir la pila*/
    if(sSize == i){
        result = i;
        free(S);
        S = NULL;
    }
    return result;
}

/** Agrega un elemento del tipo Alpha a la parte superior de la pila.
 * \param S Pila a la cual hay que agregarle el elemento.
 * \param elem Elemento a ser agregado a la pila.
 * \pre La pila S no es nula.
 * \return  1 si se agrego.\n 
 *          0 caso contrario.
 */
int stack_push(Stack S, void *elem){
    SElem *new = NULL;    /*El nuevo elemento a apilar*/
    int result = 0;         /*1 = esta todo OK*/
    
    new = (SElem *) malloc(sizeof(struct SElemSt));
    if(new != NULL){
        new->elem = elem;
        new->next = S->top;
        S->top = new;
        S->size ++;
        result = 1;
    }
    return result;
}

/** Quita el elemento superior de la pila. 
 * \param S Pila a la cual se le va a quitar el elemento superior.
 * \pre La pila S no debe estar vacia ni ser nula.
 * \return Elemento que se le quita a la pila.
 */
void *stack_pop(Stack S){
    SElem *aux = NULL;    /*Puntero auxiliar para no perder referencias*/
    void *elem = NULL;      /*El elemento de la estructura apilada*/
    
    assert(S != NULL && !stack_isEmpty(S));
    
    aux = S->top;
    if (aux->next != NULL)
        S->top = S->top->next;
    else
        S->top = NULL;
    elem = aux->elem;
    free(aux);
    S->size --;
    return elem;
}

/** Verifica que la pila este vacia.
 * \param S Pila sobre la cual se hará la verificación.
 * \pre La pila S no es nula.
 * \return  True si S esta vacia.\n
            False caso contrario.
*/
int stack_isEmpty(Stack S){
    assert(S != NULL);
    return (S->size == 0);
}

/** Muestra el elemento superior de la pila.
 * \param S Pila de la cual se va a mostrar el elemento superior.
 * \pre La pila S no es nula.
 * \return El elemento correspondiente a la cabeza de la pila.
 */
void *stack_top(Stack S){
    void * result = NULL;   /*El elemento top que retorno*/
    assert( S != NULL);
    if (S->top != NULL)
        result = S->top->elem;
    return result;
}

/** Devuelve la cantidad de elementos de la pila.
 * \param S Pila en la cual se contabilizaran los elementos.
 * \pre La pila S no es nula.
 * \return Cantidad de elementos de la pila.
 */
int stack_size(Stack S){
    assert( S != NULL);
    return (S->size);
}

/** Devuelve en S2 la pila S1 revertida.
 * Funcion que invierte el orden de la pila utilizando una nueva pila. El 
 * llamador debe proporcionar las pilas y se encarga de liberarlas.
 * \warning Es altamente ineficiente en cuanto a la memoria que ocupa.
 * \pre Las pilas no son nulas y S2 esta vacia.
 * \return  1 si no hubo error.\n
 *          0 caso contrario.
 */
int stack_revert(Stack S1, Stack S2 ){
    SElem *aux = NULL;    /*Puntero auxiliar para no perder referencias*/
    int result = 0;         /*Resultado del revertido*/
    
    assert((S1 != NULL) && (S2 != NULL));
    assert(stack_isEmpty(S2));

    aux = stack_top(S1);
    do{ /*copio los elementos de una pila a otra hasta el final de S1*/
        result = stack_push(S2, aux);
        aux = aux->next;
    }while((stack_size(S1) != stack_size(S2)) && (result != 0));
    
    return result;
}

/** Muestra un elemento y corre el visor al siguiente, NULL si no existe.
 * \param S Pila sobre la cual se itera.
 * \pre La pila S no es nula.
 * \return Elemento al cual esta apuntando el visor.
 * \warning Esto se usa SOLAMENTE para iterar en el stack, si se elimina el 
 * elemento devuelto queda un stack inconsistente.*/
void* stack_nextItem(Stack S){
    void * elem = NULL;     /*El elemento a mostrar por el visor*/
    assert(S!=NULL);
    
    if(S->iter != NULL){
        elem = S->iter->elem;
        S->iter = S->iter->next;
    }
    return elem;
}

/** Situa el visor en la parte superior de la pila.
 * \param S Pila sobre la cual se reseteara el visor.
 * \pre La pila S no es nula.
 */
void stack_resetViewer(Stack S){
    assert(S!=NULL);
    S->iter = S->top;
}

