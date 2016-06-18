#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include "conjunto.h"

#define LON_MAX_STRNG 50
#define LON_MAX_DIR 50
#define LON_MAX_ID 50
#define NUM_INTENTOS 3
#define LON_MAX_MENSAJE 50
#define NUM_MAX_PUESTOS 30
#define TAM_MAX_ID 1000




int main(int argc, char **argv){
    
    int numPuestosOcupados = 0;
    struct conj *carros = NULL;
    char separador = ':';
    
    if (argc != 7){
        fprintf(stderr,"Número incorrecto de argumentos.\nUso correcto:\n");
        fprintf(stderr,"\tsem_svr -l <puerto a servir>\n \t\t-i <bitácora de entrada>\n \t\t-o <bitácora de salida>\n");
        abort();
    }
    
    char *bitacoraEntrada=NULL;
    char *bitacoraSalida=NULL;
    char *puerto=NULL;
    char *solicitudParsed = (char *)calloc(LON_MAX_MENSAJE, 1);
    char *accion = NULL;
    char *identificador = NULL;
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
                } else if (!atoi(optarg)){
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
    
    /*
    char *mensaje = malloc(strlen(accion)+strlen(identificador)+1);
    strncat(mensaje, accion, strlen(accion));
    strncat(mensaje, identificador, strlen(identificador));
    */
    
    struct addrinfo infoDir;
    memset(&infoDir, 0, sizeof(infoDir));
    infoDir.ai_family = AF_UNSPEC;
    infoDir.ai_socktype = SOCK_DGRAM;
    infoDir.ai_protocol = IPPROTO_UDP;
    infoDir.ai_flags = AI_PASSIVE;

    
    struct addrinfo *dirServ;
    int codigoErr;
    if ((codigoErr = getaddrinfo(NULL, puerto, &infoDir, &dirServ)) != 0){
        fprintf(stderr," Problema al obtener información sobre el servidor.\n");
        gai_strerror(codigoErr);
        abort();
    }
    
    int socketSrvdrClt = socket(dirServ->ai_family, dirServ->ai_socktype, dirServ->ai_protocol);
    if (!socketSrvdrClt){
        fprintf(stderr," Problema al crear el socket.\n");
        abort();
    }
    
    
    if (bind(socketSrvdrClt, dirServ->ai_addr, dirServ->ai_addrlen) != 0){
        fprintf(stderr,"No se pudo enlazar el socket.\n");
        abort();
    }
    
    
    freeaddrinfo(dirServ);
    
    
    while(1){
        
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
        
        solicitud[strlen(solicitud)] = '\0';
        accion = strdup(strtok(solicitud, &separador));
        identificador = strdup(strtok(NULL, &separador));
        strncpy(solicitudParsed, accion, strlen(accion));
        strncat(solicitudParsed, identificador, strlen(identificador));
        printf("Solicitud: %s\n", solicitudParsed);
        
        if (solicitudParsed[0]=='e'){
            printf("Entrada\n");
            if(carros == NULL){
                carros = (struct conj *)malloc(sizeof(struct conj));
                inicializarConj(carros, identificador);
                printf("Carro inicializado\n");
                numPuestosOcupados++;
                solicitudParsed[0] = 's';
                
            } else if ((numPuestosOcupados < NUM_MAX_PUESTOS) && (insertarEnConj(carros, identificador) == 0)){
                numPuestosOcupados++;
                solicitudParsed[0] = 's';
                printf("Insertado\n");
            
            } else{
                solicitudParsed[0] = 'n';
                printf("No Insertado\n");
            }
        } else if (solicitudParsed[0] == 's'){
            if ((numPuestosOcupados > 0) && (eliminarEnConj(&carros, identificador)==0)){
                numPuestosOcupados--;
                solicitudParsed[0] = 's';
            }
            else{
                solicitudParsed[0] = 'n';
            }
        }
        printf("Puestos Ocupados: %d\n", numPuestosOcupados);
        
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
