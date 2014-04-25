#include "parser_edge.h"

#define END_LINE "\n" /*para indicador de final de linea*/
#define WHITE_SPACE " " /*para indicador de espacio en blanco*/

static int parse_argument (Lexer *input, u32 arg);
static bstring next_bstring (Lexer *input, const char *str);
static bool is_the_next_char (Lexer *input, const char *ch)


/* Lee todo un edge de `input' hasta llegar a un fin de línea o de archivo*/
edge *parse_edge(Lexer *input){
	edge *result = NULL;
	u32 x = 0; /*nodo x*/
	u32 y = 0; /*nodo y*/
	u32 cap = 0; /*capacidad de xy*/
	
	/*Pre:*/
	assert(input != NULL);
	assert(!lexer_is_off(input));

	/*No se debe encontrar al comienzo ningun caracter distinto a 'DIGIT'*/
	if (!is_the_next_char(input, ALPHA BLANK)){
		/*asigno el 1er argumento parseado a 'x'*/
		its_ok = parse_argument(input, x);
		if (its_ok && is_the_next_char(input, WHITE_SPACE)){
			/*asigno el 2do argumento parseado a 'y'*/
			its_ok = parse_argument(input, y);
			if (its_ok && is_the_next_char(input, WHITE_SPACE)){
				/*asigno el 3er argumento parseado a 'cap'*/
				its_ok = parse_argument(input, cap);
			}
		}
		/* Si se parseo todo bien, creo el nuevo edge con los valores*/
		if (it_ok){
			result = edge_new(x, y, cap);
		}
	}
	
	return result;
}


/* Consume el fin de línea.
 * Indica si encontro basura antes del fin de línea*/
int parse_next_line (Lexer *input){
	int result = PARSER_OK; /*si es EOF (o el sig char es '\n'), sera true*/
	bstring taken = NULL; /*caracteres leidos*/
	
	/*Pre:*/
	assert(input != NULL);

	/*consumo toda la basura anterior al primer '\n' (o EOF)*/
	taken = next_bstring (input, END_LINE);
	/*si leyo algo, entonces taken no es nulo*/
	if (taken != NULL){
		result = PARSER_ERR;
		bdestroy (taken);

	return result;
}



/*INTERNAS*/


/*Parsea un argumento de los ingresados.
 * Si no hubo error, asigna el argumento parseado en 'arg' y retorna PARSER_OK
 * Si hubo error, no asigna nada a 'arg' y retorna PARSER_ERR
 * El llamador se encarga de liberarlo.
*/
static int parse_argument (Lexer *input, u32 arg){
	int result = PARSER_ERR; /*retorno (error al menos que todo salga bien)*/
	bstring barg = NULL; /*argumento leido en tipo bstring*/
	char * sarg = NULL; /*argumento leido convertido a string*/
	
	assert(input != NULL);
	
	if (!lexer_is_off (input)){
		/*leo hasta el siguiente caracter distinto de 'DIGIT'*/
		barg = next_bstring (input, ALPHA BLANK);
		if (barg != NULL){
			/* lo convierto a string*/
			sarg = bstr2cstr (barg, '\0');
			if (sarg != NULL){
				/* lo convierto a u32*/
				arg = (u32) atoi(sarg);
				result = PARSER_OK;
				/*destruyo sarg*/
				bcstrfree (sarg);
			}
			/*destruyo barg*/
			bdestroy (barg);
		}
		
	}
	
	return result,
}

/*lee el siguiente item.
 * Consume todo caracter hasta encontrar alguno perteneciente a 'str'.
 *Deja el resultado accesible en 'result'. El llamador se encarga de liberarlo*/
static bstring next_bstring (Lexer *input, const char *str){

	bstring result=NULL;
	/*Pre:*/
	assert (input != NULL);
	/*Leo todos los caracteres anteriores y no pertenecientes a 'str'*/
	if (!lexer_is_off (input)){
		lexer_next_to (input, str);
		/*si (leyo algo) ´o´ (no EOF y no leyo nada)*/
		if (!lexer_is_off (input)){
			result = lexer_item(input);
			/*Si no leyo ningun caracter, destruyo el puntero.
			 * Esto pasa cuando el caracter inmediato a leer pertenece a str
			 * y el item es vacio*/
			if (blength (result) == 0){
				bdestroy (result);
				result=NULL;
			}
		}
	}

	return result;
}


/*Decide si el siguiente caracter leido pertenece a 'Ch'.
 * consume el caracter leido
 *Deja el resultado accesible en 'result'*/
static bool is_the_next_char (Lexer *input, const char *ch){

	bool result = false;
	bstring taken = NULL;

	/*Pre:*/
	assert (input != NULL);

	/*Leo el siguiente caracter para ver si pertenece a 'ch'*/
	if (!lexer_is_off (input)){
		lexer_next_char (input, ch);
		/*si (leyo algo) ´o´ (no EOF y no leyo nada)*/
		if (!lexer_is_off (input)){
			taken = lexer_item (input);
			/*Decide si leyo el caracter perteneciente a 'ch'*/
			if ((blength (taken) > 0)){
				result = true;
			}
			/*libero el puntero despues de usarlo*/
			bdestroy (taken);
		}
	}

	return result;
}