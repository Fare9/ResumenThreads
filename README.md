#RESUMEN TEORÍA THREADS
#Definiciones

Hebra: contiene elementos necesarios para ejecución

    -CP, registros de datos, direcciones... (Cada hebra la cpu le crea su stack frame)

Proceso(Unix): 1 a N hebras, espacio direcciones, descriptores ficheros...

Hebras en un proceso comparten espacio direcciones, descriptores ficheros...

#Tipos de operaciones:
    
    -Asincrona: se ejecutan de manera totalmente independiente.
    -Concurrencia: cuando operaciones se pueden mezclar de forma arbitraria y que ejecuten manera independiente. (No dependen una de la otra, pero pueden estar como un "Switch" en el procesador)
    -Paralelismo: operaciones concurrentes ejecutandose al mismo tiempo.

Concurrencia se da en mono y multi procesador.
Paralelismo solo multiprocesador.
Hebras proporciona concurrencia y paralelismo.


#Coste hebras:

sincronización coste computacional.
Deficiencias niveles bajos(cuellos de botella): SSOO,ficheros,drivers...
Necesario adecual nº hebras a procesador.

Programador mayor esfuerzo, programas más complicados, hebras ven espacio direcciones de los demás.

Depuración casi imposible: temporización es clave, puede no fallar en años y un día fallar.(No herramientas)

#POSIX
pthreads = POSIX threads

Forma de programación de hilos usando una norma estandar, principalmente para UNIX.

Tipo básico:
```C
    pthread_t thread; 
```

Variable de tipo thread (guarda un int).
     #define pthread_t int

Funciones:

    -Funcion para la creación de los hilos lo primero que acepta es un puntero a la variable antes dicha, segundo un puntero a atributos del hilo (normalmente se deja a NULL), 
    tercero puntero a función de tipo void, y cuarto puntero a argumentos (al no saber el tipo se pasan como void y luego se hace cast)
```C
int pthread_create(pthread_t *thread, const pthread_attr_t
*attr, void *(*start)(void *), void *arg);
```
    -Funcion para destruir el thread, la cual liberará los recursos asociados, admite el puntero a un valor para devolverlo a un hilo que espere por él.
```C    
int pthread_exit(void *value_ptr);
```
    -Función que convierte un hilo en detached, cuando está así sus recursos son automáticamente liberados y devueltos al sistema..
```C
int pthread_detach(pthread_t thread);
```
    -Función que espera por un hilo, el primer parametro es un identificador de hilo, el segundo es un puntero al puntero del valor que devuelve el hilo(si devuelve algo).

```C
int pthread_join(pthread_t thread, void **value_ptr);
```
    -Funcion para hacer que el hilo que la llama deje la CPU a otros hilos, entonces se mete al final de la cola de prioridades.
```C
int sched_yield(void);
```
    -Compara dos identificadores de thread, si ambos son iguales devuelve un número no cero(true) si es falso devuelve un 0
int pthread_equal(pthread_t t1, pthread_t t2);


Hebra es el tipo pthread_t, y su ejecución empieza con pthread_create, ejecutando la función indicada, este rellenará la variable de pthread_t. Una hebra en ejecución puede saber su identificador con: 
```C
pthread_t pthread_self()
```
Main es hebra principal: al acabar termina todo el proceso(obliga a terminar sus hebras, devuelve recursos al SSOO).

Una hebra solo devuelve recursos si está en detached (por defecto)

#Ciclo vida hebra

Creada ----> LISTA   --planificada-->   RUNNING   --esperando recurso--> BLOCKED 

puede volver de bloqueada a lista, entre LISTA y RUNNING pueden ir de una a otra según la llevan al procesador o no. Finalmente después de RUNNING va a TERMINADA cuando acaba.

ejemplo1Threads es un pequeño ejemplo donde se muestra como crear los hilos, y como al ejecutarse no siempre se ejecutan de la misma forma.


#Mutex
Sincronizar hebras, acceso mutuo excluyente a datos compartidos.
Importante al modificar datos, o leer datos antes escritos.
Hardware no lo garantiza.

Mutex son objetos que permiten esta exclusión(son candados)

Objeto:
```C
//necesario inicializarlo
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
```
Funciones:

    -Función para inicial el mutex, si no se hizo con PTHREAD_MUTEX_INITIALIZER, lo crea dinamicamente:
