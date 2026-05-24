#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <windows.h>
#include <conio.h>
#include <time.h>
#include <ctype.h>

#define MAX_PROCESOS 100
#define TAM_MEMORIA 5
#define TIEMPO_BLOQUEADO 8

typedef struct{

    int id;

    char operacion;
    double dato1;
    double dato2;

    int tiempoMax;
    int tiempoTrans;

    double resultado;

    int estado; //0 nuevo 1 listo 2 ejecucion 3 bloqueado 4 terminado 5 error

    int llegada;
    int finalizacion;
    int retorno;
    int respuesta;
    int espera;
    int servicio;

    int respondido;

    int tiempoBloq;

}Proceso;

Proceso procesos[MAX_PROCESOS];
Proceso terminados[MAX_PROCESOS];

/* COLA FIFO */
int cola[MAX_PROCESOS];
int frente=0, fin=0;

void encolar(int x){
    cola[fin]=x;
    fin=(fin+1)%MAX_PROCESOS;
}

int desencolar(){
    int x=cola[frente];
    frente=(frente+1)%MAX_PROCESOS;
    return x;
}

int colaVacia(){
    return frente==fin;
}

/* VARIABLES */
int totalProcesos;
int totalTerminados=0;
int relojGlobal=0;
int pausa=0;
int enMemoria=0;

int quantum;
int qContador=0;
int ejec=-1;

/* FUNCIONES */

char generarOperacion(){
    char ops[]={'+','-','*','/','%','^'};
    return ops[rand()%6];
}

double generarDato(){
    return rand()%100+1;
}

int generarTiempo(){
    return rand()%15+6;
}

double calcularResultado(Proceso p){
    switch(p.operacion){
        case '+': return p.dato1+p.dato2;
        case '-': return p.dato1-p.dato2;
        case '*': return p.dato1*p.dato2;
        case '/': return p.dato1/p.dato2;
        case '%': return fmod(p.dato1,p.dato2);
        case '^': return pow(p.dato1,p.dato2);
    }
    return 0;
}

void mostrarTerminados(){

    printf("\n========== TERMINADOS ==========\n");

    for(int i=0;i<totalTerminados;i++){

        printf("\nID:%d | %.0lf %c %.0lf | ",
        terminados[i].id,
        terminados[i].dato1,
        terminados[i].operacion,
        terminados[i].dato2);

        if(terminados[i].estado==5)
            printf("ERROR");
        else
            printf("Resultado: %.2lf",terminados[i].resultado);

        printf("\nTL:%d TF:%d TR:%d TRES:%d TES:%d TS:%d\n",
        terminados[i].llegada,
        terminados[i].finalizacion,
        terminados[i].retorno,
        terminados[i].respuesta,
        terminados[i].espera,
        terminados[i].servicio);
    }
}

void mostrarBCP(){

    printf("\n=========== TABLA DE PROCESOS ===========\n");

    for(int i=0;i<totalProcesos;i++){

        Proceso p = procesos[i];

        printf("\nID:%d ", p.id);

        switch(p.estado){
            case 0: printf("NUEVO"); break;
            case 1: printf("LISTO"); break;
            case 2: printf("EJECUCION"); break;
            case 3: printf("BLOQUEADO (%d)", TIEMPO_BLOQUEADO - p.tiempoBloq); break;
            case 4: printf("TERMINADO"); break;
            case 5: printf("ERROR"); break;
        }

        printf("\nOperacion: %.0lf %c %.0lf",
        p.dato1, p.operacion, p.dato2);

        if(p.estado==4)
            printf(" = %.2lf", p.resultado);

        printf("\nTL:%d TF:%d TR:%d",
        p.llegada,
        p.finalizacion,
        p.retorno);

        printf("\nTE:%d TS:%d",
        p.espera,
        p.servicio);

        if(p.estado!=4 && p.estado!=5){
            printf("\nTR CPU:%d",
            p.tiempoMax - p.tiempoTrans);
        }

        printf("\nTRES:%d\n", p.respuesta);
    }
}

void mostrarTeclas(){

    printf("\n========= TECLAS =========\n");
    printf("I = Interrupcion\n");
    printf("E = Error\n");
    printf("P = Pausa\n");
    printf("C = Continuar\n");
    printf("N = Nuevo Proceso\n");
    printf("B = Ver BCP\n");
}

/* MAIN */

