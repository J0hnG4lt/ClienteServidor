/**
 * Header con la declaración de las estructuras y funciones para el tipo de dato
 * conjunto. Para más datos sobre la implementación y los comentarios de las 
 * funciones, consulte el archivo con el código.
 * 
 * @author Alfredo Fanghella, 12-10967
 * @author Georvic Tur, 12-11402
 */
#ifndef CONJUNTO_HASH
#define CONJUNTO_HASH

#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#define TAM_HASH 401		// número primo que mantiene la carga baja
#define MAX_CARGA 300       // carga máxima antes de hacer reHash
#define MIN_BORRADOS 100    // cantidad de elementos borrados para hacer reHash
#define LIBRE 0				// Estado de los elementos de la tabla
#define OCUPADO 1
#define BORRADO 2

// struct para cada elemento de la tabla
struct hash_elem {
	time_t tiempo;
	uint32_t ident;
	char estado;
};

// Tipo de dato conjunto, usando tabla de hash
typedef struct {
    struct hash_elem elems[TAM_HASH];
    int ocupados;
    int borrados;
} conjunto;

// Operaciones sobre el tipo de dato
void inicializarConj(conjunto * c);
bool insertarEnConj(conjunto *c, uint32_t identificador, time_t tiempo);
bool eliminarEnConj(conjunto *c, uint32_t identificador, time_t *tiempoEstacionado);
void imprimirConj(conjunto *c);

#endif
