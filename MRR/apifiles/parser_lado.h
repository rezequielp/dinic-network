/*
 * PARSER DE LADOS (EDGES)
 * El path es un archivo con 3 int separados por un espacio entre ellos
 * y luego un '\n', esta estructura puede estar repetida
 * varias veces. al final hay un EOF.
 * 
 * s::= {'+int+' '+int+' '+int+'\n'}+EOF
 * 
 * Ej: 2 4 6\n2 4 54\n321321 321321 4888\nEOF
*/

#ifndef _PARSER_LADO_H
#define _PARSER_LADO_H

#include "auxlibs/lexer/lexer.h"
#include "lado.h"

#define PARSER_OK 0
#define PARSER_ERR 1

/* Lee todo un Lado de `input' hasta llegar a un fin de línea o de archivo
 * Pre:
 *  input != NULL
 *  ! lexer_is_off (input)
 * Ret:
 *  puntero a un nuevo Lado (a liberar por el llamador), NULL en caso de error.
 * ENSURES:
 *  Se consumió input hasta el primer error o hasta completar el Lado.
 *  No se consumió ningun \n.
 */
Lado *parse_lado(Lexer *input);


/* Consume el fin de línea. Indica si encontro basura antes del fin de línea.
 * Pre:
 *     input != NULL
 * Ret:
 *   true en caso de éxito (se consumió hasta el fin de línea o archivo
 *         sin encontrar nada)
 *   false si encuentra algo, pero igual los consume.
 * ENSURES:
 *     no se consumió más entrada de la necesaria
 *     el lexer esta detenido justo luego de un \n o en el fin de archivo.
 */
int parse_nextLine (Lexer *input);

#endif