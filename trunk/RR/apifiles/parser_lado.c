#include "parser_lado.h"
#include "u64.h"
#include <assert.h>

#define EOL "\n"            /*para indicador de final de linea*/
#define WHITE_SPACE " "     /*para indicador de espacio en blanco*/

/*flags para next_bstring*/
#define WITH 1              
#define WITHOUT 0

/*                      Funciones del modulo
 */
static int parse_argument(Lexer *input, u64 *arg);
static bstring next_bstring(Lexer *input, int flag, const char *str);
static bool is_theNextChar(Lexer *input, const char *ch);

/*Lee todo un Lado de `input' hasta llegar a un fin de línea o de archivo*/
Lado parser_lado(Lexer *input){
    u64 x;                      /*Nodo x*/
    u64 y;                      /*Nodo y*/
    u64 cap;                    /*Capacidad de xy*/
    Lado result = LadoNulo;     /*Lado parseado*/
    int its_ok = PARSER_ERR;    /*Chequea que el parse va bien*/   

    assert(input != NULL);
    
    /*Asigno el 1er argumento parseado a 'x'*/
    its_ok = parse_argument(input, &x);
    if (its_ok && is_theNextChar(input, WHITE_SPACE)){
        /*Asigno el 2do argumento parseado a 'y'*/
        its_ok = parse_argument(input, &y);
        if (its_ok && is_theNextChar(input, WHITE_SPACE))
            /*Asigno el 3er argumento parseado a 'cap'*/
            its_ok = parse_argument(input, &cap);   
    }
    /*Si se parseo todo bien, creo el nuevo Lado con los valores*/
    if (its_ok){
        result = lado_new(x, y, cap);
    }
    
    return result;
}


/* Consume el fin de línea.
 * Indica si encontro (o no) basura antes del fin de línea.
 * Retorno: PARSER_OK si no hay basura, PARSER_ERR caso contrario*/
int parser_nextLine(Lexer *input){
    int result = PARSER_OK;     /*Si es EOF (o el sig char es '\n'), sera true*/
    bstring gbCollector = NULL; /*Recolector de caracteres basura*/
    
    assert(input != NULL);
    /*Quito los espacio en blanco que pueden haber al comienzo*/
    if (!lexer_is_off(input))
        lexer_skip(input, WHITE_SPACE);
    /*Consumo toda la basura anterior al primer '\n' (o EOF)*/
    gbCollector = next_bstring(input, WITHOUT, EOL);
    /*Si leyo algo, entonces gbCollector no es nulo*/
    if (gbCollector != NULL){
        result = PARSER_ERR;
        bdestroy(gbCollector);
    }else if (is_theNextChar(input, EOL) || lexer_is_off(input))
        result = PARSER_OK;
        
    return result;
}


/*INTERNAS*/


/*Parsea un argumento de los ingresados.
 * Si no hubo error, asigna el argumento parseado en 'arg' y retorna PARSER_OK
 * Si hubo error, no asigna nada a 'arg' y retorna PARSER_ERR
 * El llamador se encarga de liberarlo.
*/
static int parse_argument(Lexer *input, u64 *n){
    int result = PARSER_ERR;    /*Retorno (error al menos que todo salga bien)*/
    bstring barg = NULL;        /*Argumento leido en tipo bstring*/
    char * carg;                /*Argumento leido en tipo char*/
    int sr = 0;                 /*Resultado del scaneo (sscanf())*/
    
    assert(input != NULL);
    /*Quito los espacio en blanco que pueden haber al comienzo*/
    if (!lexer_is_off(input))
        lexer_skip(input, WHITE_SPACE);
    /*Leo hasta el siguiente caracter distinto de 'DIGIT'*/
    barg = next_bstring(input, WITH, DIGIT);
    if (barg != NULL){
        /*Lo convierto a u64*/
        carg = bstr2cstr(barg, '\0');
        rs = sscanf(carg, "%"SCNu64"\n", n)
        if (rs > 0)
            result = PARSER_OK;
        /*Libero memoria*/
        bcstrfree(carg);
        bdestroy(barg);
    }
    
    return result;
}

/*lee el siguiente item.
 * Si flag = WITH: Consume hasta encontrarse con un caracter distinto de 'str'
 * Si flag = WITHOUT: Consume hasta encontrarse con un caracter de 'str'
 * El llamador se encarga de liberarlo*/
static bstring next_bstring(Lexer *input, int flag, const char *str){
    bstring result = NULL;        /*Los caracteres que se leyeron*/
    
    assert (input != NULL);
    assert(flag == WITH || flag == WITHOUT);
    
    if (!lexer_is_off(input)){
        /*Leo todos los caracteres anteriores WITH/WITHOUT 'str'*/
        if (flag == WITH)
            lexer_next(input, str);
        else
            lexer_next_to(input, str);
    }
    /*Caso: (leyo algo) o (no EOF y no leyo nada)*/
    if (!lexer_is_off(input)){
        result = lexer_item(input);
        /*Si no leyo ningun caracter, destruyo el puntero.
            * Esto pasa cuando el caracter inmediato a leer no pertenece a str
            * y el item es vacio*/
        if (blength(result) == 0){
            bdestroy(result);
            result = NULL;
        }
    }

    return result;
}


/*Decide si el siguiente caracter leido pertenece a 'ch'.
 * Consume el caracter leido*/
static bool is_theNextChar(Lexer *input, const char *ch){
    bool result = false;    /*Resultado*/
    bstring taken = NULL;   /*Caracter obtenido en la lectura*/

    assert (input != NULL);
    
    if (!lexer_is_off(input)){
        /*Leo el siguiente caracter para ver si pertenece a 'ch'*/
        lexer_next_char(input, ch);
        
    /*Caso: (leyo algo) o (no EOF y no leyo nada)*/
    if (!lexer_is_off(input)){
        taken = lexer_item(input);
        if ((blength(taken) > 0)){
            /*Hay un caracter leido y es igual a 'ch'*/
            result = true;
        }
        /*Libero el puntero despues de usarlo*/
        bdestroy(taken);
    }
    }
    return result;
}
