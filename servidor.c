#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include "cliente.h"
#include "conjunto.h"
#include "configuracion.h"

// Autores: Georvic Tur           12-11402
//          Alfredo Fanghella     12-10967


//Constantes definidas en el archivo de configuración

//Se una un manejador para cerrar bien el server, pues se usará ctr-c
void manejadorINTERRUPT(int num);

uint32_t obtenerPrecio(time_t duracion){
	uint32_t costo = 0;
	if (duracion > 3600){
		costo += 80;
		duracion -= 3600;
	}
	
	while(duracion > 0){
		costo += 30;
		duracion -= 3600;
	}
	
	return costo;
	
}

int main(int argc, char **argv){
    
    
    if (argc != 7){
        fprintf(stderr,"Número incorrecto de argumentos.\nUso correcto:\n");
        fprintf(stderr,"\tsem_svr \
                          -l <puerto a servir>\n \t\t\
                          -i <bitácora de entrada>\n \t\t\
                          -o <bitácora de salida>\n");
        exit(EXIT_FAILURE);
    }
    
    //Argumentos del servidor
    char *bitacoraEntrada=NULL;
    char *bitacoraSalida=NULL;
    char *puerto=NULL;
    
    //
    char accion; //Entrar o Salir del estacionamiento
    char *identificador = NULL; //ID del carro actual
    int numPuestosOcupados = 0; //Número de puestos ocupados
    
    time_t *tiempoEstacionado = malloc(sizeof(time_t));
    
    struct conj *carros = NULL; //Conjunto de carros en el estacionamiento
    
    //char *separador = ";";
    //Separador de elementos del mensaje a enviar y recibir
    
    FILE *archivoBitacoraEntrada;
    FILE *archivoBitacoraSalida;
    
    //Manejador de interrupciones para ctr-c
    void manejadorINTERRUPT(int num){
        fclose(archivoBitacoraEntrada);
        fclose(archivoBitacoraSalida);
        liberarConj(carros);
        printf("\n----------------\n");
        printf("Servidor apagado\n");
        printf("----------------\n");
        exit(0);
    }
    
    //Se registra el manejador
    signal(SIGINT, manejadorINTERRUPT);
    
    //Estructura para obtener el tiempo. Luego se pone en el mensaje de respuesta.
    time_t tiempo = time(NULL);
    char *tiempoString = asctime(localtime(&tiempo));
    
    struct tm *structTiempo = (struct tm *) malloc(sizeof(struct tm));
    
    //Se parsean los argumentos
    int opcn;
    while ((opcn = getopt(argc,argv, "l:i:o:")) != -1){
        switch(opcn){
            case 'i':
                if (LON_MAX_STRNG < strlen(optarg)){
                    fprintf(stderr,"Bitácora de Entrada: Longitud máxima de nombre sobrepasada.\n");
                    exit(EXIT_FAILURE);
                }
                bitacoraEntrada = optarg;
                break;
             case 'l':
                if (strlen(optarg) != 4){
                    fprintf(stderr,"Número de puerto de tamaño equivocado.\n");
                    exit(EXIT_FAILURE);
                } else if (!atoi(optarg)){ //Se usa atoi para determinar si es un número
                    fprintf(stderr,"El puerto ha de ser un número.\n");
                    exit(EXIT_FAILURE);
                }
                
                puerto = optarg;
                break;
             case 'o':
                 if (LON_MAX_STRNG < strlen(optarg)){
                     fprintf(stderr,"Bitácora de Salida: Longitud máxima de nombre sobrepasada.\n");
                     exit(EXIT_FAILURE);
                 }
                 bitacoraSalida = optarg;
                 break;
                
             default:
                exit(EXIT_FAILURE);
        }
    }
    
    printf("-------------------------------------------\n");
    printf("Servidor escuchando en el puerto %s\n", puerto);
    printf("-------------------------------------------\n");
    
    //Se abren los archivos
    
    archivoBitacoraEntrada = fopen(bitacoraEntrada, "w");
    
    if (archivoBitacoraEntrada == NULL){
        fprintf(stderr, "No se pudo abrir la bitácora de entrada\n");
        exit(EXIT_FAILURE);
    }
    
    archivoBitacoraSalida = fopen(bitacoraSalida, "w");
    
    if (archivoBitacoraSalida == NULL){
        fprintf(stderr, "No se pudo abrir la bitácora de salida\n");
        exit(EXIT_FAILURE);
    }
    
    //Tipo de socket a usar
    struct addrinfo infoDir;
    memset(&infoDir, 0, sizeof(infoDir));
    infoDir.ai_family = AF_UNSPEC;
    infoDir.ai_socktype = SOCK_DGRAM;
    infoDir.ai_protocol = IPPROTO_UDP;
    infoDir.ai_flags = AI_PASSIVE;

    //Configuracion del socket
    struct addrinfo *dirServ;
    int codigoErr;
    if ((codigoErr = getaddrinfo(NULL, puerto, &infoDir, &dirServ)) != 0){
        fprintf(stderr," Problema al obtener información sobre el servidor.\n");
        gai_strerror(codigoErr);
        exit(EXIT_FAILURE);
    }
    
    //Creación del socket
    int socketSrvdrClt = socket(dirServ->ai_family, dirServ->ai_socktype, dirServ->ai_protocol);
    if (!socketSrvdrClt){
        fprintf(stderr," Problema al crear el socket.\n");
        exit(EXIT_FAILURE);
    }
    
    //Se enlaza el socket
    if (bind(socketSrvdrClt, dirServ->ai_addr, dirServ->ai_addrlen) != 0){
        fprintf(stderr,"No se pudo enlazar el socket.\n");
        exit(EXIT_FAILURE);
    }
    
    //Ya no se necesita
    freeaddrinfo(dirServ);
    mensaje solicitud;
    mensaje structRespuesta;
    //Se repite por cada mensaje entrante
    while(1){
        
        //Se recibe el mensaje de algún cliente
        struct sockaddr_storage dirClnt;
        socklen_t tamanoSocket = sizeof(dirClnt);
        ssize_t numBytesRecibidos = recvfrom(socketSrvdrClt, 
                                        &solicitud, 
                                        sizeof solicitud - sizeof solicitud.pad,
                                        0,
                                        (struct sockaddr *) &dirClnt, 
                                        &tamanoSocket);
        
        if (!numBytesRecibidos){
            fprintf(stderr,"Error al recibir solicitud.\n");
            exit(EXIT_FAILURE);
        }
        
        
        //Se parsea el mensaje de llegada
        accion = solicitud.accion;
        identificador = malloc(20);
        sprintf(identificador, "%d", ntohl(solicitud.ident));
        
        //Se obtiene el tiempo actual
        tiempo = time(NULL);
        tiempoString = asctime(localtime(&tiempo));
        tiempoString[strlen(tiempoString)-1] = '\0';
		
		printf("------------------------------------\n");
        printf("Recibida solicitud de %s en %s\n", identificador, tiempoString);
        printf("------------------------------------\n");
        
        structTiempo = localtime(&tiempo);

        //Se construye el mensaje de respuesta
        ///////////////////////////////////////////
        structRespuesta.ident = solicitud.ident;
        structRespuesta.dia = htonl((uint32_t) structTiempo->tm_mday);
        structRespuesta.mes = htonl((uint32_t) structTiempo->tm_mon);
        structRespuesta.anyo = htonl((uint32_t) structTiempo->tm_year);
        structRespuesta.hora = htonl((uint32_t) structTiempo->tm_hour);
        structRespuesta.minuto = htonl((uint32_t) structTiempo->tm_min);
        //Si el cliente quiere salir o entrar
        if (accion == 'e'){
            
            
            if(carros == NULL){
                carros = (struct conj *)malloc(sizeof(struct conj));
                inicializarConj(carros,identificador, time(NULL));
                numPuestosOcupados++;
                structRespuesta.accion = 's';//Sí se puede ejecutar la acción
                
                //Si quedan puestos y el carro no está ya en el conjunto
            } else if ((numPuestosOcupados < NUM_MAX_PUESTOS) && (insertarEnConj(carros,identificador, time(NULL)) == 0)){
                numPuestosOcupados++;
                structRespuesta.accion = 's';//Sí se puede ejecutar la acción
            
            } else{
                structRespuesta.accion = 'n';//No se puede ejecutar la acción
                
            }
            
            structRespuesta.precio = htonl((uint32_t) 0);
            
            
            //Se registra la operación de entrada
            fprintf(archivoBitacoraEntrada, "Entrada: %c. Identificador: %s. Tiempo: %s\n",
            								structRespuesta.accion,
            								identificador,
            								tiempoString);
            
            
        } else if (accion == 's'){
            
            //Si hay carros y el carro que va a salir está en el conjunto
            if ((numPuestosOcupados > 0) && (eliminarEnConj(&carros,identificador,tiempoEstacionado)==1)){
                numPuestosOcupados--;
                structRespuesta.accion = 's';//Sí se puede ejecutar la acción
                structRespuesta.precio = htonl(obtenerPrecio(tiempo - *tiempoEstacionado));
                //printf("PRECIO: %d\n", obtenerPrecio(tiempo-*tiempoEstacionado));
                
            }
            else{
                structRespuesta.accion = 'n';//No se puede ejecutar la acción
            }
            
            //Se registra la operación de salida
            fprintf(archivoBitacoraSalida, "Salida: %c. Identificador: %s. Tiempo: %s. Monto: Bs. %d.\n",
            								structRespuesta.accion,
            								identificador,
            								tiempoString,
            								obtenerPrecio(tiempo-*tiempoEstacionado));
        
        }
        
        //printf("Puestos Ocupados: %d\n", numPuestosOcupados);
        
        
        
        //Se envía el mensaje de respuesta
        ssize_t numBytesEnviados = sendto(socketSrvdrClt, 
                                        &structRespuesta, 
                                        (sizeof structRespuesta - sizeof structRespuesta.pad),
                                        0,
                                        (struct sockaddr *) &dirClnt, 
                                        tamanoSocket);
        
        
        if (numBytesEnviados < 0){
            fprintf(stderr,"Error al enviar respuesta: %s.\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        
        //imprimirConj(carros);
        
        
    }
    
    return 0;
}
