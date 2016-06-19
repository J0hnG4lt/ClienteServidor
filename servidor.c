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
#include "conjunto.h"
#include "configuracion.h"

// Autores: Georvic Tur           12-11402
//          Alfredo Fanghella     12-10967


//Constantes definidas en el archivo de configuración

//Se una un manejador para cerrar bien el server, pues se usará ctr-c
void manejadorINTERRUPT(int num);


int main(int argc, char **argv){
    
    
    if (argc != 7){
        fprintf(stderr,"Número incorrecto de argumentos.\nUso correcto:\n");
        fprintf(stderr,"\tsem_svr \
                          -l <puerto a servir>\n \t\t\
                          -i <bitácora de entrada>\n \t\t\
                          -o <bitácora de salida>\n");
        abort();
    }
    
    //Argumentos del servidor
    char *bitacoraEntrada=NULL;
    char *bitacoraSalida=NULL;
    char *puerto=NULL;
    
    //
    char *accion = NULL; //Entrar o Salir del estacionamiento
    char *identificador = NULL; //ID del carro actual
    char *solicitudParsed = (char *)calloc(LON_MAX_MENSAJE, 1); //Mensaje de respuesta
    
    int numPuestosOcupados = 0; //Número de puestos ocupados
    
    struct conj *carros = NULL; //Conjunto de carros en el estacionamiento
    
    char *separador = (char *)malloc(sizeof(char));
    separador[0] = ';'; //Separador de elementos del mensaje a enviar y recibir
    
    FILE *archivoBitacoraEntrada;
    FILE *archivoBitacoraSalida;
    
    //Manejador de interrupciones para ctr-c
    void manejadorINTERRUPT(int num){
        free(solicitudParsed);
        fclose(archivoBitacoraEntrada);
        fclose(archivoBitacoraSalida);
        liberarConj(carros);
        exit(0);
    }
    
    //Se registra el manejador
    signal(SIGINT, manejadorINTERRUPT);
    
    //Estructura para obtener el tiempo. Luego se pone en el mensaje de respuesta.
    time_t tiempo = time(NULL);
    char *tiempoString = asctime(localtime(&tiempo));
    
    //Se parsean los argumentos
    int opcn;
    while ((opcn = getopt(argc,argv, "l:i:o:")) != -1){
        switch(opcn){
            case 'i':
                if (LON_MAX_STRNG < strlen(optarg)){
                    fprintf(stderr,"Bitácora de Entrada: Longitud máxima de nombre sobrepasada.\n");
                    abort();
                }
                bitacoraEntrada = optarg;
                break;
             case 'l':
                if (strlen(optarg) != 4){
                    fprintf(stderr,"Número de puerto de tamaño equivocado.\n");
                    abort();
                } else if (!atoi(optarg)){ //Se usa atoi para determinar si es un número
                    fprintf(stderr,"El puerto ha de ser un número.\n");
                    abort();
                }
                
                puerto = optarg;
                break;
             case 'o':
                 if (LON_MAX_STRNG < strlen(optarg)){
                     fprintf(stderr,"Bitácora de Salida: Longitud máxima de nombre sobrepasada.\n");
                     abort();
                 }
                 bitacoraSalida = optarg;
                 break;
                
             default:
                abort();
        }
    }
    
    printf("Puerto:%s, In:%s, Out:%s\n",puerto,bitacoraEntrada, bitacoraSalida);
    
    
    //Se abren los archivos
    
    archivoBitacoraEntrada = fopen(bitacoraEntrada, "w");
    
    if (archivoBitacoraEntrada == NULL){
        fprintf(stderr, "No se pudo abrir la bitácora de entrada\n");
    }
    
    archivoBitacoraSalida = fopen(bitacoraSalida, "w");
    
    if (archivoBitacoraSalida == NULL){
        fprintf(stderr, "No se pudo abrir la bitácora de salida\n");
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
        abort();
    }
    
    //Creación del socket
    int socketSrvdrClt = socket(dirServ->ai_family, dirServ->ai_socktype, dirServ->ai_protocol);
    if (!socketSrvdrClt){
        fprintf(stderr," Problema al crear el socket.\n");
        abort();
    }
    
    //Se enlaza el socket
    if (bind(socketSrvdrClt, dirServ->ai_addr, dirServ->ai_addrlen) != 0){
        fprintf(stderr,"No se pudo enlazar el socket.\n");
        abort();
    }
    
    //Ya no se necesita
    freeaddrinfo(dirServ);
    
    //Se repite por cada mensaje entrante
    while(1){
        
        //Se recibe el mensaje de algún cliente
        struct sockaddr_storage dirClnt;
        char solicitud[LON_MAX_MENSAJE+1];
        socklen_t tamanoSocket = sizeof(dirClnt);
        ssize_t numBytesRecibidos = recvfrom(socketSrvdrClt, 
                                        solicitud, 
                                        LON_MAX_MENSAJE,
                                        0,
                                        (struct sockaddr *) &dirClnt, 
                                        &tamanoSocket);
        
        if (!numBytesRecibidos){
            fprintf(stderr,"Error al recibir solicitud.\n");
            abort();
        }
        
        //Se parsea el mensaje de llegada
        solicitud[strlen(solicitud)] = '\0';
        accion = strdup(strtok(solicitud, separador));
        identificador = strdup(strtok(NULL, separador));
        
        //Se obtiene el tiempo actual
        tiempo = time(NULL);
        tiempoString = asctime(localtime(&tiempo));
        tiempoString[strlen(tiempoString)-1] = '\0';
        
        //Se construye el mensaje de respuesta
        strncpy(solicitudParsed, accion, strlen(accion));
        strncat(solicitudParsed, separador, strlen(separador));
        strncat(solicitudParsed, identificador, strlen(identificador));
        strncat(solicitudParsed, separador, strlen(separador));
        strncat(solicitudParsed, tiempoString, strlen(tiempoString));
        strncat(solicitudParsed, separador, strlen(separador));
        printf("Solicitud: %s\n", solicitudParsed);
        
        //Si el cliente quiere salir o entrar
        if (solicitudParsed[0]=='e'){
            
            
            if(carros == NULL){
                carros = (struct conj *)malloc(sizeof(struct conj));
                inicializarConj(carros, identificador);
                numPuestosOcupados++;
                solicitudParsed[0] = 's'; //Sí se puede ejecutar la acción
                
                //Si quedan puestos y el carro no está ya en el conjunto
            } else if ((numPuestosOcupados < NUM_MAX_PUESTOS) && (insertarEnConj(carros, identificador) == 0)){
                numPuestosOcupados++;
                solicitudParsed[0] = 's'; //Sí se puede ejecutar la acción
            
            } else{
                solicitudParsed[0] = 'n'; //No se puede ejecutar la acción
            }
            
            //Se registra la operación de entrada
            fprintf(archivoBitacoraEntrada, "%s\n",solicitudParsed);
            
            
        } else if (solicitudParsed[0] == 's'){
            
            //Si hay carros y el carro que va a salir está en el conjunto
            if ((numPuestosOcupados > 0) && (eliminarEnConj(&carros, identificador)==1)){
                numPuestosOcupados--;
                solicitudParsed[0] = 's'; //Sí se puede ejecutar la acción
            }
            else{
                solicitudParsed[0] = 'n'; //No se puede ejecutar la acción
            }
            
            //Se registra la operación de salida
            fprintf(archivoBitacoraSalida, "%s\n",solicitudParsed);
        
        }
        printf("Puestos Ocupados: %d\n", numPuestosOcupados);
        
        //Se envía el mensaje de respuesta
        ssize_t numBytesEnviados = sendto(socketSrvdrClt, 
                                        solicitudParsed, 
                                        numBytesRecibidos,
                                        0,
                                        (struct sockaddr *) &dirClnt, 
                                        tamanoSocket);
        
        if (numBytesEnviados < 0){
            fprintf(stderr,"Error al enviar respuesta.\n");
            abort();
        } else if (numBytesEnviados != numBytesRecibidos){
            fprintf(stderr," No se envió el número correcto de bytes.\n");
        }
        
        imprimirConj(carros);
        
        free(solicitudParsed);
        solicitudParsed = (char *)calloc(LON_MAX_MENSAJE, 1);
        
    }
    
    free(solicitudParsed);
    
    return 0;
}


