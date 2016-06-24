#ifndef CLIENTE
#define CLIENTE

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <time.h>

typedef struct {
	uint32_t ident;
    uint32_t precio;
    uint32_t hora;
    uint32_t minuto;
    uint32_t dia;
    uint32_t mes;
    uint32_t anyo;
	uint8_t accion;
	uint8_t pad[3]; 
	/* C incluye este espacio en la estructura como relleno por razones de
	 * alineamiento de datos. Aquí la pusimos explicita para facilitar 
	 * obtener su tamaño al momento de enviarla
	 */
} mensaje;

#endif