```C
int pthread_mutex_init(pthread_mutex_t *mutex,pthread_mutexattr_t *attr);
```
    -Función para destruir mutex si se creo dinamicamente.
```C
int ptread_mutex_destroy(pthread_mutex_t *mutex);
```
    -Función para bloquear el mutex, solo un hilo puede bloquear el mutex, si otro intenta bloquearlo cuando otro lo ha hecho espera hasta que el primero lo libera:
```C
int pthread_mutex_lock(pthread_mutex_t *mutex);
```
    -Función para desbloquear mutex:
```C
int pthread_mutex_unlock(pthread_mutex_t *mutex);
```
    -Función que intenta bloquearlo y si no puede devuelve un error para mostrar (error EBUSY):
```C
int pthread_mutex_trylock(pthread_mutex_t *mutex);
```
Así que cuando un hilo accede a una variable global compartida por todos los hilos, echa el candado(mutex) la usa y luego quita el candado, permitiendo otros hagan lo mismo.


#Variables de estado

para comunicar estado datos compartidos, siempre llevan asociado mutex.
hebra bloquea mutex y espera cambio estado, pthreads asegura atomicidad en operaciones. Variables de estado señalizan condición no hacen exclusión. Cada variable un solo predicado.

Objeto:
```C
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
```
Funciones:
    -Función para inicializarlo dinámicamente:
```C
int pthread_cond_init(pthread_cond_t *cond,pthread_condattr_t *attr);
```
    -Función para destruir condicion, necesario saber no hay ni habrá hebras a la espera de cambio de estado, ni a señalizar cambio:
```C
int ptread_cond_destroy(pthread_cond_t *cond);
```
    -Función para hacer una espera voluntaria, al realizar esta función el hilo debe tener bloqueado el mutex, la función libera el mutex al realizar la espera. Una vez señalizada condición función desbloquea y retoma bloqueo del mutex antes de seguir.
```C
int pthread_cond_wait(pthread_cond_t *cond,pthread_mutex_t *mutex);
```
    -Función para hacer la espera pero esta vez con un tiempo, que pasado el tiempo se desbloquea.
```C
int ptread_cond_timedwait(pthread_cond_t *cond,pthread_mutex_t *mutex,struct timespec *expiration);
```

    -Función para señalizar el cambio de estado a una sola hebra, cualquiera en espera puede procesar el cambio:
```C
int ptread_cond_signal(pthread_cond_t *cond);
```    
    -Función para señalizar el cambio de estado a todas las hebras:
```C
int ptread_cond_broadcast(pthread_cond_t *cond);
```

#Paradigmas de uso

muchas soluciones para trabajo con hilos.

#Modelo trabajo en cadena

Trabajo descompone tareas secuenciales, input/output definidos.
Cada hebra una o más tareas, pasando dato a otra hebra para su tarea. (cadena).

#Modelo grupo de trabajo

Trabajo descompone tareas paralelas. Descompone conjunto datos en subconjuntos, cada uno procesado por hilo. Similar a SIMD, grupo trabajo = hilos realizan procesamiento independiente sobre conjuntos distintos de datos (MIMD).

#Modelo cliente-servidor

Servidor realiza tarea especializada, cliente pide realización de trabajo y realiza otras tareas (ejemplo pedir a otros servidores), cliente coordina tareas entre servidores.(Modelo capataz-trabajadores)


#Programación avanzada

    -Inicialización única: cuando hay operaciones han de ser realizadas una sola vez (iniciar variables, crear mutex...),usamos una variable booleana para controlar (esta estará protegida por mutex inicializado estáticamente). Normalmente esto se hace en el main, si no es posible usamos función pthread_once.
```C
pthread_once_t once_control = PTHREAD_ONCE_INIT;

int pthread_once(pthread_once_t *once_control,void (*rutina_inicio)(void));
```
rutina_inicio se ejecutará una sola vez da igual hebra, la función comprueba primero once_control.

    -Objetos atributo: lista de argumentos que se añade cuando se van a crear ciertos objetos. Se aplican a mutex,variables estado y hebras.

