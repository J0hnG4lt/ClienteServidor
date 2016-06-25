# Ejecute make con el argumento DEBUG=1 para que se incluya la información
# para el debugger

ifdef DEBUG
FLAGS = -Wall -g
else
FLAGS = -Wall
endif

.PHONY: all clean
all : cliente servidor
cliente : cliente.c mensajes.h configuracion.h
	gcc $(FLAGS) cliente.c -o cliente
servidor : servidor.c conjunto_hash.c conjunto_hash.h mensajes.h configuracion.h
	gcc $(FLAGS) servidor.c conjunto_hash.c -o servidor
clean:
	rm cliente servidor
