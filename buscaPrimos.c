/*
    Programa ejemplo que usa mutex, usa el paradigma de grupo de trabajos, variables de estado
    predicados,estados de cancelacion.
    Hace el calculo de numeros primos hasta cierta cantidad
*/

#include <pthread.h> //para uso de hilos 
#include <stdio.h> //estandar input output
#include <stdlib.h>
#include <errno.h> //para tratamiento de errores

/* Constantes usadas */
#define workers 5 //numero de hilos 
/* Hebras que realizan la búsqueda */
#define request 110 /* Primos a encontrar */
/*
* Macros
*/
//para mostrar errores
#define check(status,string) if (status != 0) { errno = status; fprintf(stderr, "%s status %d: %s\n", string, status, strerror(status));}

/* Datos globales */
pthread_mutex_t prime_list = PTHREAD_MUTEX_INITIALIZER; /* Mutex para el primo */
pthread_mutex_t current_mutex = PTHREAD_MUTEX_INITIALIZER; /* Número actual */
pthread_mutex_t cond_mutex = PTHREAD_MUTEX_INITIALIZER; /* Mutex para arranque */
pthread_cond_t cond_var = PTHREAD_COND_INITIALIZER; /* Variable de estado para el arranque */

int current_num = 1; /*Siguiente número a comprobar */
int thread_hold = 1; /*Número asociado al estado */
int count = 1; /*Cuenta de números primos e índice a los mismos */
int primes[request]; /*Reserva de primos – acceso sincronizado */
pthread_t threads[workers]; /*Matriz con las hebras trabajadoras */

static void unlock_cond(void *arg)
{
    /*****************
        Para desbloquear el mutex de condicion
        devuelve un status y luego se mira si 
        hubo error
    *****************/
    int status;
    status = pthread_mutex_unlock(&cond_mutex);
    check(status, "Mutex_unlock");
}

/*
    Rutina de trabajo.
    Cada hebra arranca con esta rutina. Se realiza primero una espera diseñada
    para sincronizar los trabajadores con el capataz. Cada trabajador hace después
    su turno tomando un número del que determina si es primo o no.
*/

void *prime_search(void *arg){
    int numerator; /* Usada para determinar la primalidad */
    int denominator; /* Usada para determinar la primalidad */
    int cut_off; /* Número a comprobar dividido por 2 */
    int notifiee; /* Usada durante la cancelación */
    int prime; /* Flag para indicar la primalidad */
    int my_number; /* Identificador de la hebra trabajadora */
    int status; /* Status de las llamadas a pthread_* */
    int not_done = 1; /* Predicado del lazo de trabajo */
    int oldstate; /* Estado de cancelado previo */

    my_number = (int)arg; //el argumento es el id del thread

    /*
        Sincronizamos los trabajadores y el capataz usando una variable de estado cuyo
        predicado (thread_hold) será rellenado por el capataz.
    */

    status = pthread_mutex_lock(&cond_mutex); //ponemos candado para establecer valores
    check(status, "Mutex_lock"); //miramos si error
    pthread_cleanup_push(unlock_cond, NULL); //establecemos metodo para cuando se haga exit, o cancell o pop
    while (thread_hold) {
        status = pthread_cond_wait(&cond_var, &cond_mutex);
        check(status, "Cond_wait");
    }
    pthread_cleanup_pop(1);

    /*
    Realiza las comprobaciones sobre números cada vez mayores hasta encontrar el
    número deseado de primos.
    */

    while (not_done) {/* Comprobar petición de cancelación */

        pthread_testcancel();
        /* Obtener siguiente número a comprobar */
        status = pthread_mutex_lock(&current_mutex); //cerramos mutex
        check(status, "Mutex_lock"); //miramos si hubo error
        /********************************************/
        //zona protegida por mutex
            current_num = current_num + 2; /* Nos saltamos los pares */
            numerator = current_num;
        /********************************************/
        status = pthread_mutex_unlock(&current_mutex);
        check(status, "Mutex_unlock");
        /* Verificamos primalidad hasta número/2 */
        cut_off = numerator/2 + 1;
        prime = 1;
        /* Comprobamos la divisibilidad */
        for (denominator = 2;((denominator < cut_off) && (prime)); denominator++)
            prime = numerator % denominator;

        if (prime != 0) {
            /* Inhibir posibles cancelaciones */
            pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldstate);
            /*
                Obtener mutex y añadir este primo a la lista. Cancelar el resto de hebras si
                ya se ha obtenido la cantidad pedida de primos.
            */
            status = pthread_mutex_lock(&prime_list);
            check(status, "Mutex_lock");
            /********************************************/
            //zona protegida por mutex
                if (count < request) {
                    primes[count] = numerator;
                    count++;
                }
                else if (count == request) {
                    not_done = 0;
                    count++;
                    for (notifiee = 0; notifiee < workers; notifiee++) {
                        if (notifiee != my_number) {
                            status = pthread_cancel(threads[notifiee]);
                            check(status, "Cancel");
                        }
                    }
                }
            /*******************************************************/

            status = pthread_mutex_unlock(&prime_list);
            check (status, "Mutex_unlock");
            /* Permitir de nuevo cancelaciones */
            pthread_setcancelstate(oldstate, &oldstate);
        }
        pthread_testcancel();
    }
    return arg;
}

