#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>




struct nodo {
    char *identificador;
};

struct conj {
    struct nodo *elemento;
    struct conj *siguiente;
};

void inicializarConj(struct conj *conjunto, char *identificador){
    conjunto->siguiente = NULL;
    struct nodo *nuevoNodo = (struct nodo *)malloc(sizeof(struct nodo));
    nuevoNodo->identificador = identificador;
    conjunto->elemento = nuevoNodo;
}

int insertarEnConj(struct conj *conjunto, char *identificador){
    struct conj *conjActual = conjunto;
    struct conj *conjAnterior = NULL;
    int encontrado = 0;
    while (conjActual){
        if (conjActual->elemento){
            if (strcmp(conjActual->elemento->identificador, identificador)==1){
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
        conjActual->elemento = nuevoNodo;
        conjAnterior->siguiente = conjActual;
        conjActual->siguiente = NULL;
    }
    return encontrado;
}


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


int eliminarEnConj(struct conj **conjunto, char *identificador){
    struct conj *conjActual = *conjunto;
    struct conj *conjAnterior = NULL;
    struct conj *aux = *conjunto;
    int numero=1;
    int liberado=0;
    while (conjActual){
        
        conjAnterior = conjActual;
        conjActual = conjActual->siguiente;
        if (conjAnterior->elemento){
            printf("ElementoEl: %s\n", conjAnterior->elemento->identificador);
            if (strcmp(conjAnterior->elemento->identificador, identificador)==1){
                
                if(numero==1){
                    *conjunto = conjActual;
                }
                
                if (numero >= 3){
                    aux = aux->siguiente;
                }
                
                free(conjAnterior->elemento);
                free(conjAnterior->elemento->identificador);
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
        printf("aux: %s\n", aux->elemento->identificador);
    }
    
    return liberado;
}

void imprimirConj(struct conj *conjunto){
    struct conj *conjActual = conjunto;
    struct conj *conjAnterior = NULL;
    while (conjActual){
        conjAnterior = conjActual;
        conjActual = conjActual->siguiente;
        if (conjAnterior->elemento){
            printf("Elemento %s\n", conjAnterior->elemento->identificador);
        }
    }
}

/*
int main(int argc, char **argv){
    struct conj *carros = (struct conj *)malloc(sizeof(struct conj));
    char *carroID1 = malloc(sizeof(char));
    char *carroID2 = malloc(sizeof(char));
    char *carroID3 = malloc(sizeof(char));
    char *carroID4 = malloc(sizeof(char));
    *carroID1 = '1';
    *carroID2 = '2';
    *carroID3 = '3';
    *carroID4 = '4';
    inicializarConj(carros, carroID1);
    insertarEnConj(carros, carroID2);
    insertarEnConj(carros, carroID3);
    insertarEnConj(carros, carroID4);
    if (eliminarEnConj(&carros, '1')){
        printf("Liberado\n");
    }else{
        printf("No Liberado\n");
    }
    imprimirConj(carros);
    liberarConj(carros);
    return 0;
}
*/
