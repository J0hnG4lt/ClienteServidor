# Ejecute make con el argumento DEBUG=1 para que se incluya la información
# para el debugger

ifdef DEBUG
FLAGS = -Wall -g
else
FLAGS = -Wall
endif

.PHONY: all clean
all : cliente servidor
cliente : cliente.c cliente.h configuracion.h
	gcc $(FLAGS) cliente.c configuracion.h -o cliente
servidor : servidor.c conjunto_hash.c conjunto_hash.h configuracion.h
	gcc $(FLAGS) servidor.c conjunto_hash.c configuracion.h -o servidor
clean:
	rm cliente servidor
