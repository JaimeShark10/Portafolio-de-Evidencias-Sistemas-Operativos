#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <windows.h>
#include <conio.h>
#include <time.h>

#define MAX_PROCESOS 100
#define TAM_LOTE 5


typedef struct{

int id;
char nombre[50];

char operacion;
double dato1;
double dato2;

int tiempoMax;
int tiempoTrans;

double resultado;

int estado;

}Proceso;



Proceso procesos[MAX_PROCESOS];
Proceso terminados[MAX_PROCESOS];

int totalProcesos;
int totalTerminados=0;

int relojGlobal=0;

int pausa=0;



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

if(totalTerminados==0){

printf("\n");
return;

}

int loteActual=-1;

for(int i=0;i<totalTerminados;i++){

int lote=i/TAM_LOTE;

if(lote!=loteActual){

loteActual=lote;

printf("\nLOTE %d:\n",loteActual+1);

}

printf("ID:%d | %.0lf %c %.0lf | ",

terminados[i].id,
terminados[i].dato1,
terminados[i].operacion,
terminados[i].dato2);

if(terminados[i].estado==4)

printf("ERROR\n");

else

printf("Resultado: %.2lf\n",terminados[i].resultado);

}

}



void mostrarTeclas(){

printf("\n============== TECLAS ==============\n");

printf("I = Interrupcion\n");
printf("E = Error\n");
printf("P = Pausa\n");
printf("C = Continuar\n");

}



/* MAIN */

int main(){

srand(time(NULL));

printf("Numero de procesos: ");
scanf("%d",&totalProcesos);



/* GENERAR AUTOMATICO */

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

}



int totalLotes=(totalProcesos+TAM_LOTE-1)/TAM_LOTE;



/* SIMULACION */

for(int lote=0;lote<totalLotes;lote++){

int inicio=lote*TAM_LOTE;
int fin=inicio+TAM_LOTE;

if(fin>totalProcesos)
fin=totalProcesos;



for(int i=inicio;i<fin;i++){


while(procesos[i].tiempoTrans
< procesos[i].tiempoMax){


system("cls");



printf("CONTADOR GLOBAL: %d\n",relojGlobal);

printf("====================================================\n");



/* LISTA TOTAL */

printf("LISTA TOTAL DE PROCESOS ACTIVOS:\n\n");

for(int j=0;j<totalProcesos;j++){

if(procesos[j].estado!=3 &&
procesos[j].estado!=4){

int loteNum=(j/TAM_LOTE)+1;

printf("ID:%d | %s | %.0lf %c %.0lf | TME:%d | Lote:%d\n",

procesos[j].id,
procesos[j].nombre,
procesos[j].dato1,
procesos[j].operacion,
procesos[j].dato2,
procesos[j].tiempoMax,
loteNum);

}

}



printf("\n====================================================\n");



printf("LOTE EN EJECUCION: Lote %d\n",lote+1);



printf("\nPROCESO EN EJECUCION\n\n");

printf("ID:%d\n",procesos[i].id);

printf("Operacion: %.0lf %c %.0lf\n",

procesos[i].dato1,
procesos[i].operacion,
procesos[i].dato2);

printf("TME:%d\n",procesos[i].tiempoMax);

printf("Transcurrido:%d\n",procesos[i].tiempoTrans);

printf("Restante:%d\n",

procesos[i].tiempoMax-
procesos[i].tiempoTrans);



mostrarTerminados();

mostrarTeclas();



/* TECLAS */

if(kbhit()){

char tecla=toupper(getch());

if(tecla=='I')

break;


if(tecla=='E'){

procesos[i].estado=4;

terminados[totalTerminados++]
=procesos[i];

goto siguiente;

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



/* TERMINADO NORMAL */

procesos[i].estado=3;

procesos[i].resultado=
calcularResultado(procesos[i]);

terminados[totalTerminados++]
=procesos[i];


siguiente:;

}

}



system("cls");

printf("==== TODOS LOS PROCESOS TERMINARON ====\n");

printf("Tiempo Global Final: %d\n",relojGlobal);

mostrarTerminados();

system("pause");

}
