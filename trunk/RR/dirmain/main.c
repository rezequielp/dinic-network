/**Main para probar la API que corre el algoritmo DINIC en networks.*/
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include "../apifiles/API.h"

/**MACROS PARA MANEJAR LOS PARAMETROS DE ENTRADA.*/
/**\def Flags.*/
#define DONT_DINIC      0b10000000  /**<Permite (o no) que se realice dinic.*/
#define S_OK            0b01000000  /**<'s' fue pasado como parametro.*/
#define T_OK            0b00100000  /**<'t' fue pasado como parametro.*/
#define DINIC_TIME      0b00010000  /**<Activa el muestreo de tiempos de calculo.*/
#define PATH            0b00001000  /**<Activa el muestreo de caminos aumentantes.*/
#define FLOW            0b00000100  /**<Activa el muestreo de flujo.*/
#define CUT             0b00000010  /**<Activa el muestreo de corte.*/
#define FLOW_V          0b00000001  /**<Activa el muestre de valor del flujo.*/
/**\def Macros para manipular flags.*/
#define SET_FLAG(f) STATUS |= f         /*Setea un bit de flag en STATUS en 1.*/
#define UNSET_FLAG(f) STATUS &= ~f      /**<Setea un bit de flag en STATUS en 0.*/
#define CLEAR_FLAG() 0b00000000         /**<Pone todas las flags en 0.*/
#define IS_SET_FLAG(f) (STATUS & f) > 0 /**<Consulta si una flag esta activa.*/

/*FUNCIONES ESTATICAS.*/
static void load_from_stdin(DovahkiinP dova);
static void print_help(char * programName);
static short int parametersChecker(int argc, char *argv[], u64 * source, u64 * sink);
static void print_dinicTime(float time);
static bool isu64(char * sU64);

/**Lee desde el standard input los lados del network y los carga en dovahkiin, 
hasta acabar los lados o bien hasta el primer lado que no se pueda cargar
    \param dova DovahkiinP donde carga los lados.*/
void load_from_stdin(DovahkiinP dova){

    Lado lado = LadoNulo;/**<Lado leido. Caso vacio se retorna LadoNulo.*/
    int load_ok = 0;/**<Indica si el lado se pudo cargar.*/
    
    assert(dova != NULL);
    do{
        lado = LeerUnLado();
        load_ok = CargarUnLado(dova, lado);
    }while(load_ok);
}

/**Imprime la ayuda del programa.
    \param programName Nompre del programa. Se pasa por parametro por si se cambia al compilar. Encotrado en arv[0].*/
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
    printf("Ejemplo: $%s -f -vf -s 1 -t 0 < network.txt\n\n", programName);
}

/**Verifica si los paramentros con los que se invoco al programa son correctos.
    \param argc Indica el largo de arreglo argv.
    \param argv Vector de argumentos con los que se invoco al programa.
    \param source Fuente del Network.
    \param sink Resumidero del Network.
    \return STATUS Retorna un Short int indicando el estado de los parametros.
    Estados:
        DONT_DINIC   Permite (o no) que se realice dinic.
        S_OK        s' fue pasado como parametro.
        T_OK        t' fue pasado como parametro.
        DINIC_TIME  Activa el muestreo de tiempos de calculo.
        PATH        Activa el muestreo de caminos aumentantes.
        FLOW        Activa el muestreo de flujo.
        CUT         Activa el muestreo de corte.
        FLOW_V      Activa el muestre de valor del flujo.
    Macros para manipular los estados:
        SET_FLAG(f)     Setea un bit de flag en STATUS en 1.
        UNSET_FLAG(f)   Setea un bit de flag en STATUS en 0.
        CLEAR_FLAG()    Pone todas las flags en 0.
        IS_SET_FLAG(f)  Consulta si una flag esta activa.*/
