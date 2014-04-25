#include "abb.h"
#include "assert.h"
#include <stdlib.h>
#include <stdio.h>
#include "Stack.h"
#include <math.h>
#include <stdbool.h>

#define Leaf ((pandorasBox) NULL)

static void impr(pandoraBox t, char *k);
static void balance(Abb tree, Stack s);
static int factor_equil(pandoraBox pBox);
static void high_update(pandoraBox pBox);
static pandoraBox s_rotate(pandoraBox pBox, bool rgt);
static pandoraBox d_rotate(pandoraBox pBox, bool rgt);
static int abb_toArray(Abb tree, pandoraBox *array);
static bool key_equal(u32 x1, u32 x2);
static bool key_less(u32 x1, u32 x2);


struct AbbSt {
	int size;
    pandorasBox cache;
	pandorasBox top;
};

typedef struct pandorasBoxSt{
	u32 key; 
	void *data;
	pandorasBox left;
	pandorasBox right;
    int high;
} *pandorasBox;


Abb abb_create (void){
	Abb tree;
	tree = (Abb) malloc(sizeof(struct sAbb));
	if (tree != NULL){
        tree->size = 0;
        tree->cache = NULL;
        tree->top = Leaf;
    }
	return tree;
}

int abb_destroy(Abb tree, void *garbage){
    pandoraBox pBox;
    pandoraBox *array;
    result = -1;
    int aSize;
    int i;
    void * data;
    
    assert(tree != NULL);
    
    abbSize = abb_size(tree);
    array = (pandoraBox) malloc(abbSize * sizeof(pandoraBox));
    arrSize = abb_toArray(tree, array);
    if (abbSize == arrSize){
        while(i < arrSize){
            data =array[i]->data;
            if(garbage != NULL){
                garbage[i] = data;
            }
            free(array[i]);
            array[i] = NULL;
            i++;
        }
        if(i == abbSize){
            result = i;
            free(tree);
            tree = NULL;
        }
    }
    free(array);
    array = NULL;
    return result;
}

void abb_add (Abb tree, u32 x, void *data){
	pandorasBox *pBox;
	Stack s;
	
    assert(tree != NULL);
	assert(data != NULL);
	
    pBox = &(tree->top);
	s = stack_create();
	if((*pBox) != Leaf){
		while((*pBox) != Leaf){
			stack_push(s, *pBox);
			if(key_less(x, (*pBox)->key))
				pBox = &((*pBox)->left);
			else
				pBox = &((*pBox)->right);
		}
	}
	*pBox = (pandorasBox) malloc(sizeof(struct pandorasBoxSt));
	if ((*pBox) != NULL){
        (*pBox)->key = x;
        (*pBox)->data = data;
        (*pBox)->left = Leaf; 
        (*pBox)->right = Leaf;
        (*pBox)->high = 0;
    }
	tree->cache = *pBox;
	tree->size ++;
	balance(tree, s);
	stack_destroy(s, NULL);
}

bool abb_exists (Abb tree, u32 x){
	pandorasBox pBox;
	
    assert(tree != NULL);
    if (tree->cache->key == x){
        data = tree->cache->data;
    }else{
        pBox = tree->top;
        while ((pBox != Leaf && !key_equal(x, pBox->key))){	
            if (key_less(x, pBox->key)){
                pBox = pBox->left;
            }else{
                pBox = pBox->right;
            }
        }
    }
	tree->cache = pBox;
	return (pBox != Leaf);
}

/* busca un dato segun la clave si no lo encuentra devuelve NULL */
void *abb_search (Abb tree, u32 x){
	void *data = NULL;
	pandorasBox pBox;
	
    assert(tree != NULL);
	
    if (tree->cache->key == x){
        data = tree->cache->data;
    }else{
        pBox = tree->top;
        while(pBox != Leaf && !key_equal(pBox->key, x)){
            if(key_less(x, pBox->key))
                pBox = pBox->left;
            else
                pBox = pBox->right;
        }
        if(pBox != Leaf){
            data = pBox->data;
        }
    }
	return data;
}

