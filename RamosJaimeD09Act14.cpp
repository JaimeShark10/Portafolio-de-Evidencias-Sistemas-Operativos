#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <windows.h>
#include <conio.h>
#include <ctype.h>

#define MAX_PROCESOS 200

#define TAM_MEMORIA 240
#define TAM_MARCO 5

#define TOTAL_MARCOS 48
#define MARCOS_SO 4
#define MARCOS_USUARIO 44

#define BLOQUEO 8

#define NUEVO 0
#define LISTO 1
#define EJECUCION 2
#define BLOQUEADO 3
#define TERMINADO 4
#define ERROR_ESTADO 5

/* ========================================================= */
/* ESTRUCTURAS */
/* ========================================================= */

typedef struct{

    int libre;
    int procesoID;
    int pagina;

}Marco;

/* ========================================================= */

typedef struct{

    int id;

    char op;

    int a,b;

    int tme;
    int tt;
    int tr;

    double resultado;

    int estado;

    int llegada;
    int finalizacion;
    int retorno;
    int respuesta;
    int espera;
    int servicio;

    int respondido;

    int tbloq;

    /* PAGINACION */

    int tamanio;
    int paginas;

    int marcos[20];

}Proceso;

/* ========================================================= */
/* VARIABLES GLOBALES */
/* ========================================================= */

Proceso p[MAX_PROCESOS];

Marco memoria[TOTAL_MARCOS];

int cola[MAX_PROCESOS];

int frente=0;
int fin=0;

int totalProcesos=0;
int procesosTerminados=0;

int relojGlobal=0;

int quantum=0;
int quantumActual=0;

int ejec=-1;

/* ========================================================= */
/* FUNCIONES */
/* ========================================================= */

void limpiar(){

    system("cls");
}

/* ========================================================= */

int aleatorio(int min,int max){

    return rand()%(max-min+1)+min;
}

/* ========================================================= */

char opRandom(){

    char ops[] = {'+','-','*','/','%','^'};

    return ops[rand()%6];
}

/* ========================================================= */

double resolver(char op,int a,int b){

    switch(op){

        case '+': return a+b;
        case '-': return a-b;
        case '*': return a*b;
        case '/': return (double)a/b;
        case '%': return a%b;
        case '^': return pow(a,b);
    }

    return 0;
}

/* ========================================================= */
/* MEMORIA */
/* ========================================================= */

void inicializarMemoria(){

    int i;

    for(i=0;i<TOTAL_MARCOS;i++){

        memoria[i].libre=1;

        memoria[i].procesoID=-1;

        memoria[i].pagina=-1;
    }

    /* SISTEMA OPERATIVO */

    for(i=44;i<48;i++){

        memoria[i].libre=0;

        memoria[i].procesoID=0;
    }
}

/* ========================================================= */

int marcosLibres(){

    int i,c=0;

    for(i=0;i<44;i++){

        if(memoria[i].libre)
            c++;
    }

    return c;
}

/* ========================================================= */

int asignarMemoria(int idx){

    int i;
    int pagina=0;

    if(marcosLibres() < p[idx].paginas)
        return 0;

    for(i=0;i<44 && pagina<p[idx].paginas;i++){

        if(memoria[i].libre){

            memoria[i].libre=0;

            memoria[i].procesoID=p[idx].id;

            memoria[i].pagina=pagina;

            p[idx].marcos[pagina]=i;

            pagina++;
        }
    }

    return 1;
}

/* ========================================================= */

void liberarMemoria(int idx){

    int i;

    for(i=0;i<p[idx].paginas;i++){

        int m=p[idx].marcos[i];

        memoria[m].libre=1;

        memoria[m].procesoID=-1;

        memoria[m].pagina=-1;
    }
}

/* ========================================================= */
/* COLA */
/* ========================================================= */

int colaVacia(){

    return frente==fin;
}

/* ========================================================= */

void encolar(int x){

    cola[fin]=x;

    fin=(fin+1)%MAX_PROCESOS;
}

/* ========================================================= */

int desencolar(){

    int x=cola[frente];

    frente=(frente+1)%MAX_PROCESOS;

    return x;
}

