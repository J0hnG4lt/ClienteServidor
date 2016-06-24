#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <time.h>

// Autores: Georvic Tur           12-11402
//          Alfredo Fanghella     12-10967



//Se usa una lista enlazada para implementar un conjunto de identificadores
// con tiempo de llegada en segundos desde epoch
struct nodo {
    char *identificador;
    time_t tiempo;
};

struct conj {
    struct nodo *elemento;
    struct conj *siguiente;
};

/**
 * Introduce el primer elemento al conjunto vacío.
 *
 * @param conjunto Conjunto vacío cuya memoria ya ha sido asignada
 * @param identificador ID numérico del carro
 * @param tiempo Tiempo de llegada medido en segundos desde epoch
 */
void inicializarConj(struct conj *conjunto, char *identificador, time_t tiempo){
    conjunto->siguiente = NULL;
    struct nodo *nuevoNodo = (struct nodo *)malloc(sizeof(struct nodo));
    nuevoNodo->identificador = identificador;
    nuevoNodo->tiempo = tiempo;
    conjunto->elemento = nuevoNodo;
}


/**
 * Inserta un elemento en el conjunto. Si este elemento ya existe en el
 * mismo, entonces no se hace nada. De lo contrario, se inserta al final.
 *
 * @param conjunto Conjunto que ya ha sido inicializado
 * @param identificador ID numérico del carro
 * @param tiempo Tiempo de llegada medido en segundos desde epoch
 * @return 0 si el conjunto no tenía ya el identificador. Es 1 de lo contrario.
 */
int insertarEnConj(struct conj *conjunto, char *identificador, time_t tiempo){
    struct conj *conjActual = conjunto;
    struct conj *conjAnterior = NULL;
    int encontrado = 0;
    while (conjActual){
        if (conjActual->elemento){
            if (strcmp(conjActual->elemento->identificador, identificador)==0){
                encontrado = 1;
                break;
            }
        }
        conjAnterior = conjActual;
        conjActual = conjActual->siguiente;
    }
    if (!encontrado){
        conjActual = (struct conj *)malloc(sizeof(struct conj));
        struct nodo *nuevoNodo = (struct nodo *)malloc(sizeof(struct nodo));
        nuevoNodo->identificador = identificador;
        nuevoNodo->tiempo = tiempo;
        conjActual->elemento = nuevoNodo;
        conjAnterior->siguiente = conjActual;
        conjActual->siguiente = NULL;
    }
    return encontrado;
}


/**
 * Libera la memoria usada por el conjunto
 * 
 * @param conjunto Conjunto que ya ha sido inicializado
 */
void liberarConj(struct conj *conjunto){
    struct conj *conjActual = conjunto;
    struct conj *conjAnterior = NULL;
    while (conjActual){
        conjAnterior = conjActual;
        conjActual = conjActual->siguiente;
        if (conjAnterior->elemento){
            free(conjAnterior->elemento);
        }
        free(conjAnterior);
    }   
}


/**
 * Elimina un nodo del conjunto si éste contiene al identificador
 * 
 * @param conjunto Conjunto que ya ha sido inicializado
 * @param identificador ID numérico del carro a eliminar del conjunto
 * @param tiempoEstacionado Apuntador al tiempo de llegada medido en segundos desde epoch
 * @return 1 si el conjunto tenía el identificador que fue liberado. Es 0 de lo contrario.
 */
int eliminarEnConj(struct conj **conjunto, char *identificador, time_t *tiempoEstacionado){
    struct conj *conjActual = *conjunto;
    struct conj *conjAnterior = NULL;
    struct conj *aux = *conjunto;
    int numero=1;
    int liberado=0;
    while (conjActual){
        
        conjAnterior = conjActual;
        conjActual = conjActual->siguiente;
        if (conjAnterior->elemento){
            if (strcmp(conjAnterior->elemento->identificador, identificador)==0){
                
                if(numero==1){
                    *conjunto = conjActual;
                }
                
                if (numero >= 3){
                    aux = aux->siguiente;
                }
                
                memcpy(tiempoEstacionado, &(conjAnterior->elemento->tiempo), sizeof(time_t));
                free(conjAnterior->elemento->identificador);
                free(conjAnterior->elemento);
                free(conjAnterior);
                if (conjActual){
                    aux->siguiente = conjActual;
                }else {
                    aux->siguiente = NULL;
                }
                liberado=1;
                
                
                break;
            }
        }
        
        
        if (numero >= 3){
            aux = aux->siguiente;
        }
        numero++;
        
    }
    
    return liberado;
}

/**
 * Imprime los elementos del conjunto.
 *
 * @param conjunto Conjunto que ya ha sido inicializado
 * 
 */
void imprimirConj(struct conj *conjunto){
    struct conj *conjActual = conjunto;
    struct conj *conjAnterior = NULL;
    while (conjActual){
        conjAnterior = conjActual;
        conjActual = conjActual->siguiente;
        if (conjAnterior->elemento){
            printf("Elemento de Conj %s. Tiempo: %f\n", conjAnterior->elemento->identificador, (double) conjAnterior->elemento->tiempo);
        }
    }
}


