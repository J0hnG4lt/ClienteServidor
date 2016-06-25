#ifndef MENSAJES
#define MENSAJES

#include <stdint.h>

typedef struct {
	uint32_t ident;
    uint32_t precio;
    uint8_t hora;
    uint8_t minuto;
    uint8_t dia;
    uint8_t mes;
    uint8_t anyo;
	uint8_t accion;
	uint8_t pad[2];
	/* C incluye este espacio en la estructura como relleno por razones de
	 * alineamiento de datos. Aquí la pusimos explicita para facilitar 
	 * obtener su tamaño al momento de enviarla
	 */
} mensaje_s;

typedef struct {
	uint32_t ident;
	uint8_t accion;
	uint8_t pad[3]; 
	/* C incluye este espacio en la estructura como relleno por razones de
	 * alineamiento de datos. Aquí la pusimos explicita para facilitar 
	 * obtener su tamaño al momento de enviarla
	 */
} mensaje_c;

#endif
