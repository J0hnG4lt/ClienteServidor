all : cliente servidor
	

cliente : cliente.c configuracion.h
	gcc -Wall cliente.c configuracion.h -o cliente

servidor : servidor.c conjunto.c configuracion.h
	gcc -Wall servidor.c conjunto.c configuracion.h -o servidor

