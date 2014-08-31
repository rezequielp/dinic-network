#include "parser_lado.h"
#include "u64.h"
#include <assert.h>

#define EOL "\n"            /**<Para indicador de final de linea.*/
#define WHITE_SPACE " "     /**<Para indicador de espacio en blanco.*/

/*flags para next_bstring*/
#define ACCEPT 1   /**<Solo se aceptan caracteres contenidos en \a str. */
#define EXCEPT 0   /**<Se acepta cualquier caracter que no este contenido en \a str.*/

/*                      Funciones del modulo
 */
static int parse_argument(Lexer *input, u64 *arg);
static bstring next_bstring(Lexer *input, int flag, const char *str);
static bool is_theNextChar(Lexer *input, const char *ch);

/** Lee todo un Lado de \a input hasta llegar a un fin de línea o de archivo.
 *  Se asegura que se consumió input hasta el primer error o hasta completar 
 *  el lado y No se consumió ningún "\n".
 * \param input El analizador léxico del descriptor de entrada
 * \pre input!=NULL.
 * \return Un lado bien formado si el parseo fue exitoso. \n
 *         LadoNulo caso contrario.
 */
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


/** Consume el fin de línea.
 * Indica si encontro (o no) basura antes del fin de línea.
 * Se asegura que no se consumió más entrada de la necesaria y el lexer esta 
 * detenido justo luego de un "\n" o en el fin de archivo.
 * \param input El analizador léxico del descriptor de entrada.
 * \pre input!=NULL.
 * \return  PARSER_OK si no hay basura. \n
 *          PARSER_ERR caso contrario.
 */
int parser_nextLine(Lexer *input){
    int result = PARSER_OK;     /*Si es EOF (o el sig char es '\n'), sera true*/
    bstring gbCollector = NULL; /*Recolector de caracteres basura*/
    
    assert(input != NULL);
    /*Quito los espacio en blanco que pueden haber al comienzo*/
    if (!lexer_is_off(input))
        lexer_skip(input, WHITE_SPACE);
    /*Consumo toda la basura anterior al primer '\n' (o EOF)*/
    gbCollector = next_bstring(input, EXCEPT, EOL);
    /*Si leyo algo, entonces gbCollector no es nulo*/
    if (gbCollector != NULL){
        result = PARSER_ERR;
        bdestroy(gbCollector);
    }else if (is_theNextChar(input, EOL) || lexer_is_off(input))
        result = PARSER_OK;
        
    return result;
}


/*INTERNAS*/


/** Parsea un argumento de los ingresados.
 * Los caracteres que se lean deben ser digitos para que el argumento sea válido.
 * \param input El analizador léxico del descriptor de entrada.
 * \param n     El argumento parseado resultante.
 * \pre input!=NULL.
 * \return  Si no hubo error, asigna el argumento parseado en \a n y retorna PARSER_OK \n
 *          Si hubo error, no asigna nada a \a n y retorna PARSER_ERR
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
    barg = next_bstring(input, ACCEPT, DIGIT);
    if (barg != NULL){
        /*Lo convierto a u64*/
        carg = bstr2cstr(barg, '\0');
        sr = sscanf(carg, "%"SCNu64"\n", n);
        if (sr > 0)
            result = PARSER_OK;
        /*Libero memoria*/
        bcstrfree(carg);
        bdestroy(barg);
    }
    
    return result;
}

/** Lee el siguiente item.
 * Si \a flag=ACCEPT: Consume hasta encontrarse con un caracter distinto de \a str.
 * Si \a flag=EXCEPT: Consume hasta encontrarse con un caracter de \a str.
 * \param input El analizador léxico del descriptor de entrada.
 * \param flag  ACCEPT|EXCEPT respecto al contenido de \a str.
 * \param str   Los caracteres con los que se compara cada caracter leído.
 * \pre input!=NULL y flag debe ser una opcion valida.
 * \return  Un \a bstring del item leído. El llamador se encarga de liberarlo.\n
 *          NULL caso contrario.\n
 */
static bstring next_bstring(Lexer *input, int flag, const char *str){
    bstring result = NULL;        /*Los caracteres que se leyeron*/
    
    assert (input != NULL);
    assert(flag == ACCEPT || flag == EXCEPT);
    
    if (!lexer_is_off(input)){
        /*Leo todos los caracteres anteriores ACCEPT/EXCEPT 'str'*/
        if (flag == ACCEPT)
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


/** Decide si el siguiente caracter leido es \a ch.
 * Consume el caracter leido.
 * \param input El analizador léxico del descriptor de entrada.
 * \param ch    El caracter con el que se quiere comparar.
 * \pre input!=NULL.
 * \return  True si el caracter leido es \a ch. \n
 *          False caso contrario.
 */
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
