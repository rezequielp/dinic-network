﻿/**Main para probar la API que corre el algoritmo DINIC en networks.*/
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include "../apifiles/API.h"

/*MACROS PARA MANEJAR LOS parámetros DE ENTRADA.*/
/*Flags.*/
#define DONT_DINIC      0b10000000  /**<Permite (o no) que se realice dinic.*/
#define S_OK            0b01000000  /**<'s' fue pasado como parámetro.*/
#define T_OK            0b00100000  /**<'t' fue pasado como parámetro.*/
#define DINIC_TIME      0b00010000  /**<Activa el muestreo de tiempos de calculo.*/
#define PATH            0b00001000  /**<Activa el muestreo de caminos aumentantes.*/
#define FLOW            0b00000100  /**<Activa el muestreo de flujo.*/
#define CUT             0b00000010  /**<Activa el muestreo de corte.*/
#define FLOW_V          0b00000001  /**<Activa el muestreo de valor del flujo.*/
/*Macros para manipular flags.*/
#define SET_FLAG(f) STATUS |= f         /**<Setea un bit de flag en STATUS en 1.*/
#define UNSET_FLAG(f) STATUS &= ~f      /**<Setea un bit de flag en STATUS en 0.*/
#define CLEAR_FLAG() 0b00000000         /**<Pone todas las flags en 0.*/
#define IS_SET_FLAG(f) (STATUS & f) > 0 /**<Consulta si una flag esta activa.*/

/*FUNCIONES ESTATICAS.*/
static void load_from_stdin(DovahkiinP dova);
static void print_help(char * programName);
static short int parametersChecker(int argc, char *argv[], u64 * source, u64 * sink);
static void print_dinicTime(float time);
static bool isu64(char * sU64);

/** Lee desde el standard input los lados del network y los carga en dovahkiin.
 * Se lee hasta acabar los lados o bien hasta el primer lado que no se pueda
 * cargar.
 * \param dova El dova donde se cargaran los lados.
 */
void load_from_stdin(DovahkiinP dova){

    Lado lado = LadoNulo;/*Lado leído. Caso vacío se retorna LadoNulo.*/
    int load_ok = 0;/*Indica si el lado se pudo cargar.*/
    
    assert(dova != NULL);
    do{
        lado = LeerUnLado();
        load_ok = CargarUnLado(dova, lado);
    }while(load_ok);
}

/** Imprime la ayuda del programa.
 * \param programName Nombre del programa. Se pasa por parámetro por si se 
                        cambia al compilar. Encontrado en arv[0].
*/
void print_help(char * programName){
    printf("\n\nUSO: %s -s source -t sink [OPCIONES] < NETWORK\n\n", programName);
    printf("OPCIONES:\n");
    printf("\t-s SOURCE \t\tConfigura al nodo SOURCE como fuente.\n");
    printf("\t-t SINK \t\tConfigura al nodo SINK como resumidero.\n");
    printf("\t-vf --valorflujo \tImprime el valor del flujo.\n");
    printf("\t-f --flujo \t\tImprime el flujo.\n");
    printf("\t-c --corte \t\tImprime el corte.\n");
    printf("\t-p --path \t\tImprime los caminos aumentantes.\n");
    printf("\t-r --reloj \t\tImprime el tiempo en hh:mm:ss.ms de la ejecución "
            "ignorando el tiempo\n\t\t\t\tde carga de datos. Usarlo sin otros "
            "parámetros de impresion para\n\t\t\t\tobtener el tiempo de calculo"
            " del algoritmo de Dinic. \n");
    printf("\t-a --all \t\tEquivalente a -vf -f -p y -c.\n\n");
    printf("\tNETWORK\t\t\tUna serie de elementos de la forma: x y c \\n, que "
            "representan el lado\n\t\t\t\tx->y de capacidad c.\n\n");
    printf("Ejemplo: $%s -f -vf -s 1 -t 0 < network.txt\n\n", programName);
}

