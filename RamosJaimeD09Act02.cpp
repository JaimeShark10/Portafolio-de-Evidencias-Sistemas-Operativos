#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <windows.h>
#define MAX_PROCESOS 100
#define TAM_LOTE 5
#define MAX_LOTES 25

typedef struct {
    char nombre[50];
    char operacion;
    double dato1;
    double dato2;
    int tiempoMax;
    int id;
    double resultado;
    int terminado;   // 0 = pendiente, 1 = terminado
} Proceso;

Proceso procesos[MAX_PROCESOS];
Proceso terminados[MAX_PROCESOS];

int totalProcesos = 0;
int totalTerminados = 0;

/* -------- FUNCIONES -------- */

void limpiarBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int leerEntero(int *var) {
    if (scanf("%d", var) != 1) {
        limpiarBuffer();
        return 0;
    }
    return 1;
}

int leerDouble(double *var) {
    if (scanf("%lf", var) != 1) {
        limpiarBuffer();
        return 0;
    }
    return 1;
}

int idExiste(int id, int limite) {
    for (int i = 0; i < limite; i++)
        if (procesos[i].id == id)
            return 1;
    return 0;
}

int operacionValida(char op) {
    return (op=='+'||op=='-'||op=='*'||op=='/'||op=='%'||op=='^');
}

int validarDivision(char op, double d2) {
    if ((op=='/'||op=='%') && d2==0) return 0;
    return 1;
}

double calcularResultado(Proceso p) {
    switch (p.operacion) {
        case '+': return p.dato1 + p.dato2;
        case '-': return p.dato1 - p.dato2;
        case '*': return p.dato1 * p.dato2;
        case '/': return p.dato1 / p.dato2;
        case '%': return fmod(p.dato1, p.dato2);
        case '^': return pow(p.dato1, p.dato2);
    }
    return 0;
}

void limpiarPantalla() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void mostrarTerminados() {
    printf("\n=========== PROCESOS TERMINADOS ===========\n");
    for (int i = 0; i < totalTerminados; i++) {
        printf("ID:%d | %.2lf %c %.2lf = %.2lf\n",
               terminados[i].id,
               terminados[i].dato1,
               terminados[i].operacion,
               terminados[i].dato2,
               terminados[i].resultado);
    }
}

/* -------- MAIN -------- */

int main() {

    int contadorGlobal = 0;

    printf("Ingrese el numero de procesos: ");
    while (!leerEntero(&totalProcesos) || totalProcesos <= 0 || totalProcesos > MAX_PROCESOS)
        printf("Numero invalido: ");

    /* -------- CAPTURA -------- */
    for (int i = 0; i < totalProcesos; i++) {

        printf("\n==== Proceso %d ====\n", i + 1);

        printf("Nombre: ");
        scanf("%s", procesos[i].nombre);

        do {
            printf("Operacion (+,-,*,/,%%,^): ");
            scanf(" %c", &procesos[i].operacion);

            if (!operacionValida(procesos[i].operacion)) {
                printf("Operacion invalida.\n");
                continue;
            }

            printf("Dato 1: ");
            while (!leerDouble(&procesos[i].dato1))
                printf("Dato invalido: ");

            printf("Dato 2: ");
            while (!leerDouble(&procesos[i].dato2))
                printf("Dato invalido: ");

            if (!validarDivision(procesos[i].operacion, procesos[i].dato2))
                printf("Error: Division entre 0.\n");

        } while (!operacionValida(procesos[i].operacion) ||
                 !validarDivision(procesos[i].operacion, procesos[i].dato2));

        do {
            printf("Tiempo Maximo (>0): ");
        } while (!leerEntero(&procesos[i].tiempoMax) || procesos[i].tiempoMax <= 0);

        int valido;
        do {
            printf("ID unico: ");
            valido = leerEntero(&procesos[i].id);

            if (!valido) {
                printf("Solo numeros.\n");
                continue;
            }

            if (idExiste(procesos[i].id, i)) {
                printf("ID repetido.\n");
                valido = 0;
            }

        } while (!valido);

        procesos[i].terminado = 0;   // ?? Inicializar estado
    }

    /* -------- COLA DINAMICA -------- */

    int totalLotes = (totalProcesos + TAM_LOTE - 1) / TAM_LOTE;
    int colaLotes[MAX_LOTES];

    for (int i = 0; i < totalLotes; i++)
        colaLotes[i] = i;

    int lotesActivos = totalLotes;

    /* -------- SIMULACION -------- */

    while (lotesActivos > 0) {

        int loteReal = colaLotes[0];

        int inicio = loteReal * TAM_LOTE;
        int fin = inicio + TAM_LOTE;
        if (fin > totalProcesos) fin = totalProcesos;

        for (int p = inicio; p < fin; p++) {

            if (procesos[p].terminado) continue;

            Proceso actual = procesos[p];
            actual.resultado = calcularResultado(actual);
            int t = 0;

            while (t < actual.tiempoMax) {

                limpiarPantalla();

                printf("CONTADOR GLOBAL: %d\n", contadorGlobal);
                printf("====================================================\n");

                printf("LISTA TOTAL DE LOTES ACTIVOS:\n\n");

                for (int l = 0; l < lotesActivos; l++) {

                    int loteTemp = colaLotes[l];
                    int ini = loteTemp * TAM_LOTE;
                    int fi = ini + TAM_LOTE;
                    if (fi > totalProcesos) fi = totalProcesos;

                    printf("LOTE %d:\n", loteTemp + 1);

                    for (int i = ini; i < fi; i++) {

                        if (!procesos[i].terminado) {
                            printf("   ID:%d | %s | %.2lf %c %.2lf | TME:%d\n",
                                   procesos[i].id,
                                   procesos[i].nombre,
                                   procesos[i].dato1,
                                   procesos[i].operacion,
                                   procesos[i].dato2,
                                   procesos[i].tiempoMax);
                        }
                    }
                    printf("\n");
                }

                printf("====================================================\n");
                printf("LOTE EN EJECUCION: Lote %d\n", loteReal + 1);

                printf("\nPROCESO EN EJECUCION\n\n");
                printf("ID:%d\n", actual.id);
                printf("Operacion: %.2lf %c %.2lf\n",
                       actual.dato1,
                       actual.operacion,
                       actual.dato2);
                printf("TME:%d\n", actual.tiempoMax);
                printf("Transcurrido:%d\n", t);
                printf("Restante:%d\n", actual.tiempoMax - t);

                mostrarTerminados();

                Sleep(1000);
                contadorGlobal++;
                t++;
            }

            /* ?? Marcar como terminado y mover a lista */
            procesos[p].resultado = actual.resultado;
            procesos[p].terminado = 1;
            terminados[totalTerminados++] = procesos[p];
        }

        /* Eliminar lote terminado */
        for (int i = 0; i < lotesActivos - 1; i++)
            colaLotes[i] = colaLotes[i + 1];

        lotesActivos--;
    }

    limpiarPantalla();
    printf("\n==== TODOS LOS PROCESOS TERMINARON ====\n");
    printf("Tiempo Global Final: %d\n", contadorGlobal);
    mostrarTerminados();

    printf("\nPresione ENTER para salir...");
    limpiarBuffer();
    getchar();

    return 0;
}