/* devuelve la cantidad de elementos */
int abb_size (Abb tree){
	assert(tree!=NULL);
	return (tree->size);
}

/* VER PARA LAS IMPRESIONES*/
/* imprime todos los pares (u32, void *) en el arbol */
void abb_pprint (Abb tree){
	pandoraBox pBox;
	int i;
	char *x;
	u32 cache; /*ultimo elemento impreso*/
	Stack s;
	assert (tree!=NULL);
	s=stack_create();	
	pBox=tree->top;
	i=tree->size;
	x = (char *) malloc (key_maxLen() * sizeof(char));
	if (x==NULL)
     		err(EXIT_FAILURE, ERRFMT, "abb_pprint",__LINE__);
	if (pBox!=Leaf){ /*si el arbol no es vacio*/
		while(pBox->left!=Leaf){ /*posiciono pBox en el min del arbol*/
			stack_push(s, pBox);
			pBox=pBox->left;
		}
		impr(pBox, x);
		cache=pBox->key;
		i--;
	}
	while(i>0){ /* i es la cantidad de elem por imprimir*/
		if(pBox->left!=Leaf){	/*si cache>=pBox->left => imprimo pBox y me muevo a pBox->right ó al sig sub/meta arbol. Es decir, si ya agregue el elem mas grande de la parte izq de pBox, entonces subo a un nodo padre ó bajo hacia el hijo derecho*/ 
			if (!key_less(cache, pBox->left->key)){/*si esto pasa quiere decir que pBox no fue impreso*/
				impr(pBox, x);
				cache=pBox->key;
				i--;
				if(pBox->right!=Leaf)
					pBox=pBox->right;
				else{
					if(i>0){
						assert(!stack_isEmpty(s));
						pBox=stack_top(s);
						stack_pop(s);
					}		
				}
			}
			else {
				stack_push(s, pBox);
				pBox = pBox->left;
			}
		}					
		else if(pBox->left==Leaf){ /*pBox es menor. Luego, si chache<pBox => imprimo pBox.*/
			if (key_less(cache, pBox->key)){
				impr(pBox, x);
				cache=pBox->key;
				i--;
			}
			if((pBox->right==Leaf || !key_less(cache, pBox->right->key)) && i>0){
				assert(!stack_isEmpty(s));
				pBox=stack_top(s);
				stack_pop(s);		
			}
			else{
				pBox=pBox->right;			
			}
		}
	}
	free(x);
	s = stack_destroy(s);
}

/* Destructor. Destruye el abb y retorna una pila con los datas su contenido */
/* PRE: tree != NULL y arreglos del mismo tamaño (nahuel edita)*/



/***********************************************FUNCIONES LOCALES*****************************************************/	

/*VER PARA IMPRESIONES*/
void impr(pandoraBox pBox, char *x){
	char *da;
	assert(pBox!=NULL);
	assert(x!=NULL);	
	key_toString(pBox->key, x);
	da = data_toString (pBox->data);
	printf("%s:%s\n", x, da);
	/*printf("           %i\n",pBox->high);*/ /*PREGUNTARRRRRRRRRRRRRRRRR*/
	free(da);
}


void balance (Abb tree, Stack s){
	pandoraBox pBox; 
	pandoraBox q;
	int fe; /*factor de equilibrio*/
    
	assert(tree != NULL);
	assert(s != NULL);
	q = tree->top;
	while(!stack_isEmpty(s)){		
		pBox = stack_top(s);	
		stack_pop(s);
		if(!stack_isEmpty(s)){
			q = stack_top(s);
        }
		high_update(pBox);
		fe = factor_equil(pBox);
		if(fe == 2){
			if(factor_equil(pBox->right) == 1){
				if(!stack_isEmpty(s)){
					if (key_less(q->key, pBox->key))
						q->right = s_rotate(pBox, false);
					else						
						q->left = s_rotate(pBox, false);
				}
				else
					tree->top = s_rotate(pBox, false);
			}
			else{
				if(!stack_isEmpty(s)){
					if (key_less(q->key, pBox->key))
						q->right = d_rotate(pBox, false);
					else
						q->left = d_rotate(pBox, false);
				}
				else
					tree->top = d_rotate(pBox, false);
			}
		}
		if(fe == -2){
			if(factor_equil(pBox->left) == -1){
				if(!stack_isEmpty(s)){
					if (key_less(q->key, pBox->key))
						q->right = s_rotate(pBox, true);
					else
						q->left = s_rotate(pBox, true);
				}
				else
					tree->top = s_rotate(pBox, true);
			}
			else{
				if(!stack_isEmpty(s)){
					if (key_less(q->key, pBox->key))
						q->right=d_rotate(pBox, true);
					else
						q->left = d_rotate(pBox, true);
				}
				else
					tree->top = d_rotate(pBox, true);
			}
		}
	}
}

