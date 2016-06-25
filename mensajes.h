/**
 * Definición de los tipos de datos para los mensajes.
 * Los mensajes se envían y leen directamente del socket usando estructuras de
 * este tipo. Ambas incluyen un campo pad, que ocupa el relleno el compilador 
 * les debe incluir de forma implícita.
 * La intención es restar el tamaño de ese campo para obtener el tamaño real
 * de la estructura al enviarla y recibirla.
 * Por su tamaño, se garantiza que cualquier dispositivo que implemente el 
 * protocolo IPv4 pueda recibirlos (el mínimo exigido es 576, que restándole
 * 60 de header IP con opciones y 8 de header UDP nos deja un tamaño de 508 
 * octetos).
 * Consulte el informe para ver el significado de cada campo.
 * 
 * @author Alfredo Fanghella, 12-10967
 * @author Georvic Tur, 12-11402
 * @see https://tools.ietf.org/html/rfc791 (página 24)
 * @see http://www.catb.org/esr/structure-packing/
 */
#ifndef MENSAJES
#define MENSAJES

#include <stdint.h>

// Mensaje que envía el servidor al cliente.
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
} mensaje_s;

// Mensaje que envía el cliente al servidor.
typedef struct {
	uint32_t ident;
	uint8_t accion;
	uint8_t pad[3]; 
} mensaje_c;

#endif
