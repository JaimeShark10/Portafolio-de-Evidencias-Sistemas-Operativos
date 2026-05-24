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

#define BLOQUEO 8

/* ESTADOS */

#define NUEVO 0
#define LISTO 1
#define EJECUCION 2
#define BLOQUEADO 3
#define TERMINADO 4
#define ERROR_ESTADO 5
#define SUSPENDIDO 6


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
/* VARIABLES */
/* ========================================================= */

Proceso p[MAX_PROCESOS];
Proceso siguienteNuevo;

int existeSiguiente=0;
Marco memoria[TOTAL_MARCOS];

/* COLA LISTOS */

int cola[MAX_PROCESOS];

int frente=0;
int fin=0;

/* COLA SUSPENDIDOS */

int suspendidos[MAX_PROCESOS];

int frenteSusp=0;
int finSusp=0;

/* CONTROL */

int totalProcesos=0;

int procesosTerminados=0;

int relojGlobal=0;

int quantum=0;
int quantumActual=0;

int ejec=-1;

/* ========================================================= */
/* FUNCIONES GENERALES */
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

    /* SO ocupa 44-47 */

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
/* COLA LISTOS */
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
/* COLA SUSPENDIDOS */
/* ========================================================= */

int colaSuspVacia(){

    return frenteSusp==finSusp;
}

/* ========================================================= */

void encolarSusp(int x){

    suspendidos[finSusp]=x;

    finSusp=(finSusp+1)%MAX_PROCESOS;
}

/* ========================================================= */

