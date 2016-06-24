# Ejecute make con el argumento DEBUG=1 para que se incluya la información
# para el debugger

ifdef DEBUG
FLAGS = -Wall -g
else
FLAGS = -Wall
endif

all : cliente servidor
	

cliente : cliente.c configuracion.h
	gcc $(FLAGS) cliente.c configuracion.h -o cliente

servidor : servidor.c conjunto.c configuracion.h
	gcc $(FLAGS) servidor.c conjunto.c configuracion.h -o servidor
