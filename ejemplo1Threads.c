#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#define NUM_THREADS 5


void *hola(void *identidad)
{
    /*
        Funcion que ejecutara el hilo, acepta un entero
        como parametro y solo muestra un mensaje
    */
    printf("\nHola, desde la hebra %d\n", identidad);
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    //array con identificadores de hilos
    pthread_t hebras[NUM_THREADS];

    int rc, t;
    //bucle para crear 5 hilos
    for(t=0;t<NUM_THREADS;t++){
        printf("Creando la hebra %d\n", t);
        //con esto se crea el hilo y arranca
        //lo primero que acepta es el identificador,
        //lo segundo atributos == NULL
        //lo tercero la funcion
        //lo cuarto los argumentos con un cast a void *
        rc = pthread_create(&hebras[t], NULL, hola, (void *)t);

        if (rc){
            //si ha habido error
            printf("ERROR; fallo al crear la hebra, retorno: %d\n", rc);
            exit(-1);
        }
    }

    pthread_exit(NULL);
}