#include "parser_lado.h"
#include "auxlibs/u64/u64.h"

#define EOL "\n"       /*para indicador de final de linea*/
#define WHITE_SPACE " "     /*para indicador de espacio en blanco*/

static int parse_argument(Lexer *input, u64 arg);
static bstring next_bstring(Lexer *input, const char *str);
static bool is_theNextChar(Lexer *input, const char *ch)


/* Lee todo un Lado de `input' hasta llegar a un fin de línea o de archivo*/
Lado *parse_lado(Lexer *input){
    Lado *result = NULL;
    u64 x = NULL;       /*nodo x*/
    u64 y = NULL;       /*nodo y*/
    u64 cap = NULL;     /*capacidad de xy*/
    
    /*Pre:*/
    assert(input != NULL);
    assert(!lexer_is_off(input));
    
    /*No se debe encontrar al comienzo ningun caracter distinto a 'DIGIT'*/
    if (!is_theNextChar(input, ALPHA BLANK)){
        /*asigno el 1er argumento parseado a 'x'*/
        its_ok = parse_argument(input, x);
        if (its_ok && is_theNextChar(input, WHITE_SPACE)){
            /*asigno el 2do argumento parseado a 'y'*/
            its_ok = parse_argument(input, y);
            if (its_ok && is_theNextChar(input, WHITE_SPACE)){
                /*asigno el 3er argumento parseado a 'cap'*/
                its_ok = parse_argument(input, cap);
            }
        }
        /* Si se parseo todo bien, creo el nuevo Lado con los valores*/
        if (it_ok){
            result = lado_new(x, y, cap);
        }
    }
    
    return result;
}


/* Consume el fin de línea.
 * Indica si encontro basura antes del fin de línea*/
int parse_nextLine(Lexer *input){
    int result = PARSER_OK; /*si es EOF (o el sig char es '\n'), sera true*/
    bstring taken = NULL; /*caracteres leidos*/
    
    /*Pre:*/
    assert(input != NULL);

    /*consumo toda la basura anterior al primer '\n' (o EOF)*/
    taken = next_bstring(input, EOL);
    /*si leyo algo, entonces taken no es nulo*/
    if (taken != NULL){
        result = PARSER_ERR;
        bdestroy(taken);
    }
    return result;
}



/*INTERNAS*/


/*Parsea un argumento de los ingresados.
 * Si no hubo error, asigna el argumento parseado en 'barg' y retorna PARSER_OK
 * Si hubo error, no asigna nada a 'barg' y retorna PARSER_ERR
 * El llamador se encarga de liberarlo.
*/
static int parse_argument(Lexer *input, u64 arg){
    int result = PARSER_ERR;    /*retorno (error al menos que todo salga bien)*/
    bstring barg = NULL;        /*argumento leido en tipo bstring*/
    
    assert(input != NULL);
    
    if (!lexer_isOff(input)){
        /*leo hasta el siguiente caracter distinto de 'DIGIT'*/
        barg = next_bstring(input, ALPHA BLANK);
        if (barg != NULL){
            /* lo convierto a u64*/
            u64_fromBstr(barg, arg);
            result = PARSER_OK;
            /*destruyo barg*/
            bdestroy(barg);
        }
    }
    return result,
}

/*lee el siguiente item.
 * Consume todo caracter hasta encontrar alguno perteneciente a 'str'.
 * El llamador se encarga de liberarlo*/
static bstring next_bstring(Lexer *input, const char *str){

    bstring result=NULL;
    assert (input != NULL);
    
    /*Leo todos los caracteres anteriores y no pertenecientes a 'str'*/
    if (!lexer_isOff(input)){
        lexer_nextTo(input, str);
        /*si (leyo algo) ´o´ (no EOF y no leyo nada)*/
        if (!lexer_isOff(input)){
            result = lexer_item(input);
            /*Si no leyo ningun caracter, destruyo el puntero.
             * Esto pasa cuando el caracter inmediato a leer pertenece a str
             * y el item es vacio*/
            if (blength(result) == 0){
                bdestroy(result);
                result = NULL;
            }
        }
    }
    return result;
}


/*Decide si el siguiente caracter leido pertenece a 'ch'.
 * Consume el caracter leido*/
static bool is_theNextChar(Lexer *input, const char *ch){

    bool result = false;
    bstring taken = NULL;

    /*Pre:*/
    assert (input != NULL);

    /*Leo el siguiente caracter para ver si pertenece a 'ch'*/
    if (!lexer_is_off(input)){
        lexer_nextChar(input, ch);
        /*si (leyo algo) ´o´ (no EOF y no leyo nada)*/
        if (!lexer_is_off(input)){
            taken = lexer_item(input);
            /*Decide si leyo el caracter perteneciente a 'ch'*/
            if ((blength(taken) > 0)){
                result = true;
            }
            /*libero el puntero despues de usarlo*/
            bdestroy(taken);
        }
    }
    return result;
}