int desencolarSusp(){

    int x=suspendidos[frenteSusp];

    frenteSusp=(frenteSusp+1)%MAX_PROCESOS;

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

void prepararSiguienteNuevo(int id){

    siguienteNuevo.id=id;

    siguienteNuevo.op=opRandom();

    siguienteNuevo.a=aleatorio(1,50);

    do{

        siguienteNuevo.b=aleatorio(1,10);

    }while(
    (siguienteNuevo.op=='/' ||
     siguienteNuevo.op=='%')
     &&
     siguienteNuevo.b==0);

    if(siguienteNuevo.op=='^')
        siguienteNuevo.b=aleatorio(1,5);

    siguienteNuevo.tme=aleatorio(6,15);

    siguienteNuevo.tamanio=
    aleatorio(6,30);

    siguienteNuevo.paginas=
    (siguienteNuevo.tamanio +
    TAM_MARCO -1)/TAM_MARCO;

    existeSiguiente=1;
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
/* SUSPENDIDOS */
/* ========================================================= */

void guardarSuspendidos(){

    FILE *f;

    int i;
    int aux;

   f=fopen("suspendidos.txt","w");

    if(f==NULL){
        printf("Error al crear el archivo\n");
        return;
    }

    fprintf(f,
    "=========== PROCESOS SUSPENDIDOS ===========\n\n");

    aux=frenteSusp;

    if(aux==finSusp){

        fprintf(f,"No hay procesos suspendidos\n");
    }

    while(aux!=finSusp){

        i=suspendidos[aux];

        fprintf(f,
        "ID: %d\n"
        "Operacion: %d %c %d\n"
        "Tamano: %d\n"
        "Paginas: %d\n"
        "Tiempo restante: %d\n"
        "----------------------------------\n",
        p[i].id,
        p[i].a,
        p[i].op,
        p[i].b,
        p[i].tamanio,
        p[i].paginas,
        p[i].tr);

        aux=(aux+1)%MAX_PROCESOS;
    }

    fclose(f);

    /* ABRIR ARCHIVO AUTOMATICAMENTE */

    
}



void suspenderProceso(){

    int i;

    for(i=0;i<totalProcesos;i++){

        if(p[i].estado==BLOQUEADO){

            p[i].estado=SUSPENDIDO;

            liberarMemoria(i);

            encolarSusp(i);

            guardarSuspendidos();

            return;
        }
    }
}

/* ========================================================= */

void regresarSuspendido(){

    int idx;

    if(colaSuspVacia())
        return;

    idx=suspendidos[frenteSusp];

    if(marcosLibres() >= p[idx].paginas){

        desencolarSusp();

        asignarMemoria(idx);

        p[idx].estado=LISTO;

        p[idx].tbloq=0;

        encolar(idx);

        guardarSuspendidos();
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
/* TABLA PAGINAS */
/* ========================================================= */

void mostrarPaginas(){

    int i,j;

    limpiar();

    printf("============= TABLA DE PAGINAS =============\n\n");

    for(i=0;i<totalProcesos;i++){

        if(p[i].estado!=NUEVO &&
           p[i].estado!=SUSPENDIDO){

            printf("=====================================\n");

            printf("Proceso : P%d\n",
            p[i].id);

            printf("Estado  : ");

            switch(p[i].estado){

                case LISTO:
                    printf("LISTO");
                    break;

                case EJECUCION:
                    printf("EJECUCION");
                    break;

                case BLOQUEADO:
                    printf("BLOQUEADO");
                    break;

                case TERMINADO:
                    printf("TERMINADO");
                    break;

                case ERROR_ESTADO:
                    printf("ERROR");
                    break;
            }

            printf("\n");

            printf("Operacion : %d %c %d\n",
            p[i].a,
            p[i].op,
            p[i].b);

            printf("TME       : %d\n",
            p[i].tme);

            printf("TT        : %d\n",
            p[i].tt);

            printf("TR        : %d\n",
            p[i].tr);

            printf("Tamano    : %d\n",
            p[i].tamanio);

            printf("Paginas   : %d\n",
            p[i].paginas);

            printf("-------------------------------------\n");

            printf("Pagina -> Marco\n\n");

            for(j=0;j<p[i].paginas;j++){

                printf("%3d -> %3d\n",
                j,
                p[i].marcos[j]);
            }

            printf("\n");
        }
    }

    /* ===================================================== */
    /* SUSPENDIDOS */
    /* ===================================================== */

    printf("=========== SUSPENDIDOS ===========\n\n");

    if(colaSuspVacia())
        printf("Sin suspendidos\n");

    else{

        int aux=frenteSusp;

        while(aux!=finSusp){

            int idx=suspendidos[aux];

            printf("P%d | Tam:%d | Pag:%d | TR:%d\n",
            p[idx].id,
            p[idx].tamanio,
            p[idx].paginas,
            p[idx].tr);

            aux=(aux+1)%MAX_PROCESOS;
        }
    }

    /* ===================================================== */
    /* MARCOS */
    /* ===================================================== */

    printf("\n=========== MARCOS ===========\n\n");

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

                case SUSPENDIDO:
                    printf("%-12s","SUSPENDIDO");
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
/* PANTALLA */
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

    printf("Suspendidos  : %d\n",
    (finSusp-frenteSusp+MAX_PROCESOS)%MAX_PROCESOS);

    /* ===================================================== */
    /* PROXIMO NUEVO */
    /* ===================================================== */

    if(existeSiguiente){

        printf("\nPROXIMO NUEVO:\n");

        printf("ID      : P%d\n",
        siguienteNuevo.id);

        printf("Tamano  : %d\n",
        siguienteNuevo.tamanio);

        printf("Paginas : %d\n",
        siguienteNuevo.paginas);

        printf("Marcos  : %d\n",
        siguienteNuevo.paginas);

        if(siguienteNuevo.paginas <= marcosLibres())
            printf("Estado  : CABE EN MEMORIA\n");

        else
            printf("Estado  : NO CABE EN MEMORIA\n");
    }

    /* ===================================================== */

    if(!colaSuspVacia()){

        int idx=suspendidos[frenteSusp];

        printf("\nRegresa -> P%d Tam:%d Pag:%d\n",
        p[idx].id,
        p[idx].tamanio,
        p[idx].paginas);
    }

    /* ===================================================== */
    /* LISTOS */
    /* ===================================================== */

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

    /* ===================================================== */
    /* EJECUCION */
    /* ===================================================== */

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

    /* ===================================================== */
    /* BLOQUEADOS */
    /* ===================================================== */

    printf("\n------ BLOQUEADOS ------\n\n");

    for(i=0;i<totalProcesos;i++){

        if(p[i].estado==BLOQUEADO){

            printf("P%d TB:%d\n",
            p[i].id,
            BLOQUEO-p[i].tbloq);
        }
    }

    /* ===================================================== */
    /* SUSPENDIDOS */
    /* ===================================================== */

    printf("\n------ SUSPENDIDOS ------\n\n");

    if(colaSuspVacia())
        printf("Sin suspendidos\n");

    else{

        aux=frenteSusp;

        while(aux!=finSusp){

            int idx=suspendidos[aux];

            printf("P%d TR:%d PAG:%d\n",
            p[idx].id,
            p[idx].tr,
            p[idx].paginas);

            aux=(aux+1)%MAX_PROCESOS;
        }
    }

    mostrarTerminados();

    /* ===================================================== */
    /* MEMORIA */
    /* ===================================================== */

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

    printf("\n\n-----------------------------------------\n");

    printf("I=Interrupcion  E=Error  P=Pausa\n");

    printf("N=Nuevo         B=BCP\n");

    printf("T=Paginas       S=Suspender\n");

    printf("R=Regresar      C=Continuar\n");

    printf("-----------------------------------------\n");
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
    prepararSiguienteNuevo(siguienteID);

    while(getchar()!='\n');

    admitirNuevos();

    asignarCPU();

    while(procesosTerminados < totalProcesos){

        mostrarPantalla();
			
		if(
			ejec==-1 &&
			colaVacia() &&
			contarEstado(BLOQUEADO)==0 &&
			contarEstado(NUEVO)==0 &&
			!colaSuspVacia()
		){
		
		    printf("\n");
		    printf("====================================\n");
		    printf(" HAY PROCESOS SUSPENDIDOS\n");
		    printf(" PRESIONE R PARA REGRESARLOS\n");
		    printf("====================================\n");
		}
				
		
        if(kbhit()){

            char t=toupper(getch());

            if(t=='B'){

                mostrarBCP();
                continue;
            }

            if(t=='T'){

                mostrarPaginas();
                continue;
            }

            if(t=='P'){

                pausaPrograma();
                continue;
            }

			if(t=='N'){
			
			    p[totalProcesos]=siguienteNuevo;
			
			    p[totalProcesos].estado=NUEVO;
			
			    p[totalProcesos].tt=0;
			
			    p[totalProcesos].tr=
			    p[totalProcesos].tme;
			
			    p[totalProcesos].resultado=0;
			
			    p[totalProcesos].llegada=-1;
			
			    p[totalProcesos].finalizacion=-1;
			
			    p[totalProcesos].retorno=0;
			
			    p[totalProcesos].respuesta=-1;
			
			    p[totalProcesos].espera=0;
			
			    p[totalProcesos].servicio=0;
			
			    p[totalProcesos].respondido=0;
			
			    p[totalProcesos].tbloq=0;
			
			    totalProcesos++;
			
			    siguienteID++;
			
			    prepararSiguienteNuevo(siguienteID);
			
			    admitirNuevos();
			
			    asignarCPU();
			
			    continue;
			}

            if(t=='S'){

                suspenderProceso();

                admitirNuevos();

                asignarCPU();

                continue;
            }

            if(t=='R'){

                regresarSuspendido();

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
