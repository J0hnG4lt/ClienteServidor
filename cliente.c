#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <time.h>
#include "configuracion.h"

// Autores: Georvic Tur           12-11402
//          Alfredo Fanghella     12-10967

//Constantes definidas en el archivo de configuración

int main(int argc, char **argv){
    
    if (argc != 9){
        fprintf(stderr,"Número incorrecto de argumentos.\nUso correcto:\n");
        fprintf(stderr,"\tsem_cli \
                -d <dirección IP o nombre de dominio>\n \t\t\
                -p <puerto>\n \t\t\
                -c <acción>\n \t\t\
                -i <identificador de vehículo>\n");
        abort();
    }
    
    //Argumentos del cliente
    char *direccion=NULL; //Nombre de dominio o IP
    char *puerto=NULL; //Número de Puerto
    char *accion=NULL; //Entrada o Salida
    char *identificador=NULL; //ID del carro
    
    int opcn; //Se parsean los argumentos del cliente
              //optarg es una variable global con un apuntador al argumento actual
    while ((opcn = getopt(argc,argv, "d:p:c:i:")) != -1){
        switch(opcn){
            case 'd':
                if (LON_MAX_DIR < strlen(optarg)){
                    fprintf(stderr,"Longitud máxima de dirección sobrepasada.\n");
                    abort();
                }
                direccion = optarg;
                break;
             case 'p':
                if (strlen(optarg) != 4){
                    fprintf(stderr,"Número de puerto de tamaño equivocado.\n");
                    abort();
                } else if (!atoi(optarg)){ //Se usa atoi para ver si optarg es un número
                    fprintf(stderr,"El puerto ha de ser un número.\n");
                    abort();
                }
                
                puerto = optarg;
                break;
             case 'c':
                if ((*optarg != 'e') && (*optarg != 's')){
                    fprintf(stderr,"Tipo de acción equivocada. Debe ser 'e' para entrada o 's' para salida\n");
                    abort();
                }
                accion = optarg;
                break;
             case 'i':
                if (strlen(optarg) > LON_MAX_ID){
                    fprintf(stderr,"Longitud máxima de identificador sobrepasada.\n");
                    abort();
                } else if (!atoi(optarg)){
                    fprintf(stderr,"El identificador ha de ser un número.\n");
                    abort();
                }
                identificador = optarg;
                break;
             
             default:
                abort();
        }
        
    }
    
    //Este es el separador para distinguir los elementos del mensaje
    char *separador = (char *)malloc(sizeof(char));
    *separador = ';';
    
    //El mensaje tiene la acción (Entrar o Salir) y el ID del carro
    char *mensaje = malloc(strlen(accion)+strlen(identificador)+3);
    strncat(mensaje, accion, strlen(accion));
    strncat(mensaje, separador, strlen(separador));
    strncat(mensaje, identificador, strlen(identificador));
    strncat(mensaje, separador, strlen(separador));
    
    //El tipo de socket a usar (UDP)
    struct addrinfo infoDir;
    memset(&infoDir, 0, sizeof(infoDir));
    infoDir.ai_family = AF_UNSPEC;
    infoDir.ai_socktype = SOCK_DGRAM;
    infoDir.ai_protocol = IPPROTO_UDP;
    
    
    //Configuracion del socket
    struct addrinfo *dirServ;
    int codigoErr;
    if ((codigoErr=getaddrinfo(direccion, puerto, &infoDir, &dirServ)) != 0){
        fprintf(stderr," Problema al obtener información sobre el servidor.\n");
        gai_strerror(codigoErr);
        abort();
    }
    
    
    //Creación del socket
    int socketCltSrvdr = socket(dirServ->ai_family, dirServ->ai_socktype, dirServ->ai_protocol);
    if (socketCltSrvdr == -1){
        fprintf(stderr," Problema al crear el socket.\n");
        abort();
    }
    
    
    //Estructura para establecer el timeout de la espera por la respuesta
    //del servidor.
    struct timeval tiempoEsperaMax;
    tiempoEsperaMax.tv_sec = TIEMPO_MAX_SEG;
    tiempoEsperaMax.tv_usec = TIEMPO_MAX_USEG;
    
    //Se configura el socket para esperar un tiempo máximo al recibir
    if (setsockopt(socketCltSrvdr, 
                    SOL_SOCKET, 
                    SO_RCVTIMEO, 
                    &tiempoEsperaMax, 
                    sizeof(tiempoEsperaMax))){
        
        fprintf(stderr," Problema al establecer tiempo máximo de espera\n");
        abort();
    }
    
    
    ssize_t numBytesEnviados=-1;
    struct sockaddr_storage dirOrigenServ;
    char mensajeRecibido[LON_MAX_MENSAJE + 1];
    socklen_t tamanoSocket = sizeof(dirOrigenServ);
    ssize_t numBytesRecibidos = 0;
    int i=0;
    
    //Se repite hasta que se envíe y reciba la info o se agoten los intentos
    while((i < NUM_INTENTOS) && ((numBytesEnviados==-1) || (numBytesRecibidos < 0))){
        
        numBytesEnviados = sendto(socketCltSrvdr, 
                                    mensaje, 
                                    LON_MAX_MENSAJE, 
                                    0, 
                                    dirServ->ai_addr, 
                                    dirServ->ai_addrlen);
        
        numBytesRecibidos = recvfrom(socketCltSrvdr, mensajeRecibido, 
                                    LON_MAX_MENSAJE, 0,
                                    (struct sockaddr *) &dirOrigenServ, 
                                    &tamanoSocket);
        
        i++;
    }
    
    
    if (!socketCltSrvdr){
        fprintf(stderr,"Problema al enviar información.\n");
    }
    
    if (numBytesRecibidos == -1){
        fprintf(stderr,"No se recibió ningún mensaje.\n");
    }
    
    if (i==NUM_INTENTOS){
        fprintf(stderr,"Tiempo de respuesta agotado.\n");
        abort();
    }
    
    
    //Respuestas del servidor
    char *permisoEntrar = strdup(strtok(mensajeRecibido, separador));
    char *identificadorRcbd = strdup(strtok(NULL, separador));
    char *horaFecha = strdup(strtok(NULL, separador));
    
    //Se verifica que el identificador recibido corresponda al cliente actual
    if (strcmp(identificadorRcbd, identificador) != 0){
        fprintf(stderr," Respuesta recibida no corresponde al conductor.\n");
        abort();
    }
    
    
    mensajeRecibido[LON_MAX_MENSAJE] = '\0';
    printf("Permiso: %s, Identificador: %s, Hora: %s\n", permisoEntrar, identificadorRcbd, horaFecha);
    freeaddrinfo(dirServ);
    
    close(socketCltSrvdr);
    
    printf("Mensaje: %s\n", mensaje);
    
    free(mensaje);
    
    free(horaFecha);
    free(permisoEntrar);
    free(identificadorRcbd);
    
    printf("ID:%s, Puerto:%s, Accion:%s, Direccion:%s\n", identificador, puerto, accion, direccion);
    
    return 0;
}
