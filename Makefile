# Makefile para construir el proyecto.
# Ejecute make con el argumento DEBUG=1 para que se incluya la información
# para el debugger.
# Autores:
#    Alfredo Fanghella, 12-10967
#    Georvic Tur, 12-11402

ifdef DEBUG
FLAGS = -Wall -g
else
FLAGS = -Wall
endif

.PHONY: all clean test
all: sem_cli sem_svr
sem_cli: cliente.c mensajes.h configuracion.h
	gcc $(FLAGS) cliente.c -o sem_cli
sem_svr: servidor.c conjunto_hash.c conjunto_hash.h mensajes.h configuracion.h
	gcc $(FLAGS) servidor.c conjunto_hash.c -o sem_svr
clean:
	rm sem_cli sem_svr
test_hash: test_hash.c conjunto_hash.c conjunto_hash.h
	gcc -Wall -o _test_hash test_hash.c conjunto_hash.c
	./_test_hash
	rm _test_hash
