#ifndef _PARSER_LADO_H
#define _PARSER_LADO_H
/** \file parser_lado.h
 * Parseador de lados desde la entrada Estandar Input.
 * El path es un archivo con 3 int separados por un espacio entre ellos
 * y luego un "\n". Esta estructura puede estar repetida varias veces. 
 * Al final hay un EOF.
 * \verbatim 
    s::= {'+int+' '+int+' '+int+'\n'}+EOF
    
    Ejemplo: 2 4 6\n2 4 54\n321321 321321 4888\nEOF
   \endverbatim
*/
#include "_lexer.h"
#include "lado.h"

/* Indicadores de resultados de las operaciones realizadas con el parser*/
#define PARSER_OK 1     /**< Operación exitosa.*/
#define PARSER_ERR 0    /**< Operación erronea.*/


/* Lee todo un Lado de \a input hasta llegar a un fin de línea o de archivo.
 *  Se asegura que se consumió input hasta el primer error o hasta completar 
 *  el lado y no se consumió ningún "\n".
 * input El analizador léxico del descriptor de entrada.
 * pre: input!=NULL.
 * return: Un lado bien formado si el parseo fue exitoso.
 *         LadoNulo caso contrario.
 */
Lado parser_lado(Lexer *input);

/* Consume el fin de línea.
 * Indica si encontro (o no) basura antes del fin de línea.
 * Se asegura que no se consumió más entrada de la necesaria y el lexer esta 
 * detenido justo luego de un "\n" o en el fin de archivo.
 * input El analizador léxico del descriptor de entrada.
 * pre: input!=NULL.
 * return:  PARSER_OK si no hay basura.
 *          PARSER_ERR caso contrario.
 */
int parser_nextLine (Lexer *input);

#endif