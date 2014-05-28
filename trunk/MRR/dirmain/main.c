#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>



/*MACROS PARA MANEJAR LOS PARAMETROS DE ENTRADA
*/
#define IMP_CAMINO_AUMENTANE        0b00001000
#define IMP_FLUJO                   0b00000100
#define IMP_CORTE                   0b00000010
#define IMPR_VALOR_FLUJO            0b00000001

#define SET_IMPST(f) STATUS |= f
#define UNSET_IMPST(f) network->flags &= ~f
#define CLEAR_IMPST() 0b00000000
#define IS_SET_IMPST(f) (network->flags & f) > 0

static void load_from_stdin(DovahkiinP net);

int main(int argc, char *argv[]){
    DovahkiinP network = NULL;  /*network principal*/
    int parameters = 0;
    u64 s = NULLL;
    u64 t = NULL;
    short int status = CLEAR_IMPRST();
    int counter = 0;
    u64 source, sink;
    int break_w=0;
    
    source = u64_new();
    sink = u64_new();
    
    /*Se controlan los parametros de ingreso*/
    while (counter < argc && !break_w){
        switch(argv[i]){
            case('-f'):
            case('--flujo'):
                SET_IMPST(IMP_CAMINO_AUMENTANE);
                break;
            case('-vf'):
            case('--valorflujo'):
                SET_IMPST(IMP_FLUJO);
                break
            case('-c'):
            case('--corte'):
                SET_IMPST(IMP_CORTE );
                break;
            case('-p'):
            case('--path'):
                SET_IMPST(IMPR_VALOR_FLUJO);
                break;
            case('-s'):
                /*if(isu64(argv[i+1])){*/
                    source = argv[i+1];
                    i++;
                /*}else{
                    printf("%s: s%: is not unsigned 64 bits");
                    break_w = true;
                }*/
                break;
            case('-t'):
                sink = argv[i+1];
                i++;
                break;
            case('-a'):
            case('-all'):
                SET_IMPST(IMP_CAMINO_AUMENTANE);
                SET_IMPST(IMP_FLUJO);
                SET_IMPST(IMP_CORTE);
                SET_IMPST(IMPR_VALOR_FLUJO);
                break;
            case ('-h'):
            case ('-help'):
                print_help():
                break;
            default:
                printf("%s: %s: Invalid option.\n", argv[0], argv[i]);
                print_help(argv[0]);
                break;
        }
    }
    /* Se crea un nuevo network*/
    /* TODO cambiar esto segun los argumentos que pasemos*/
    network = NuevoDovahkiin();

    assert(network != NULL)
    /* Se carga los valores del network*/
    load_from_stdin(network);
    
    /*se calcula e imprime lo requerido*/
    FijarFuente(network, s);
    FijarResumidero(network, t);
    
    Prepararse(network);
    ActualizarDistancias(network);

    while BusquedaCaminoAumentante(network){
        if (IS_SET_IMPST(IMP_CAMINO_AUMENTANE))
            AumentarFlujoYtambienImprimirCamino(network);
        else
            AumentarFlujo(network);
        
        if (IS_SET_IMPST(IMP_FLUJO))
            ImprimirFlujo(netwrok);
    }
    
    if (IS_SET_IMPST(IMPR_VALOR_FLUJO))
        ImprimirValorFlujo(netwrok);
    
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
void load_from_stdin(DovahkiinP network){
    Lado lado = NULL;          /*un lado leido*/
    int load_ok = 0;        /*indica si el lado se pudo cargar*/
    
    assert(network != NULL);
    
    /* no hace falta limpiar el input porque lexer se come todo lo leido
     * caso vacio -> leerUnLado retorna LadoNulo*/
    
    do{
        lado = LeerUnLado(network);
        load_ok = CargarUnLado(network, lado)
    }while(lado!=LadoNulo && load_ok)
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
    printf("\t-all: Equivalente a -vf -f -p y -c.\n\n");
    printf("\tNETWORK: Una serie de elementos de la forma x y z\\n que representan el nodo x->y de capacidad z terminada con un ENF.\n");
    printf("Ejemplo: $%s -f -vf -s 1 -t 0 <network.txt\n\n", programName);
}

/*TODO?*/
/*bool isu64(char * sU64){
    bstring bU64;
    
    bU64 bfromcstr(sU64);
    
    
}*/