#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <assert.h>
#include <string.h>
//#include "API.h"
#include "../apifiles/API.h"


/*MACROS PARA MANEJAR LOS PARAMETROS DE ENTRADA
*/
#define IMP_ERROR                   0b10000000
#define IMP_MISSINGOPTION_S         0b01000000
#define IMP_MISSINGOPTION_T         0b00100000
#define IMP_TIEMPODINIC             0b00010000
#define IMP_CAMINO_AUMENTANTE       0b00001000
#define IMP_FLUJO                   0b00000100
#define IMP_CORTE                   0b00000010
#define IMP_VALOR_FLUJO             0b00000001

#define SET_IMPST(f) STATUS |= f
#define UNSET_IMPST(f) STATUS &= ~f
#define CLEAR_IMPST() 0b00000000
#define IS_SET_IMPST(f) (STATUS & f) > 0


static void load_from_stdin(DovahkiinP network);
static void print_help(char * programName);
static short int parametersChecker(int argc, char *argv[], u64 * source, u64 * sink);
static void print_dinicTime(int time);
static bool isu64(char * sU64);



void load_from_stdin(DovahkiinP network){
    Lado lado = LadoNulo;          /*un lado leido*/
    int load_ok = 0;        /*indica si el lado se pudo cargar*/
    
    assert(network != NULL);
    
    /* no hace falta limpiar el input porque lexer se come todo lo leido

     * caso vacio -> leerUnLado retorna LadoNulo*/
    
    do{
        lado = LeerUnLado();
        load_ok = CargarUnLado(network, lado);
    }while(load_ok);
}

void print_help(char * programName){
    printf("\n\nUSO: %s -s source -t sink [OPCIONES] < NETWORK\n\n", programName);
    printf("OPCIONES:\n");
    printf("\t-s SOURCE \t\tConfigura al nodo SOURCE como fuente.\n");
    printf("\t-t SINK \t\tConfigura al nodo SINK como resumidero.\n");
    printf("\t-vf --valorflujo \tImprime el valor del flujo.\n");
    printf("\t-f --flujo \t\tImprime el flujo.\n");
    printf("\t-c --corte \t\tImprime el corte.\n");
    printf("\t-p --path \t\tImprime los caminos aumentantes.\n");
    printf("\t-r --reloj \t\tImprime el tiempo en hh:mm:ss del algoritmo Dinic sin tener en cuenta el cargado de datos.\n");
    printf("\t-a --all \t\tEquivalente a -vf -f -p y -c.\n\n");
    printf("\tNETWORK: Una serie de elementos de la forma x y z\\n que representan el nodo x->y de capacidad z terminada con un EOF.\n\n");
    printf("Ejemplo: $%s -f -vf -s 1 -t 0 <network.txt\n\n", programName);
}