short int parametersChecker(int argc, char *argv[], u64 * source, u64 * sink){
    int i = 1;/**<Indice para loopear entre los parametros de entrada. Saltea el nombre del programa.*/
    short int STATUS = CLEAR_FLAG();/**<Retorno de la funcion. Se limpia al inicializar.*/
    short int HELP = 0;             /*<flag de alcance local*/
    
    /**<Valida cada uno de los parametros de entrada en un loop que
    termina cuando se pide imprimir la ayuda o cuando se leyeron todos los parametros*/
    while (i < argc && !HELP){
        /**Se fija si el parametro indica que se debe imprimir el flujo.*/
        if(strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--flujo")== 0 )
            SET_FLAG(FLOW);
        /**Se fija si el parametro indica que se debe imprimir el valor del flujo.*/
        else if(strcmp(argv[i], "-vf") == 0 || strcmp(argv[i], "--valorflujo")== 0 )
            SET_FLAG(FLOW_V);
        /**Se fija si el parametro indica que se debe imprimir el corte.*/
        else if(strcmp(argv[i], "-c") == 0 || strcmp(argv[i],"--corte" )== 0 )
            SET_FLAG(CUT);
        /**Se fija si el parametro indica que se debe imprimir los caminos.*/
        else if(strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--path")== 0 )
            SET_FLAG(PATH);
        /**Se fija si el parametro indica que elemento es la fuente.*/
        else if(strcmp(argv[i], "-s") == 0 && !IS_SET_FLAG(S_OK)){
            if (i+1 < argc){/**<Valida que exista un siguiente argumento(Fuente)*/
                if(isu64(argv[i+1]))/**<Valida que sea u64 caso contrario imprime el error y no corre el algoritmo.*/
                    sscanf(argv[i+1], "%" SCNu64, source);
                else{
                    printf("%s: -s: Invalid argument \"%s\".\n", argv[0], argv[i+1]);
                    SET_FLAG(DONT_DINIC);
                }
                SET_FLAG(S_OK);
                i++;
            }else
                SET_FLAG(DONT_DINIC);
            
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
            
        }else if(strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--all")== 0 ){
            SET_FLAG(PATH);
            SET_FLAG(FLOW);
            SET_FLAG(CUT);
            SET_FLAG(FLOW_V);
            SET_FLAG(DINIC_TIME);
            
        }else if(strcmp(argv[i], "-r") == 0 || strcmp(argv[i],"--reloj" )== 0 )
            SET_FLAG(DINIC_TIME);

        else if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help")== 0 ){
            print_help(argv[0]);
            HELP = 1;
        }else{/*default case*/
            printf("%s: %s: Invalid Option.\n", argv[0], argv[i]);
            SET_FLAG(DONT_DINIC);
        }
        i++;
    }
       
    if(!IS_SET_FLAG(S_OK) && !HELP){
        printf("%s: -s is not set.\n", argv[0]);
        SET_FLAG(DONT_DINIC);
    }
    
    if(!IS_SET_FLAG(T_OK) && !HELP){
        printf("%s: -t is not set.\n", argv[0]);
        SET_FLAG(DONT_DINIC);
    }
    
    if(HELP)
        SET_FLAG(DONT_DINIC);
    else if(IS_SET_FLAG(DONT_DINIC))
        printf("use -h for help.\n");
    
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

void print_dinicTime(float time){    
    int hs,min,sec,ms;

    ms = (int) (time * 1000) % 1000;
    sec = (int) time % 60;
    min = (int) (time / 60) % 60;
    hs = (int) time / 3600;
    printf("\nDinic demoró(hh:mm:ss): %02i:%02i:%02i.%03i\n\n", hs, min, sec, ms);
}

int main(int argc, char *argv[]){
    DovahkiinP dova = NULL;
    u64 s = NULL;
    u64 t = NULL;
    short int STATUS;
    clock_t clock_startTime, clock_finishTime;
    float dinicTime = 0;
    
    /*Se controlan los parametros de ingreso*/
    STATUS = parametersChecker(argc, argv, &s, &t);

    /* Se crea un nuevo dova y se cargan los valores del network*/
    dova = NuevoDovahkiin();
    assert(dova != NULL);
    if(!IS_SET_FLAG(DONT_DINIC))
        load_from_stdin(dova);
    
    /*se calcula e imprime lo requerido*/
    FijarFuente(dova, s);
    FijarResumidero(dova, t);
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
        
        if (IS_SET_FLAG(DINIC_TIME)){
            clock_finishTime = clock();
            dinicTime = (double)(clock_finishTime - clock_startTime) / CLOCKS_PER_SEC;
            print_dinicTime(dinicTime);
        }
        if (IS_SET_FLAG(FLOW)){
            ImprimirFlujo(dova);
        }
        if (IS_SET_FLAG(FLOW_V))
            ImprimirValorFlujo(dova);
        if (IS_SET_FLAG(CUT))
            ImprimirCorte(dova);    
    }
    
    /* destruyo el dova*/
    if (!DestruirDovahkiin(dova))
        printf("Error al intentar liberar el dova\n");
    return 0;
}