/**Verifica si los parámetros con los que se invoco al programa son correctos.
 * \param argc Indica el largo del arreglo argv.
 * \param argv Vector de argumentos con los que se invoco al programa.
 * \param source Fuente del Network.
 * \param sink Resumidero del Network.
 * \return Retorna un Short int indicando el estado de los parámetros.\n
 * - Estados:\n
 *      - DONT_DINIC   Permite (o no) que se realice dinic.\n
 *      - S_OK        Indica que 's' fue pasado como parámetro.\n
 *      - T_OK        Indica que 't' fue pasado como parámetro.\n
 *      - DINIC_TIME  Activa el muestreo de tiempos de calculo.\n
 *      - PATH        Activa el muestreo de caminos aumentantes.\n
 *      - FLOW        Activa el muestreo de flujo.\n
 *      - CUT         Activa el muestreo de corte.\n
 *      - FLOW_V      Activa el muestre de valor del flujo.\n
*/
short int parametersChecker(int argc, char *argv[], u64 * source, u64 * sink){
    int i = 1;                          /*Iterador para parámetros de entrada. 
                                        Saltea el nombre del programa.*/
    short int STATUS = CLEAR_FLAG();    /*Retorno de la función.*/
    short int HELP = 0;                 /*Se pide ayuda. Flag de alcance local*/
    
    /*Valida cada uno de los parámetros de entrada en un loop que termina cuando
     * se pide imprimir la ayuda o cuando se leyeron todos los parámetros*/
    while (i < argc && !HELP){
        /*Se fija si el parámetro indica que se debe imprimir el flujo.*/
        if(strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--flujo")== 0 )
            SET_FLAG(FLOW);
        /*Se fija si el parámetro indica que se debe imprimir el valor del flujo*/
        else if(strcmp(argv[i], "-vf") == 0 || strcmp(argv[i], "--valorflujo")== 0 )
            SET_FLAG(FLOW_V);
        /*Se fija si el parámetro indica que se debe imprimir el corte.*/
        else if(strcmp(argv[i], "-c") == 0 || strcmp(argv[i],"--corte" )== 0 )
            SET_FLAG(CUT);
        /*Se fija si el parámetro indica que se debe imprimir los caminos.*/
        else if(strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--path")== 0 )
            SET_FLAG(PATH);
        /*Se fija si el parámetro indica cual nodo va a ser la fuente.*/
        else if(strcmp(argv[i], "-s") == 0 && !IS_SET_FLAG(S_OK)){
            if (i+1 < argc){/*Valida que exista un siguiente argumento(Fuente)*/
                if(isu64(argv[i+1]))/*Valida que sea u64*/
                    sscanf(argv[i+1], "%" SCNu64, source);
                else{
                    /*No es u64, se imprime el error y no corre el algoritmo.*/
                    printf("%s: -s: Invalid argument \"%s\".\n", argv[0], argv[i+1]);
                    SET_FLAG(DONT_DINIC);
                }
                SET_FLAG(S_OK);
                i++;
            }else
                SET_FLAG(DONT_DINIC);
        /*Se fija si el parámetro indica cual nodo va a ser el resumidero.*/
        }else if(strcmp(argv[i], "-t") == 0 && !IS_SET_FLAG(T_OK)){
            if (i+1 < argc){
                if(isu64(argv[i+1]))
                    sscanf(argv[i+1], "%" SCNu64, sink);
                else{
                    printf("%s: -t: Invalid argument \"%s\".\n", argv[0], argv[i+1]);
                    SET_FLAG(DONT_DINIC);
                }
                SET_FLAG(T_OK);
                i++;
            }else
                SET_FLAG(DONT_DINIC);
        /*Este parámetro setea todos los demás parámetros, menos el de ayuda.*/

        }else if(strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--all")== 0 ){
            SET_FLAG(PATH);
            SET_FLAG(FLOW);
            SET_FLAG(CUT);
            SET_FLAG(FLOW_V);
            SET_FLAG(DINIC_TIME);
        /*Se fija si el parámetro indica que se debe imprimir el tiempo que 
         * tarda el algoritmo DINIC.*/
        }else if(strcmp(argv[i], "-r") == 0 || strcmp(argv[i],"--reloj" )== 0 )
            SET_FLAG(DINIC_TIME);
        /*Se fija si el parámetro indica que se debe imprimir el menú de ayuda*/
        else if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help")== 0 ){
            print_help(argv[0]);
            HELP = 1;
        }else{/*default case*/
            printf("%s: %s: Invalid Option.\n", argv[0], argv[i]);
            SET_FLAG(DONT_DINIC);
        }
        i++;
    }/*Finaliza el ciclo while.*/
   /*Imprime un mensaje de error porque no se seteó la fuente
   y configura que no se corra el algoritmo DINIC*/
    if(!IS_SET_FLAG(S_OK) && !HELP){
        printf("%s: -s is not set.\n", argv[0]);
        SET_FLAG(DONT_DINIC);
    }
   /*Imprime un mensaje de error porque no se seteó el resumidero
   y configura que no se corra el algoritmo DINIC*/
    if(!IS_SET_FLAG(T_OK) && !HELP){
        printf("%s: -t is not set.\n", argv[0]);
        SET_FLAG(DONT_DINIC);
    }
    
    /*Si se pidio imprimir ayuda entonces se configura que no corra 
     *el algoritmo de DINIC*/
    if(HELP)
        SET_FLAG(DONT_DINIC);
    /*Si se cometió un error con algún parámetro, se le aconseja imprimir ayuda*/
    else if(IS_SET_FLAG(DONT_DINIC))
        printf("use -h for help.\n");
    
    return STATUS;
}

/** Verifica que se le haya pasado como parámetro algo de tipo u64.
 * \param sU64 String para verificar que sea u64.
 * \pre su64 no es nulo.
 * \return  True si el parámetro es un u64. 
 *          False caso contrario.
 */
bool isu64(char * sU64){
    int i = 0;
    bool result = true;
    assert(sU64 != NULL);
    while(sU64[i] != '\0' && result){
        result =result && isdigit(sU64[i]);
        i++;
    }
    return result;    
}
/** Imprime el tiempo que tarda el algoritmo DINIC en ejecutarse en formato 
 * [hh:mm:ss.ms]. 
 * No contabiliza el tiempo de carga de los elementos del network ni otras 
 * acciones innecesarias. Esto es así porque se supone que el tiempo de carga 
 * varia mucho cuando se pide al sistema operativo interactuar con IO. 
 * De esta manera se logra un tiempo solamente de la parte del programa que 
 * implementa DINIC.
    \param time Tiempo que tardo el algoritmo DINIC en correr medidos segundos.
*/
void print_dinicTime(float time){    
    int hs,min,sec,ms;

    ms = (int) (time * 1000) % 1000;
    sec = (int) time % 60;
    min = (int) (time / 60) % 60;
    hs = (int) time / 3600;
    printf("\nDinic demoró(hh:mm:ss.ms): %02i:%02i:%02i.%03i\n\n", hs, min, sec, ms);
}

/** Algoritmo principal. 
 * Se encarga de llamar las funciones del API para implementar DINIC.
 * \param argc Cantidad de argumentos con los que se invoco el programa.
 * \param argv Vector de argumentos con los que se invoco el programa.
 * \return Retorna un 0 si el programa termina.
 */
int main(int argc, char *argv[]){
    DovahkiinP dova = NULL; /*El dovahkiin que voy a usar*/
    u64 s = NULL;           /*El nombre del nodo que sera fuente*/
    u64 t = NULL;           /*El nombre del nodo que sera resumidero*/
    short int STATUS;       /*Estado de ejecución según el ingreso de parámetros*/
    clock_t clock_startTime = 0; /*Registra el tiempo de inicio*/
    clock_t clock_finishTime = 0; /*Registra el tiempo de finalización*/
    float dinicTime = 0;    /*Resultado del tiempo*/
    
    /*Se controlan los parámetros de ingreso*/
    STATUS = parametersChecker(argc, argv, &s, &t);

    /* Se crea un nuevo dova y se cargan los valores del network*/
    dova = NuevoDovahkiin();
    assert(dova != NULL);
    if(!IS_SET_FLAG(DONT_DINIC))
        load_from_stdin(dova);
    
    /*se calcula e imprime lo requerido*/
    FijarFuente(dova, s);
    FijarResumidero(dova, t);
    
//  Funciones no utilizadas en nuestro main. Quedan para testeo de la API.
//  ImprimirFuente(dova);
//  ImprimirResumidero(dova);

    if (Prepararse(dova) == 1){
        if (IS_SET_FLAG(DINIC_TIME))
            clock_startTime = clock();
        
        while (ActualizarDistancias(dova)){
            while (BusquedaCaminoAumentante(dova)){
                if (IS_SET_FLAG(PATH)){
                    AumentarFlujoYTambienImprimirCamino(dova); 
                }else{
                    AumentarFlujo(dova); 
                }  
            }
        }
        /*Imprimo resultados de lo que se haya pedido*/
        if (IS_SET_FLAG(FLOW))
            ImprimirFlujo(dova);

        if (IS_SET_FLAG(FLOW_V))
            ImprimirValorFlujo(dova);
            
        if (IS_SET_FLAG(CUT))
            ImprimirCorte(dova);
        
        if (IS_SET_FLAG(DINIC_TIME)){
            clock_finishTime = clock();
            dinicTime = (double)(clock_finishTime - clock_startTime) / CLOCKS_PER_SEC;
            print_dinicTime(dinicTime);
        }
    }
    /* destruyo el dova*/
    if (!DestruirDovahkiin(dova))
        printf("Error al intentar liberar el dova\n");

    return 1;
}
