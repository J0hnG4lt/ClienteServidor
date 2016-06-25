/**
 * Este archivo implementa las operaciones sobre el tipo conjunto que utiliza
 * el servidor. El conjunto está implementado como una tabla de hash, con 
 * resolución de colisiones por direccionamiento abierto con sondeo lineal.
 * Como se conoce de antemano la cantidad máxima de puestos, la tabla se puede
 * construir usando memoria estática. Para el tamaño se escogió un número primo,
 * para poder usar un hashing por división sencillo, y suficientemente grande
 * para mantener el factor de carga de la tabla bajo aunque el estacionamiento
 * este lleno. Así se mejora la eficiencia.
 * Al borrar un elemento de una tabla con direccionamiento abierto, este no
 * saca realmente sino que se marca como borrado. Los elementos borrados 
 * causan retrasos en la eliminación, así que luego de que la carga llega a
 * aproximadamente a 0.75, se limpian.
 * 
 * @author Alfredo Fanghella, 12-10967
 * @author Georvic Tur, 12-11402
 */
#include "conjunto_hash.h"

/**
 * Inicializa el conjunto, dejándolo listo para recibir elementos.
 *
 * @param conjunto Conjunto vacío cuya memoria ya ha sido asignada
 * @param identificador ID numérico del carro
 * @param tiempo Tiempo de llegada medido en segundos desde epoch
 */
void inicializarConj(conjunto * c){
    memset(c, 0, sizeof(conjunto));
}

/**
 * Si la tabla está demasiado cargada por los elementos borrados, 
 * se redistribuyen los elementos reales y se eliminan los borrados.
 * 
 * @param c Conjunto del que se limpiaran los borrados.
 */

void reHash(conjunto * c) {
	conjunto copia;
	memcpy(&copia, c, sizeof(conjunto));
	inicializarConj(c);
	int i;
	for (i = 0; i < TAM_HASH; i++) 
		if (copia.elems[i].estado == OCUPADO) 
			insertarEnConj(c, copia.elems[i].ident, copia.elems[i].tiempo);
}

/**
 * Inserta un elemento en el conjunto. Como se sabe que la cantidad de elementos
 * siempre será menor a los que caben en la tabla, no crece nunca.
 * 
 * @param c Conjunto que ya ha sido inicializado
 * @param identificador ID numérico del carro
 * @param tiempo Tiempo de llegada medido en segundos desde epoch
 * @return true si el conjunto no tenía ya el identificador. Es false de lo contrario.
 */
bool insertarEnConj(conjunto * c, uint32_t identificador, time_t tiempo) {
	int hash = identificador % TAM_HASH;
	int n = 0;
	
	while (n < TAM_HASH) {
		if (c->elems[hash].estado == OCUPADO) {
			if (c->elems[hash].ident == identificador) {
				return false;
			} else {
				hash = (hash + 1) % TAM_HASH;
			}
		} else {
			if (c->elems[hash].estado == BORRADO)
				c->borrados--;
			c->ocupados++;
			c->elems[hash].estado = OCUPADO;
			c->elems[hash].tiempo = tiempo;
			c->elems[hash].ident = identificador;
			if ((c->ocupados + c->borrados > MAX_CARGA) && (c->borrados >= MIN_BORRADOS))
			// si se pasa cierta carga, se eliminan los BORRADOS para aumentar
			// la eficiencia, pero como está operación es costosa solo se hace
			// si la ganancia es significativa
				reHash(c);
			return true;
		}
		n++;
	}
	
	// solo llega aquí si la tabla está llena, lo cual no debe pasar en
	// nuestro programa.
	return false;
}

/**
 * Elimina un elemento del conjunto si éste contiene al identificador. El tiempo
 * de llegada se guarda en la dirección tiempoEstacionado.
 * 
 * @param c Conjunto que ya ha sido inicializado
 * @param identificador ID numérico del carro a eliminar del conjunto
 * @param tiempoEstacionado Apuntador al tiempo de llegada medido en segundos desde epoch
 * @return true si el conjunto tenía el identificador. Es false de lo contrario.
 */
bool eliminarEnConj(conjunto *c, uint32_t identificador, time_t *tiempoEstacionado){
	int hash = identificador % TAM_HASH;
	int n;
	
	while (n < TAM_HASH) {
		if (c->elems[hash].estado == OCUPADO) {
			if (c->elems[hash].ident == identificador) {
				c->elems[hash].estado = BORRADO;
				c->borrados++;
				c->ocupados--;
				*tiempoEstacionado = c->elems[hash].tiempo;
				return true;
			}
		} else if (c->elems[hash].estado == LIBRE) {
			break;
		}
		n++;
		hash = (hash + 1) % TAM_HASH;
	}
	return false;
}

/**
 * Imprime los elementos del conjunto.
 *
 * @param c Conjunto que ya ha sido inicializado.
 * 
 */
void imprimirConj(conjunto * c){
    int i;
    for (i = 0; i < TAM_HASH; i++) {
        if (c->elems[i].estado == OCUPADO){
            printf(
				"Elemento de Conj %d. Tiempo: %ld\n", 
				(int)(c->elems[i].ident),
				(long)(c->elems[i].tiempo));
        }
    }
}
