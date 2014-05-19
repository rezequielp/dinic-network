#include <stdio.h>

static void load_from_stdin(DovahkiinP net);

int main(){
    DovahkiinP network = NULL;  /*network principal*/
    int parameters = 0;
    u64 s;
    u64 t;
    
    /*Se controlan los parametros de ingreso*/
    if (ctrl_parameters(argc, argv)){
        /* Se crea un nuevo network con la verbosidad*/
        /* TODO cambiar esto segun los argumentos que pasemos*/
        network = NuevoDovahkiin();
        parameters = aoti(argv[PARAMETERS])
    }
    
    assert(network != NULL)
    /* Se carga los valores del network*/
    load_from_stdin(network);
    
    /*se calcula e imprime lo requerido*/
    FijarFuente(network, s);
    FijarResumidero(network, t);
    
    Prepararse(network);
    ActualizarDistancias(network);

    while BusquedaCaminoAumentante(network){
        if ("seteado imprimir camino aumentante")
            AumentarFlujoYtambienImprimirCamino(network);
        else
            AumentarFlujo(network);
        
        if ("seteado imprimir flujo")
            ImprimirFlujo(netwrok);
    }
    
    if ("seteado imprimir valor flujo ")
        ImprimirValorFlujo(netwrok);
    
    if ("imprimir corte")
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
    Lado lado;          /*un lado leido*/
    int load_ok = 0;        /*indica si el lado se pudo cargar*/
    
    assert(network != NULL);
    
    /* no hace falta limpiar el input porque lexer se come todo lo leido
     * caso vacio -> leerUnLado retorna LadoNulo*/
    
    do{
        lado = LeerUnLado(network);
        load_ok = CargarUnLado(network, lado)
    }while(lado!=LadoNulo && load_ok)
}