int main(){

    srand(time(NULL));

    printf("Numero de procesos: ");
    scanf("%d",&totalProcesos);

    printf("Valor del Quantum: ");
    scanf("%d",&quantum);

    for(int i=0;i<totalProcesos;i++){

        procesos[i].id=i+1;
        procesos[i].operacion=generarOperacion();
        procesos[i].dato1=generarDato();

        do{
            procesos[i].dato2=generarDato();
        }while((procesos[i].operacion=='/'||procesos[i].operacion=='%')
        && procesos[i].dato2==0);

        procesos[i].tiempoMax=generarTiempo();
        procesos[i].tiempoTrans=0;
        procesos[i].estado=0;
        procesos[i].respondido=0;
        procesos[i].tiempoBloq=0;
        procesos[i].espera=0;
        procesos[i].servicio=0;
        procesos[i].llegada=0;
        procesos[i].finalizacion=0;
        procesos[i].retorno=0;
        procesos[i].respuesta=0;
    }

    while(totalTerminados < totalProcesos){

        system("cls");
        printf("RELOJ GLOBAL: %d\n",relojGlobal);
        printf("QUANTUM: %d\n",quantum);

        /* ADMITIR */
        for(int i=0;i<totalProcesos;i++){
            if(enMemoria < TAM_MEMORIA && procesos[i].estado==0){
                procesos[i].estado=1;
                procesos[i].llegada=relojGlobal;
                enMemoria++;
                encolar(i);
            }
        }

        /* SELECCION RR */
        if(ejec==-1 && !colaVacia()){
            ejec = desencolar();
            procesos[ejec].estado=2;
            qContador=0;
        }

        /* NUEVOS */
        int nuevos=0;
        for(int i=0;i<totalProcesos;i++)
            if(procesos[i].estado==0) nuevos++;

        printf("\nNUEVOS: %d\n",nuevos);

        /* LISTOS */
        printf("\n====== COLA LISTOS ======\n");
        int aux=frente;
        while(aux!=fin){
            int i=cola[aux];
            printf("ID:%d TME:%d TT:%d TR:%d\n",
            procesos[i].id,
            procesos[i].tiempoMax,
            procesos[i].tiempoTrans,
            procesos[i].tiempoMax - procesos[i].tiempoTrans);
            aux=(aux+1)%MAX_PROCESOS;
        }

        /* BLOQUEADOS */
        printf("\n====== BLOQUEADOS ======\n");
        for(int i=0;i<totalProcesos;i++){
            if(procesos[i].estado==3){
                printf("ID:%d TRB:%d\n",
                procesos[i].id,
                TIEMPO_BLOQUEADO - procesos[i].tiempoBloq);
            }
        }

        /* EJECUCION */
        if(ejec!=-1){

            Proceso *p=&procesos[ejec];

            if(!p->respondido){
                p->respuesta=relojGlobal-p->llegada;
                p->respondido=1;
            }

            printf("\n====== EJECUCION ======\n");
            printf("ID:%d\n",p->id);
            printf("Operacion: %.0lf %c %.0lf\n",
            p->dato1,p->operacion,p->dato2);
            printf("TME:%d\n",p->tiempoMax);
            printf("TT:%d\n",p->tiempoTrans);
            printf("TR:%d\n",p->tiempoMax-p->tiempoTrans);
            printf("TQ:%d\n",qContador);

            mostrarTerminados();
            mostrarTeclas();

            if(kbhit()){
                char tecla=toupper(getch());

                if(tecla=='I'){
                    p->estado=3;
                    p->tiempoBloq=0;
                    ejec=-1;
                }
                else if(tecla=='E'){
                    p->estado=5;
                    p->finalizacion=relojGlobal+1;
                    p->servicio=p->tiempoTrans;
                    p->retorno=p->finalizacion-p->llegada;
                    p->espera=p->retorno-p->servicio;
                    terminados[totalTerminados++]=*p;
                    enMemoria--;
                    ejec=-1;
                }
                else if(tecla=='P'){
                    pausa=1;
                }
                else if(tecla=='N'){
                    if(totalProcesos < MAX_PROCESOS){

                        Proceso *np = &procesos[totalProcesos];

                        np->id = totalProcesos+1;
                        np->operacion=generarOperacion();
                        np->dato1=generarDato();

                        do{
                            np->dato2=generarDato();
                        }while((np->operacion=='/'||np->operacion=='%')
                        && np->dato2==0);

                        np->tiempoMax=generarTiempo();
                        np->tiempoTrans=0;
                        np->estado=0;
                        np->respondido=0;
                        np->tiempoBloq=0;
                        np->espera=0;
                        np->servicio=0;
                        np->llegada=0;
                        np->finalizacion=0;
                        np->retorno=0;
                        np->respuesta=0;

                        totalProcesos++;
                    }
                }
                else if(tecla=='B'){
                    pausa=1;
                    system("cls");
                    mostrarBCP();
                }
            }

            while(pausa){
                if(kbhit()){
                    char t=toupper(getch());
                    if(t=='C') pausa=0;
                }
            }

            Sleep(1000);

            if(p->estado==2){

                p->tiempoTrans++;
                p->servicio++;
                qContador++;

                if(p->tiempoTrans>=p->tiempoMax){

                    p->estado=4;
                    p->resultado=calcularResultado(*p);

                    /* CORRECCION TIEMPOS */
                    p->finalizacion=relojGlobal+1;
                    p->retorno=p->finalizacion-p->llegada;
                    p->espera=p->retorno-p->servicio;

                    terminados[totalTerminados++]=*p;
                    enMemoria--;
                    ejec=-1;
                }
                else if(qContador>=quantum){

                    p->estado=1;
                    encolar(ejec);
                    ejec=-1;
                }
            }
        }

        /* BLOQUEADOS */
        for(int i=0;i<totalProcesos;i++){
            if(procesos[i].estado==3){
                procesos[i].tiempoBloq++;

                if(procesos[i].tiempoBloq>=TIEMPO_BLOQUEADO){
                    procesos[i].estado=1;
                    encolar(i);
                }
            }
        }

        /* ESPERA */
        for(int i=0;i<totalProcesos;i++){
            if(procesos[i].estado==1){
                procesos[i].espera++;
            }
        }

        relojGlobal++;
    }

    system("cls");

    printf("\n==== TODOS LOS PROCESOS TERMINARON ====\n");
    printf("Tiempo Global Final: %d\n",relojGlobal);

    mostrarTerminados();
    mostrarBCP();

    system("pause");
    return 0;
}
