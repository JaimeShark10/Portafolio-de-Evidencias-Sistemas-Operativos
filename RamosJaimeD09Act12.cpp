#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <conio.h>
#include <time.h>

#define TAM 18

/* =========================
   VARIABLES GLOBALES
========================= */

char buffer[TAM];

int in = 0;
int out = 0;
int elementos = 0;
int ejecutar = 1;

/* mutex */
HANDLE mutexBuffer;
HANDLE mutexPantalla;

/* estados */
char estadoProd[100] = "DORMIDO";
char estadoCons[100] = "DORMIDO";
char ultimoEvento[150] = "Programa iniciado.";

/* =========================
   UTILIDADES
========================= */

void gotoxy(int x, int y)
{
    COORD c;
    c.X = x;
    c.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), c);
}

void color(int c)
{
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), c);
}

void ocultarCursor()
{
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 100;
    info.bVisible = FALSE;
    SetConsoleCursorInfo(h, &info);
}

char generarProducto()
{
    char lista[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ123456789@#$%&*";
    return lista[rand() % (sizeof(lista)-1)];
}

/* =========================
   PANTALLA
========================= */

void mostrarPantalla()
{
    WaitForSingleObject(mutexPantalla, INFINITE);

    gotoxy(0,0);

    color(11);
    printf("=====================================================\n");
    printf("      PRODUCTOR - CONSUMIDOR (BUFFER CIRCULAR)\n");
    printf("=====================================================\n\n");

    color(15);
    printf("BUFFER:\n\n");

    for(int i=0;i<TAM;i++)
    {
        if(i == in && elementos < TAM)
            color(10);
        else if(i == out && elementos > 0)
            color(12);
        else
            color(15);

        printf("[%02d:%c] ", i+1, buffer[i]);

        if((i+1)%6==0)
            printf("\n");
    }

    color(15);
    printf("\n-----------------------------------------------------\n");

    color(10);
    printf("Productor : %-30s\n", estadoProd);

    color(12);
    printf("Consumidor: %-30s\n", estadoCons);

    color(14);
    printf("\nUltimo evento: %-45s\n", ultimoEvento);

    color(11);
    printf("Elementos: %-3d / %d	\n", elementos, TAM);

    color(15);
    printf("\nESC = Salir\n");

    ReleaseMutex(mutexPantalla);
}

/* =========================
   PRODUCTOR
========================= */

DWORD WINAPI productor(LPVOID lpParam)
{
    while(ejecutar)
    {
        sprintf(estadoProd, "DORMIDO");
        mostrarPantalla();

        Sleep(rand()%3000 + 1000);

        if(!ejecutar) break;

        int cantidad = rand()%4 + 3;

        sprintf(estadoProd, "QUIERE PRODUCIR %d", cantidad);
        mostrarPantalla();

        /* Esperar turno + espacio suficiente */
        while(ejecutar)
        {
            if(WaitForSingleObject(mutexBuffer, 500)==WAIT_TIMEOUT)
            {
                sprintf(estadoProd, "ESPERANDO TURNO");
                mostrarPantalla();
                continue;
            }

            if(TAM - elementos >= cantidad)
                break;

            sprintf(estadoProd, "ESPERANDO ESPACIO");
            sprintf(ultimoEvento, "Productor esperando espacio suficiente...");
            ReleaseMutex(mutexBuffer);
            mostrarPantalla();

            Sleep(500);
        }

        sprintf(estadoProd, "PRODUCIENDO %d", cantidad);
        sprintf(estadoCons, "ESPERANDO TURNO");
        mostrarPantalla();

        for(int i=0;i<cantidad && ejecutar;i++)
        {
            char p = generarProducto();

            buffer[in] = p;

            sprintf(ultimoEvento,
                    "Productor ingreso %c en casilla %02d",
                    p, in+1);

            in = (in + 1) % TAM;
            elementos++;

            mostrarPantalla();
            Sleep(500);
        }

        ReleaseMutex(mutexBuffer);
    }

    return 0;
}

/* =========================
   CONSUMIDOR
========================= */

DWORD WINAPI consumidor(LPVOID lpParam)
{
    while(ejecutar)
    {
        sprintf(estadoCons, "DORMIDO");
        mostrarPantalla();

        Sleep(rand()%3000 + 1000);

        if(!ejecutar) break;

        int cantidad = rand()%4 + 3;

        sprintf(estadoCons, "QUIERE CONSUMIR %d", cantidad);
        mostrarPantalla();

        /* Esperar turno + productos suficientes */
        while(ejecutar)
        {
            if(WaitForSingleObject(mutexBuffer, 500)==WAIT_TIMEOUT)
            {
                sprintf(estadoCons, "ESPERANDO TURNO");
                mostrarPantalla();
                continue;
            }

            if(elementos >= cantidad)
                break;

            sprintf(estadoCons, "ESPERANDO PRODUCTO");
            sprintf(ultimoEvento,
                    "Consumidor esperando productos suficientes...");
            ReleaseMutex(mutexBuffer);
            mostrarPantalla();

            Sleep(500);
        }

        sprintf(estadoCons, "CONSUMIENDO %d", cantidad);
        sprintf(estadoProd, "ESPERANDO TURNO");
        mostrarPantalla();

        for(int i=0;i<cantidad && ejecutar;i++)
        {
            char dato = buffer[out];
            buffer[out] = '-';

            sprintf(ultimoEvento,
                    "Consumidor retiro %c de casilla %02d",
                    dato, out+1);

            out = (out + 1) % TAM;
            elementos--;

            mostrarPantalla();
            Sleep(500);
        }

        ReleaseMutex(mutexBuffer);
    }

    return 0;
}

/* =========================
   MAIN
========================= */

int main()
{
    srand(time(NULL));

    for(int i=0;i<TAM;i++)
        buffer[i]='-';

    mutexBuffer   = CreateMutex(NULL,FALSE,NULL);
    mutexPantalla = CreateMutex(NULL,FALSE,NULL);

    system("cls");
    ocultarCursor();

    HANDLE hProd = CreateThread(NULL,0,productor,NULL,0,NULL);
    HANDLE hCons = CreateThread(NULL,0,consumidor,NULL,0,NULL);

    while(ejecutar)
    {
        if(kbhit())
        {
            if(getch()==27)
                ejecutar = 0;
        }

        Sleep(100);
    }

    WaitForSingleObject(hProd, INFINITE);
    WaitForSingleObject(hCons, INFINITE);

    CloseHandle(hProd);
    CloseHandle(hCons);
    CloseHandle(mutexBuffer);
    CloseHandle(mutexPantalla);

    gotoxy(0,22);
    color(15);
    printf("Programa terminado.\n");

    return 0;
}