int factor_equil(pandoraBox pBox){
	int i = 0;
	assert(pBox != NULL);
	 /*si algun hijo es Leaf, entonces se concidera la altura del Leaf=(-1)*/
	/*FE=H(pBox->data)-H(pBox->left)*/
		if (pBox->right != Leaf){
			i = pBox->right->high;
        }else{
			i = -1;
        }
		if (pBox->left != Leaf){
			i = i - pBox->left->high;
        }else{
			i = i - (-1);
        }
	return i;
}

/*soluciona un desequilibro izq-izq o der-der*/
pandoraBox s_rotate(pandoraBox pBox, bool rgt){
	
	pandoraBox q;
	assert(pBox != NULL);
	if(rgt){/*rotacion de un nodo de la izq del padre, hacia la derecha del arbol*/
		q = pBox->left;
		pBox->left = q->right;
		q->right=pBox;
	}
	else{/*rota hacia la izq del arbol*/
		q = pBox->right;
		pBox->right = q->left;
		q->left = pBox;
	}
	high_update(pBox);
	high_update(q);
	return q;
}

/*soluciona un desequilibrio izq-der o der-izq*/
pandoraBox d_rotate(pandoraBox pBox, bool rgt){
	assert(pBox != NULL);	
	if(rgt){
        /*lleva el desequilibrio izq-der a uno de la forma izq-izq*/
		pBox->left = s_rotate(pBox->left, false);
        /*soluciona un desequilibro izq-izq*/
		pBox = s_rotate(pBox, true);
	}
	else{
        /*lleva el desequilibrio der-izq a uno de la forma der-der*/
		pBox->right = s_rotate(pBox->right, true);
        /*soluciona un desequilibro der-der*/
		pBox = s_rotate(pBox, false);
	}
	return pBox;
}

/*PRE: El array debe tener la misma cantidad de elementos
 * que la cantidad de nodos que tiene el arbol*/
int abb_toArray(Abb tree, pandoraBox *array){
    int abbSize;
    int arrSize;
    int i = 0;
    int result = -1;
    pandoraBox *array;
    assert(tree != NULL);
    assert(array != NULL);
    abbSize = abb_size(tree);
    
    array[0] = pBox;
    arrSize = 1;
    while(arrSize < abbSize && i < aSize){
        if (array[i]->left != Leaf){
            array[arrSize] = array[i]->left;
            arrSize++;
        }
        if (array[i]->right != Leaf){
            array[arrSize] = array[i]->right;
            arrSize++;
        }
        i++;
    }
    if (arrSize == abbSize){
        result = arrSize;
    }
    return result;
}

void high_update(pandoraBox pBox){
	assert(pBox != NULL);
	if(pBox->right != Leaf && pBox->left != Leaf){
		pBox->high = 1 + fmax(pBox->left->high, pBox->right->high);
    }else{
		if (pBox->right == Leaf){
			if(pBox->left == Leaf){
				pBox->high = 0;
            }else{
				pBox->high = 1 + pBox->left->high;
            }
		}
		else
			pBox->high = 1 + pBox->right->high;
	}
}

bool key_equal(u32 x1, u32 x2){
    return (x1 == x2);
}

bool key_less(u32 x1, u32 x2){
    return (x1 < x2);
}