short int parametersChecker(int argc, char *argv[], u64 * source, u64 * sink){
    int i = 1;
    short int STATUS = CLEAR_IMPST();
    
    SET_IMPST(IMP_MISSINGOPTION_S);
    SET_IMPST(IMP_MISSINGOPTION_T);
    
    while (i < argc){
        if(strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--flujo")== 0 )
            SET_IMPST(IMP_FLUJO);
            
        else if(strcmp(argv[i], "-vf") == 0 || strcmp(argv[i], "--valorflujo")== 0 )
            SET_IMPST(IMP_VALOR_FLUJO);
            
        else if(strcmp(argv[i], "-c") == 0 || strcmp(argv[i],"--corte" )== 0 )
            SET_IMPST(IMP_CORTE );
            
        else if(strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--path")== 0 )
            SET_IMPST(IMP_CAMINO_AUMENTANTE);
            
        else if(strcmp(argv[i], "-s") == 0 && IS_SET_IMPST(IMP_MISSINGOPTION_S)){
            if (i+1 < argc){
                if(isu64(argv[i+1]))
                    sscanf(argv[i+1], "%" SCNu64, source);
                else{
                    printf("%s: -s: Invalid argument \"%s\".\n", argv[0], argv[i+1]);
                    SET_IMPST(IMP_ERROR);
                }
                UNSET_IMPST(IMP_MISSINGOPTION_S);
                i++;
            }
            
        }else if(strcmp(argv[i], "-t") == 0 && IS_SET_IMPST(IMP_MISSINGOPTION_T)){
            if (i+1 < argc){
                if(isu64(argv[i+1]))
                    sscanf(argv[i+1], "%" SCNu64, sink);
                else{
                    printf("%s: -t: Invalid argument \"%s\".\n", argv[0], argv[i+1]);
                    SET_IMPST(IMP_ERROR);
                }
                UNSET_IMPST(IMP_MISSINGOPTION_T);
                i++;
            }
            
        }else if(strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--all")== 0 ){
            SET_IMPST(IMP_CAMINO_AUMENTANTE);
            SET_IMPST(IMP_FLUJO);
            SET_IMPST(IMP_CORTE);
            SET_IMPST(IMP_VALOR_FLUJO);
            SET_IMPST(IMP_TIEMPODINIC);
            
        }else if(strcmp(argv[i], "-r") == 0 || strcmp(argv[i],"--reloj" )== 0 )
            SET_IMPST(IMP_TIEMPODINIC);

        else if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help")== 0 )
            print_help(argv[0]);
        else{/*default case*/
            printf("%s: %s: Invalid Option.\n", argv[0], argv[i]);
            SET_IMPST(IMP_ERROR);
        }
        i++;
    }
    i--;
    if(IS_SET_IMPST(IMP_MISSINGOPTION_S)){
        printf("%s: -s is not set.\n", argv[0]);
        SET_IMPST(IMP_ERROR);
    }
    
    if(IS_SET_IMPST(IMP_MISSINGOPTION_T)){
        printf("%s: -t is not set.\n", argv[0]);
        SET_IMPST(IMP_ERROR);
    }
    
    if(IS_SET_IMPST(IMP_ERROR))
        print_help(argv[0]);
    
    return STATUS;
}


bool isu64(char * sU64){
    int i = 0;
    bool result = true;
    while(sU64[i] != '\0' && result){
        result =result && isdigit(sU64[i]);
        i++;
    }
    return result;    
}

void print_dinicTime(int time){
    int hs,min,sec;
    sec = time % 60;
    min = time/60 % 60;
    hs = time/3600;
    printf("\nDinic demoró(hh:mm:ss): %2i:%2i:%2i\n\n", hs, min, sec);
}

int main(int argc, char *argv[]){
    DovahkiinP network = NULL;  /*network principal*/
    u64 s = NULL;
    u64 t = NULL;
    short int STATUS;
    clock_t clock_startTime, clock_finishTime;
    double dinicTime;
    
    /*Se controlan los parametros de ingreso*/
    STATUS = parametersChecker(argc, argv, &s, &t);

    /* Se crea un nuevo network*/
    network = NuevoDovahkiin();

    assert(network != NULL);
    if(!IS_SET_IMPST(IMP_MISSINGOPTION_S) && !IS_SET_IMPST(IMP_MISSINGOPTION_T))
        /* Se carga los valores del network*/
        load_from_stdin(network);
    
    /*se calcula e imprime lo requerido*/
    FijarFuente(network, s);
    FijarResumidero(network, t);
//    ImprimirFuente(network);
//    ImprimirResumidero(network);
    if (Prepararse(network) == 1){
        if (IS_SET_IMPST(IMP_TIEMPODINIC))
            clock_startTime = clock(); 
            
        while (ActualizarDistancias(network)){        
            while (BusquedaCaminoAumentante(network)){
                if (IS_SET_IMPST(IMP_CAMINO_AUMENTANTE)){
                    AumentarFlujoYTambienImprimirCamino(network); 
                    /*printf("Finish3\n");*/
                }else{
                    /*printf("pitones 4:\n");*/
                    AumentarFlujo(network); 
                    /*printf("Finish4\n");*/
                }  
            }
        }
        
        if (IS_SET_IMPST(IMP_TIEMPODINIC)){
            clock_finishTime = clock();
            dinicTime = (clock_finishTime-clock_startTime) / CLOCKS_PER_SEC; /*WARNING tipo de dato devuelto por clock(), generalmente un long int*/
/* printf("pitones7:\n");*/
            print_dinicTime(dinicTime);
        }
        if (IS_SET_IMPST(IMP_FLUJO)){
             /*printf("pitones5:");*/
            ImprimirFlujo(network);
             /*printf("Finish5\n");*/
        }
         /*printf("pitones8:\n");*/
        if (IS_SET_IMPST(IMP_VALOR_FLUJO))
            ImprimirValorFlujo(network);
/* printf("pitones9:\n");*/
        if (IS_SET_IMPST(IMP_CORTE))
            ImprimirCorte(network);    
    }

    
    /* destruyo el network*/
    if (!DestruirDovahkiin(network)){
  
        printf("Error al intentar liberar el network\n");}
  /*printf("pitones10:\n");*/
    return 0;
}
