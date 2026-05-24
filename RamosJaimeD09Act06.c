#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <windows.h>
#include <conio.h>
#include <time.h>
#include <ctype.h>

#define MAX_PROCESOS 100
#define TAM_COLA 5

typedef struct{

    int id;
    char nombre[50];

    char operacion;
    double dato1;
    double dato2;

    int tiempoMax;
    int tiempoTrans;

    double resultado;

    int estado; //3 terminado 4 error 5 interrumpido

    int llegada;
    int finalizacion;
    int retorno;
    int respuesta;
    int espera;
    int servicio;

    int respondido;

}Proceso;

Proceso procesos[MAX_PROCESOS];
Proceso terminados[MAX_PROCESOS];

int totalProcesos;
int totalTerminados=0;

int relojGlobal=0;

int pausa=0;

int siguienteProceso;


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

    printf("\n=========== PROCESOS TERMINADOS ===========\n");

    for(int i=0;i<totalTerminados;i++){

        printf("\nID:%d | %.0lf %c %.0lf | ",
        terminados[i].id,
        terminados[i].dato1,
        terminados[i].operacion,
        terminados[i].dato2);

        if(terminados[i].estado==4)
            printf("ERROR");
        else if(terminados[i].estado==5)
            printf("INTERRUMPIDO");
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


void mostrarTeclas(){

    printf("\n============== TECLAS ==============\n");

    printf("I = Interrupcion (termina proceso)\n");
    printf("E = Error\n");
    printf("P = Pausa\n");
    printf("C = Continuar\n");

}


/* MAIN */

int main(){

    srand(time(NULL));

    printf("Numero de procesos: ");
    scanf("%d",&totalProcesos);


    /* GENERAR PROCESOS */

    for(int i=0;i<totalProcesos;i++){

        procesos[i].id=i+1;

        sprintf(procesos[i].nombre,"Proceso%d",i+1);

        procesos[i].operacion=generarOperacion();

        procesos[i].dato1=generarDato();

        do{
            procesos[i].dato2=generarDato();
        }while((procesos[i].operacion=='/'||
        procesos[i].operacion=='%')
        && procesos[i].dato2==0);

        procesos[i].tiempoMax=generarTiempo();

        procesos[i].tiempoTrans=0;

        procesos[i].estado=0;

        procesos[i].respondido=0;

        if(i < TAM_COLA)
            procesos[i].llegada = 0;
        else
            procesos[i].llegada = -1;

    }

    siguienteProceso = TAM_COLA;


    /* SIMULACION */

    for(int i=0;i<totalProcesos;i++){

        if(procesos[i].llegada==-1)
            continue;

        while(procesos[i].tiempoTrans < procesos[i].tiempoMax){

            system("cls");

            printf("CONTADOR GLOBAL: %d\n",relojGlobal);
            printf("====================================================\n");


            printf("COLA DE LISTOS\n\n");

            for(int j=0;j<totalProcesos;j++){

                if(procesos[j].estado==0 && procesos[j].llegada!=-1 && j!=i){

                    printf("ID:%d | %s | TME:%d | TT:%d\n",
                    procesos[j].id,
                    procesos[j].nombre,
                    procesos[j].tiempoMax,
                    procesos[j].tiempoTrans);

                }

            }

            printf("\n====================================================\n");


            printf("\nPROCESO EN EJECUCION\n\n");

            printf("ID:%d\n",procesos[i].id);

            printf("Operacion: %.0lf %c %.0lf\n",
            procesos[i].dato1,
            procesos[i].operacion,
            procesos[i].dato2);

            printf("TME:%d\n",procesos[i].tiempoMax);

            printf("TT:%d\n",procesos[i].tiempoTrans);

            printf("TR:%d\n",
            procesos[i].tiempoMax-procesos[i].tiempoTrans);


            mostrarTerminados();
            mostrarTeclas();


            if(!procesos[i].respondido){

                procesos[i].respuesta=relojGlobal-procesos[i].llegada;
                procesos[i].respondido=1;

            }


            if(kbhit()){

                char tecla=toupper(getch());

                if(tecla=='I'){

                    procesos[i].estado=5;

                    procesos[i].finalizacion=relojGlobal;

                    procesos[i].servicio=procesos[i].tiempoTrans;

                    procesos[i].retorno=
                    procesos[i].finalizacion-procesos[i].llegada;

                    procesos[i].espera=
                    procesos[i].retorno-procesos[i].servicio;

                    terminados[totalTerminados++]=procesos[i];

                    if(siguienteProceso < totalProcesos){

                        procesos[siguienteProceso].llegada = relojGlobal;
                        siguienteProceso++;

                    }

                    break;

                }

                if(tecla=='E'){

                    procesos[i].estado=4;

                    procesos[i].finalizacion=relojGlobal;

                    procesos[i].servicio=procesos[i].tiempoTrans;

                    procesos[i].retorno=
                    procesos[i].finalizacion-procesos[i].llegada;

                    procesos[i].espera=
                    procesos[i].retorno-procesos[i].servicio;

                    terminados[totalTerminados++]=procesos[i];

                    if(siguienteProceso < totalProcesos){

                        procesos[siguienteProceso].llegada = relojGlobal;
                        siguienteProceso++;

                    }

                    break;

                }

                if(tecla=='P')
                    pausa=1;

            }

            while(pausa){

                if(kbhit()){

                    char t=toupper(getch());

                    if(t=='C')
                        pausa=0;

                }

            }

            Sleep(1000);

            procesos[i].tiempoTrans++;
            relojGlobal++;

        }


        if(procesos[i].estado==0){

            procesos[i].estado=3;

            procesos[i].resultado=calcularResultado(procesos[i]);

            procesos[i].finalizacion=relojGlobal;

            procesos[i].servicio=procesos[i].tiempoMax;

            procesos[i].retorno=
            procesos[i].finalizacion-procesos[i].llegada;

            procesos[i].espera=
            procesos[i].retorno-procesos[i].servicio;

            terminados[totalTerminados++]=procesos[i];

            if(siguienteProceso < totalProcesos){

                procesos[siguienteProceso].llegada = relojGlobal;
                siguienteProceso++;

            }

        }

    }


    system("cls");

    printf("==== TODOS LOS PROCESOS TERMINARON ====\n");

    printf("Tiempo Global Final: %d\n",relojGlobal);

    mostrarTerminados();

    system("pause");

    }