/* ========================================================= */
/* PROCESOS */
/* ========================================================= */

void generarProceso(int i,int id){

    p[i].id=id;

    p[i].op=opRandom();

    p[i].a=aleatorio(1,50);

    do{

        p[i].b=aleatorio(1,10);

    }while((p[i].op=='/' || p[i].op=='%') && p[i].b==0);

    if(p[i].op=='^')
        p[i].b=aleatorio(1,5);

    p[i].tme=aleatorio(6,15);

    p[i].tt=0;

    p[i].tr=p[i].tme;

    p[i].resultado=0;

    p[i].estado=NUEVO;

    p[i].llegada=-1;

    p[i].finalizacion=-1;

    p[i].retorno=0;

    p[i].respuesta=-1;

    p[i].espera=0;

    p[i].servicio=0;

    p[i].respondido=0;

    p[i].tbloq=0;

    /* PAGINACION */

    p[i].tamanio=aleatorio(6,30);

    p[i].paginas=
    (p[i].tamanio + TAM_MARCO -1)/TAM_MARCO;
}

/* ========================================================= */

void admitirNuevos(){

    int i;

    for(i=0;i<totalProcesos;i++){

        if(p[i].estado==NUEVO){

            if(asignarMemoria(i)){

                p[i].estado=LISTO;

                p[i].llegada=relojGlobal;

                encolar(i);
            }
        }
    }
}

/* ========================================================= */

void asignarCPU(){

    if(ejec!=-1)
        return;

    if(!colaVacia()){

        ejec=desencolar();

        p[ejec].estado=EJECUCION;

        if(!p[ejec].respondido){

            p[ejec].respuesta=
            relojGlobal-p[ejec].llegada;

            p[ejec].respondido=1;
        }

        quantumActual=0;
    }
}

/* ========================================================= */

void actualizarBloqueados(){

    int i;

    for(i=0;i<totalProcesos;i++){

        if(p[i].estado==BLOQUEADO){

            p[i].tbloq++;

            if(p[i].tbloq>=BLOQUEO){

                p[i].tbloq=0;

                p[i].estado=LISTO;

                encolar(i);
            }
        }
    }
}

/* ========================================================= */

int contarEstado(int estado){

    int i,c=0;

    for(i=0;i<totalProcesos;i++){

        if(p[i].estado==estado)
            c++;
    }

    return c;
}

/* ========================================================= */

void mostrarTerminados(){

    int i;

    printf("\n------ TERMINADOS ------\n\n");

    for(i=0;i<totalProcesos;i++){

        if(p[i].estado==TERMINADO ||
           p[i].estado==ERROR_ESTADO){

            printf("P%d -> ",
            p[i].id);

            if(p[i].estado==ERROR_ESTADO)
                printf("ERROR");

            else
                printf("%.2lf",
                p[i].resultado);

            printf("\n");
        }
    }
}

/* ========================================================= */
/* TABLA DE PAGINAS */
/* ========================================================= */

void mostrarPaginas(){

    int i,j;

    limpiar();

    printf("============= TABLA DE PAGINAS =============\n\n");

    for(i=0;i<totalProcesos;i++){

        if(p[i].estado!=NUEVO){

            printf("Proceso P%d | Tam:%d | Paginas:%d\n",
            p[i].id,
            p[i].tamanio,
            p[i].paginas);

            printf("Pagina -> Marco\n");

            for(j=0;j<p[i].paginas;j++){

                printf("   %d    ->   %d\n",
                j,
                p[i].marcos[j]);
            }

            printf("\n");
        }
    }

    printf("=========== MARCOS ===========\n\n");

    for(i=0;i<TOTAL_MARCOS;i++){

        printf("[%02d:",i);

        if(i>=44)
            printf("SO");

        else if(memoria[i].libre)
            printf("  ");

        else
            printf("P%d",
            memoria[i].procesoID);

        printf("] ");

        if((i+1)%8==0)
            printf("\n");
    }

    printf("\n\nPresione C para continuar...");

    while(1){

        if(kbhit()){

            char t=toupper(getch());

            if(t=='C')
                break;
        }

        Sleep(100);
    }
}

