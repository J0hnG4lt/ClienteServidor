// Autores: Georvic Tur           12-11402
//          Alfredo Fanghella     12-10967
// Pruebas unitarias para las operaciones sobre el tipo de dato conjunto.

#include "conjunto_hash.h"
#include <stdio.h>
#include <assert.h>

// esta no es una prueba automática. Solo agrega elementos y los muestra.
void test_agregar(conjunto *c) {
	srand(time(NULL));
	int i;
	uint32_t e;
	
	for (i = 0; i < 299; i++) {
		e = rand();
		insertarEnConj(c, e, time(NULL));
	}
	imprimirConj(c);
	
}

// Revisa que no se agreguen elementos repetidos
void test_agregar_repetidos(conjunto *c) {
	uint32_t elems[] = {1,2,3,4,1,5,27,2,4,6};
	bool exito[] = {1,1,1,1,0,1,1,0,0,1};
	char *cond[] = {"fracaso", "exito"};
	bool fracaso = false;
	bool result;
	
	int i;
	for (i = 0; i < 10; i++) {
		result = insertarEnConj(c, elems[i], time(NULL));
		if (exito[i] != result){
			fracaso = true;
			printf(
				"Se esperaba %s al insertar %d, pero resultó %s\n",
				cond[exito[i]], elems[i], cond[result]);
		}
	}
	
	imprimirConj(c);
	
	if (fracaso) exit(1);
	
}

// Revisa que se borren los elementos y que los espacios borrados se reaprovechen
void test_borrar(conjunto * c) {
	time_t t;

	assert(insertarEnConj(c, 0, time(NULL)) == true);
	assert(insertarEnConj(c, 401, time(NULL)) == true);
	assert(insertarEnConj(c, 1, time(NULL)) == true);
	assert(insertarEnConj(c, 7, time(NULL)) == true);
	assert(insertarEnConj(c, 408, time(NULL)) == true);
	assert(insertarEnConj(c, 8, time(NULL)) == true);
	
	assert(eliminarEnConj(c, 401, &t) == true);
	assert(eliminarEnConj(c, 401, &t) == false);
	assert(eliminarEnConj(c, 1, &t) == true);
	assert(eliminarEnConj(c, 1, &t) == false);
	assert(insertarEnConj(c, 1, time(NULL)) == true);
	assert(insertarEnConj(c, 0, time(NULL)) == false);
	assert(eliminarEnConj(c, 8, &t) == true);
	assert(insertarEnConj(c, 9, time(NULL)) == true);
	assert(c->borrados == 1);
	assert(c->ocupados == 5);
	
	imprimirConj(c);
	
}

// Revisa que la tabla se redistribuya para eliminar borrados cuando se pasa
// la carga máxima
void test_rehash(conjunto * c) {
	int i;
	for (i = 0; i < 200; i++) {
		insertarEnConj(c, i, time(NULL));
	}
	
	time_t t;
	for (i = 0; i < 120; i++) {
		eliminarEnConj(c, i, &t);
	}
	
	assert(c->borrados == 120);
	
	for (i = 200; i < 320; i++) {
		insertarEnConj(c, i, time(NULL));
	} 
	
	assert(c->borrados == 0); // se limpiaron los borrados
	assert(c->ocupados == 200);
}

int main(int argc, char ** argv){
    conjunto c;
    inicializarConj(&c);
    test_agregar(&c);
    puts("Exito 1");
    inicializarConj(&c);
    test_agregar_repetidos(&c);
    puts("Exito 2");
    inicializarConj(&c);
    test_borrar(&c);
    puts("Exito 3");
    inicializarConj(&c);
    test_rehash(&c);
    puts("Exito 4");
    return 0;
}
