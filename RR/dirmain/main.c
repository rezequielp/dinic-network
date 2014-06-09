#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <assert.h>

#include "../apifiles/API.h"

/*MACROS PARA MANEJAR LOS PARAMETROS DE ENTRADA
*/
#define IMP_ERROR                   0b10000000
#define IMP_TIEMPODINIC             0b00010000
#define IMP_CAMINO_AUMENTANTE       0b00001000
#define IMP_FLUJO                   0b00000100
#define IMP_CORTE                   0b00000010
#define IMP_VALOR_FLUJO             0b00000001

#define SET_IMPST(f) STATUS |= f
#define UNSET_IMPST(f) STATUS &= ~f
#define CLEAR_IMPST() 0b00000000
#define IS_SET_IMPST(f) (STATUS & f) > 0


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
    printf("USO: %s -s source -t sink [OPCIONES] NETWORK\n\n", programName);
    printf("OPCIONES:\n");
    printf("\t-s SOURCE: Configura al nodo SOURCE como fuente.\n");
    printf("\t-s SINK: Configura al nodo SINK como resumidero.\n");
    printf("\t-vf --valorflujo: Imprime el valor del flujo.\n");
    printf("\t-f --flujo: Imprime el flujo.\n");
    printf("\t-c --corte: Imprime el corte.\n");
    printf("\t-p --path: Imprime los caminos aumentantes.\n");
    printf("\t-r --reloj: Imprime el tiempo en hh:mm:ss del algoritmo Dinic sin tener en cuenta el cargado de datos.\n");
    printf("\t-all: Equivalente a -vf -f -p y -c.\n\n");
    printf("\tNETWORK: Una serie de elementos de la forma x y z\\n que representan el nodo x->y de capacidad z terminada con un ENF.\n");
    printf("Ejemplo: $%s -f -vf -s 1 -t 0 <network.txt\n\n", programName);
}

short int parametersChecker(int argc, char *argv[]){
    int counter;
    int i = 1;
    short int STATUS = CLEAR_IMPST();
    char * source, * sink;
    
    while (counter < argc && !IS_SET_IMPST(IMP_ERROR)){
        if(strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--flujo")== 0 )
            SET_IMPST(IMP_CAMINO_AUMENTANTE);
            
        else if(strcmp(argv[i], "-vf") == 0 || strcmp(argv[i], "--valorflujo")== 0 )
            SET_IMPST(IMP_FLUJO);
            
        else if(strcmp(argv[i], "-c") == 0 || strcmp(argv[i],"--corte" )== 0 )
            SET_IMPST(IMP_CORTE );
            
        else if(strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--path")== 0 )
            SET_IMPST(IMP_VALOR_FLUJO);
            
        else if(strcmp(argv[i], "-s") == 0 ){
            source = argv[i+1];
            i++;
    
        }else if(strcmp(argv[i], "-t") == 0 ){
            sink = argv[i+1];
            i++;
            
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
            printf("%s: %s: Invalid option.\n", argv[0], argv[i]);
            print_help(argv[0]);
            SET_IMPST(IMP_ERROR);
        }
    }
    return STATUS;
}

/*TODO?*/
/*bool isu64(char * sU64){

    bstring bU64;
    

    bU64 bfromcstr(sU64);
    

    
}*/

void print_dinicTime(int time){
    int hs,min,sec;
    sec = time % 60;
    min = time/60 % 60;
    hs = time/3600;
    printf("\nDinic demoró(hh:mm:ss): %2i:%2i:%2i\n\n", hs, min, sec);
}

int main(int argc, char *argv[]){
    DovahkiinP network = NULL;  /*network principal*/
    int parameters = 0;
    u64 s = NULL;
    u64 t = NULL;
    short int STATUS;
    int counter = 0;
    u64 source, sink;
    int break_w=0;
    clock_t clock_startTime, clock_finishTime;
    double dinicTime;
    
    /*Se controlan los parametros de ingreso*/
    STATUS = parametersChecker(argc, argv);

    /* Se crea un nuevo network*/
    /* TODO cambiar esto segun los argumentos que pasemos*/
    network = NuevoDovahkiin();

    assert(network != NULL);
    /* Se carga los valores del network*/
    load_from_stdin(network);
    
    /*se calcula e imprime lo requerido*/
    FijarFuente(network, s);
    FijarResumidero(network, t);
    
    Prepararse(network);
    ActualizarDistancias(network);
    
    if (IS_SET_IMPST(IMP_TIEMPODINIC))
        clock_startTime = clock(); 
    
    while (BusquedaCaminoAumentante(network)){
        if (IS_SET_IMPST(IMP_CAMINO_AUMENTANTE))
            AumentarFlujoYtambienImprimirCamino(network);
        else
            AumentarFlujo(network);
        
        if (IS_SET_IMPST(IMP_FLUJO))
            ImprimirFlujo(network);
    }
    
    if (IS_SET_IMPST(IMP_TIEMPODINIC)){
        clock_finishTime = clock();
        dinicTime = (clock_finishTime-clock_startTime) / CLOCKS_PER_SEC; /*WARNING tipo de dato devuelto por clock(), generalmente un long int*/
        
        print_dinicTime(dinicTime);
    }
    if (IS_SET_IMPST(IMP_VALOR_FLUJO))
        ImprimirValorFlujo(network);
    
    if (IS_SET_IMPST(IMP_CORTE))
        ImprimirCorte(network);
    
    /* destruyo el network*/
    if (!DestruirDovahkiin(network))
        printf("Error al intentar liberar el network\n");

    return 0;
}


/*Carga los lados del network desde el stdin
 * Pre: network != NULL
 */