/* ========================================================= */
/* BCP */
/* ========================================================= */

void mostrarBCP(){

    int i;

    while(1){

        limpiar();

        printf("===========================================================\n");
        printf("                 TABLA DE PROCESOS (BCP)\n");
        printf("===========================================================\n\n");

        printf("ID ESTADO       TL TF TR TRES TE TS REST EXTRA\n");

        printf("-----------------------------------------------------------\n");

        for(i=0;i<totalProcesos;i++){

            int tr=0,te=0,rest=0;

            if(p[i].estado==NUEVO){

                tr=0;
                te=0;
            }

            else if(p[i].estado==TERMINADO ||
                    p[i].estado==ERROR_ESTADO){

                tr=p[i].finalizacion-p[i].llegada;

                te=tr-p[i].tt;
            }

            else{

                tr=relojGlobal-p[i].llegada;

                te=tr-p[i].tt;
            }

            rest=p[i].tme-p[i].tt;

            printf("%-2d ",p[i].id);

            switch(p[i].estado){

                case NUEVO:
                    printf("%-12s","NUEVO");
                    break;

                case LISTO:
                    printf("%-12s","LISTO");
                    break;

                case EJECUCION:
                    printf("%-12s","EJECUCION");
                    break;

                case BLOQUEADO:
                    printf("%-12s","BLOQUEADO");
                    break;

                case TERMINADO:
                    printf("%-12s","TERMINADO");
                    break;

                case ERROR_ESTADO:
                    printf("%-12s","ERROR");
                    break;
            }

            printf("%2d %2d %2d %4d %2d %2d %4d ",
            p[i].llegada<0?0:p[i].llegada,
            p[i].finalizacion<0?0:p[i].finalizacion,
            tr,
            p[i].respuesta<0?0:p[i].respuesta,
            te,
            p[i].tt,
            rest);

            if(p[i].estado==BLOQUEADO)
                printf("TRB=%d",
                BLOQUEO-p[i].tbloq);

            else if(p[i].estado==TERMINADO)
                printf("%.2lf",
                p[i].resultado);

            else if(p[i].estado==ERROR_ESTADO)
                printf("ERROR");

            printf("\n");
        }

        printf("\nPresione C para continuar...");

        if(kbhit()){

            char t=toupper(getch());

            if(t=='C')
                break;
        }

        Sleep(100);
    }
}

/* ========================================================= */
/* PANTALLA PRINCIPAL */
/* ========================================================= */

void mostrarPantalla(){

    int i;
    int aux;

    limpiar();

    printf("====================================\n");
    printf(" ROUND ROBIN + PAGINACION\n");
    printf("====================================\n\n");

    printf("Reloj Global : %d\n",
    relojGlobal);

    printf("Quantum      : %d\n",
    quantum);

    printf("Nuevos       : %d\n",
    contarEstado(NUEVO));

    printf("Marcos Libres: %d / 44\n",
    marcosLibres());

    /* LISTOS */

    printf("\n------ LISTOS ------\n\n");

    aux=frente;

    while(aux!=fin){

        int idx=cola[aux];

        printf("P%d TR:%d PAG:%d\n",
        p[idx].id,
        p[idx].tr,
        p[idx].paginas);

        aux=(aux+1)%MAX_PROCESOS;
    }

    /* EJECUCION */

    printf("\n------ EJECUCION ------\n\n");

    if(ejec!=-1){

        printf("P%d\n",
        p[ejec].id);

        printf("%d %c %d\n",
        p[ejec].a,
        p[ejec].op,
        p[ejec].b);

        printf("TR: %d\n",
        p[ejec].tr);

        printf("Q : %d / %d\n",
        quantumActual,
        quantum);
    }

    else{

        printf("CPU LIBRE\n");
    }

    /* BLOQUEADOS */

    printf("\n------ BLOQUEADOS ------\n\n");

    for(i=0;i<totalProcesos;i++){

        if(p[i].estado==BLOQUEADO){

            printf("P%d TB:%d\n",
            p[i].id,
            BLOQUEO-p[i].tbloq);
        }
    }

    mostrarTerminados();

    /* MEMORIA */

    printf("\n------ MEMORIA ------\n\n");

    for(i=0;i<TOTAL_MARCOS;i++){

        if(i>=44)
            printf("[SO]");

        else if(memoria[i].libre)
            printf("[  ]");

        else
            printf("[P%d]",
            memoria[i].procesoID);

        if((i+1)%12==0)
            printf("\n");
    }

    printf("\n\n-----------------------------\n");

    printf("I=Interrupcion  ");

    printf("E=Error  ");

    printf("P=Pausa  ");

    printf("N=Nuevo\n");

    printf("B=BCP  ");

    printf("T=Paginas  ");

    printf("C=Continuar\n");

    printf("-----------------------------\n");
}