void main()
{
    int worker_num; /* Índice de trabajadores */
    void *exit_value; /* Estado final para cada trabajador */
    int list; /* Para imprimir la lista de primos encontrados */
    int status; /* Status de las llamadas a pthread_* */
    int index1; /* Para ordenar los primos */
    int index2; /* Para ordenar los primos */
    int temp; /* Parte de la ordenación */
    int line_idx; /* Alineado de columna en salida */

    /*
        Creación de las hebras trabajadoras.
    */

    for (worker_num = 0; worker_num < workers; worker_num++) {
        status = pthread_create(&threads[worker_num], NULL,prime_search, (void *)worker_num);
        check(status, "Pthread_create");
    }

    /*
        Poner a cero el predicado thread_hold y señalizar globalmente que los
        trabajadores pueden comenzar.
    */
    status = pthread_mutex_lock(&cond_mutex);
    check(status, "Mutex_lock");
    thread_hold = 0;
    status = pthread_cond_broadcast(&cond_var);
    check(status, "Cond_broadcast");
    status = pthread_mutex_unlock(&cond_mutex);
    check(status, "Mutex_unlock");

    /*
    Hacer JOIN con cada trabajador para obtener los resultados y asegurarse de que
    todos se han completado correctamente.
    */
    for (worker_num = 0; worker_num < workers; worker_num++) {
        status = pthread_join(threads[worker_num], &exit_value);
        check(status, "Pthread_join");
        /*
            Si la terminación es correcta, el valor final exit_value es igual a
            worker_num.
        */
         if (exit_value == (void *)worker_num)
            printf("Hebra %d terminada normalmente.\n", worker_num);
        else if (exit_value == PTHREAD_CANCELED)
            printf("Hebra %d fue cancelada.\n", worker_num);
        else
            printf("Hebra %d terminada con error %#lx.\n",worker_num, exit_value);
    }

    /*
        Tomamos la lista de primos encontrados
        ordenamos de menor a mayor. Puesto que
        hay ninguna garantía respecto al orden
        Por tanto, es necesaria la ordenación.
        por las hebras trabajadoras y los
        las hebras han trabajado en paralelo no
        en que están almacenados los primos.
        Algoritmo de la burbuja.
    */
    for (index1 = 1; index1 < request; index1++) {
        for (index2 = 0; index2 < index1; index2++) {
            if (primes[index1] < primes[index2]) {
                temp = primes[index2];
                primes[index2] = primes[index1];
                primes[index1] = temp;
            }
        }
    }

    /*
        Imprimir la lista de primos obtenidos.
    */
    printf("La lista de %d primos es la siguiente:\n2", request);
    for (list = 1, line_idx = 1; list < request; list++, line_idx++) {
        if (line_idx >= 4) {
            printf (",\n");
            line_idx = 0;
        }
        else if (line_idx > 0) {
            printf(",\t");
        }
        printf("%d", primes[list]);
    }
    printf("\n");
}