Mutex: 
    Objeto básico: 
    ```C
        pthread_mutexattr_t attr;
    ```
    Creación:
    ```C
        int pthread_mutexattr_init(pthread_mutexattr_t *attr);
    ```
    Destrucción:
       ```C
        int pthread_mutexattr_destroy(pthread_mutexattr_t *attr);
        ```
    Atributo: compartición mutex entre procesos:
    ```C
        int pthread_mutexattr_getpshared(pthread_mutexattr_t *attr,int *pshared);
        int pthread_mutexattr_setpshared(pthread_mutexattr_t *attr,int pshared);
    ```
Variable de condición:
    Objeto básico:
    ```C
        pthread_condattr_t attr;
    ```
    Creación:
        ```C
        int pthread_condattr_init(pthread_condattr_t *attr);
        ```
    Destrucción:
    ```C
        int pthread_condattr_destroy(pthread_condattr_t *attr);
    ```
    Atributo: compartición variable condición entre procesos:
    ```C
        int pthread_condattr_getpshared(pthread_condattr_t *attr,int *pshared);
        int pthread_condattr_setpshared(pthread_condattr_t *attr,int pshared);
    ```

Hebra:
    Objeto básico:
    ```C
        pthread_attr_t attr;
    ```
    Creación:
    ```C
        int pthread_attr_init(pthread_attr_t *attr);
        ```
    Destrucción:
        ```C
        int pthread_attr_init(pthread_attr_t *attr);
        ```
#Atributos de hebras
    Detach state:
        ID de hebra puede hacer JOIN: PTHREAD_CREATE_JOINABLE(por defecto)
        Hebra retorna recursos al SSOO: PTHREAD_CREATE_DETACHED
        Para detached funciones:
```C
//trae el valor del atributo DETACHSTATE en detachstate
int pthread_attr_getdetachstate(pthread_attr_t *attr,int *detachstate);

//establece el atributo de DETACHSTATE al valor de detachstate (o NULL o PTHREAD_CREATE_DETACHED)
int pthread_attr_setdetachstate(pthread_attr_t *attr,int detachstate);
```
    Tamaño pila: puede ser interesante modificar cantidad memoria reservada para pila(ejemplo para recursividad), modificar pila es poco portable:
```C
//trae el valor del tamaño de la pila y lo mete en stacksize
int pthread_attr_getstacksize(pthread_attr_t *attr,size_t *stacksize);
//coge el valor de stacksize y lo pone como nuevo tamaño de pila
int pthread_attr_setstacksize(pthread_attr_t *attr,size_t stacksize);
```
#Cancelar hebra

suelen acabar por sí, a veces interesante cancelar. Ejemplo si un usuario pulsa un botón "cancelar", esto no mata a hebra, la pide irse:
```C
int pthread_cancel(pthread_t thread);
```
Cancelación usa dos bits:
    - Estado (enable/disable)
    - Tipo (deferred/asynchronous)
por defecto deferred (solo ocurrir en ciertas zonas)
```C
//poner nuevo estado y recoger los antiguos
int pthread_setcancelstate(int state, int *oldstate);
```
state puede ser:
    PTHREAD_CANCEL_ENABLE
    PTHREAD_CANCEL_DISABLE
```C
//poner nuevo tipo y recoger el antiguo
int pthread_setcanceltype(int type, int *oldtype);
```
type puede ser:
    PTHREAD_CANCEL_DEFERRED
    PTHREAD_CANCEL_ASYNCHRONOUS

Funciones rutina limpieza antes de cancelación:
```C
    int pthread_cleanup_push(void (*rutina)(void *), void *arg);
```
se ejecuta cuando hebra se cancela, con pthread_exit y con la siguiente función:
```C
    int pthread_cleanup_pop(int execute);
```
#ERRORES

    -Inercia hebras: hebras asíncronas, sistema monoprocesador, hebra que crea otra tiene ventaja en ejecución. Puede pasar lo mismo en multiprocesador si se alcanzó limite. Crea "incercia"

    -Competición entre hebras: dos o más hebras quieren lo mismo a la vez, solo una gana, ¿cuál?, muchos factores. Hebra puede ser interrumpida cualquier punto, no hay orden exacto.

    -Abrazos mortales: sincronización errónea. A tiene 1 y no puede seguir hasta tener 2, B tiene 2 y no puede seguir hasta tener 1. Recursos conflictivos = Mutexes.

    -Compartición variables de estado diferentes predicados: cada variable usarse sólo para una condición.

    -Problemas rendimiento: atención paralelismo "serializado" , usar "gran" mutex para acceso a una librería es basura, usar muchos mutex jode rendimiento...