/* ========================================================= */

void pausaPrograma(){

    while(1){

        if(kbhit()){

            char t=toupper(getch());

            if(t=='C')
                break;

            if(t=='B')
                mostrarBCP();

            if(t=='T')
                mostrarPaginas();
        }

        Sleep(100);
    }
}

/* ========================================================= */
/* MAIN */
/* ========================================================= */

int main(){

    int i;

    int siguienteID=1;

    srand(time(NULL));

    inicializarMemoria();

    printf("Numero de procesos inicial: ");
    scanf("%d",&totalProcesos);

    printf("Valor del Quantum: ");
    scanf("%d",&quantum);

    for(i=0;i<totalProcesos;i++){

        generarProceso(i,siguienteID++);

    }

    while(getchar()!='\n');

    admitirNuevos();

    asignarCPU();

    while(procesosTerminados < totalProcesos){

        mostrarPantalla();

        if(kbhit()){

            char t=toupper(getch());

            /* BCP */

            if(t=='B'){

                mostrarBCP();

                continue;
            }

            /* TABLA PAGINAS */

            if(t=='T'){

                mostrarPaginas();

                continue;
            }

            /* PAUSA */

            if(t=='P'){

                pausaPrograma();

                continue;
            }

            /* NUEVO */

            if(t=='N'){

                generarProceso(
                totalProcesos,
                siguienteID++);

                totalProcesos++;

                admitirNuevos();

                asignarCPU();

                continue;
            }

            if(ejec!=-1){

                /* INTERRUPCION */

                if(t=='I'){

                    p[ejec].estado=BLOQUEADO;

                    p[ejec].tbloq=0;

                    ejec=-1;

                    quantumActual=0;

                    asignarCPU();

                    continue;
                }

                /* ERROR */

                if(t=='E'){

                    p[ejec].estado=ERROR_ESTADO;

                    p[ejec].finalizacion=
                    relojGlobal;

                    liberarMemoria(ejec);

                    procesosTerminados++;

                    ejec=-1;

                    quantumActual=0;

                    admitirNuevos();

                    asignarCPU();

                    continue;
                }
            }
        }

        Sleep(1000);

        relojGlobal++;

        actualizarBloqueados();

        if(ejec!=-1){

            p[ejec].tt++;

            p[ejec].servicio++;

            p[ejec].tr--;

            quantumActual++;

            /* TERMINADO */

            if(p[ejec].tr<=0){

                p[ejec].estado=TERMINADO;

                p[ejec].resultado=
                resolver(
                p[ejec].op,
                p[ejec].a,
                p[ejec].b);

                p[ejec].finalizacion=
                relojGlobal;

                liberarMemoria(ejec);

                procesosTerminados++;

                ejec=-1;

                quantumActual=0;
            }

            /* FIN QUANTUM */

            else if(quantumActual>=quantum){

                p[ejec].estado=LISTO;

                encolar(ejec);

                ejec=-1;

                quantumActual=0;
            }
        }

        admitirNuevos();

        asignarCPU();
    }

    limpiar();

    printf("====================================\n");
    printf(" TODOS LOS PROCESOS HAN TERMINADO\n");
    printf("====================================\n\n");

    printf("Tiempo Global Final: %d\n\n",
    relojGlobal);

    mostrarBCP();

    printf("\nPresione una tecla para salir...");

    getch();

    return 0;
}
