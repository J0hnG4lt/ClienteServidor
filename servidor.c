#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>


#define LON_MAX_STRNG 50
#define LON_MAX_DIR 50
#define LON_MAX_ID 50
#define NUM_INTENTOS 3
#define LON_MAX_MENSAJE 50

int main(int argc, char **argv){
    
    if (argc != 7){
        fprintf(stderr,"Número incorrecto de argumentos.\nUso correcto:\n");
        fprintf(stderr,"\tsem_svr -l <puerto a servir>\n \t\t-i <bitácora de entrada>\n \t\t-o <bitácora de salida>\n");
        abort();
    }
    
    char *bitacoraEntrada=NULL;
    char *bitacoraSalida=NULL;
    char *puerto=NULL;
    
    int opcn;
    while ((opcn = getopt(argc,argv, "l:i:o:")) != -1){
        switch(opcn){
            case 'i':
                if (LON_MAX_STRNG < strlen(optarg)){
                    fprintf(stderr,"Bitácora de Entrada: Longitud máxima de nombre sobrepasada.\n");
                    free(bitacoraSalida);
                    free(puerto);
                    abort();
                }
                bitacoraEntrada = strdup(optarg);
                break;
             case 'l':
                if (strlen(optarg) != 4){
                    fprintf(stderr,"Número de puerto de tamaño equivocado.\n");
                    free(bitacoraEntrada);
                    free(bitacoraSalida);
                    abort();
                } else if (!atoi(optarg)){
                    fprintf(stderr,"El puerto ha de ser un número.\n");
                    free(bitacoraEntrada);
                    free(bitacoraSalida);
                    abort();
                }
                
                puerto = strdup(optarg);
                break;
             case 'o':
                 if (LON_MAX_STRNG < strlen(optarg)){
                     fprintf(stderr,"Bitácora de Salida: Longitud máxima de nombre sobrepasada.\n");
                     free(bitacoraEntrada);
                     free(puerto);
                     abort();
                 }
                 bitacoraSalida = strdup(optarg);
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
        free(bitacoraSalida);
        free(bitacoraEntrada);
        free(puerto);
        gai_strerror(codigoErr);
        abort();
    }
    
    int socketSrvdrClt = socket(dirServ->ai_family, dirServ->ai_socktype, dirServ->ai_protocol);
    if (!socketSrvdrClt){
        fprintf(stderr," Problema al crear el socket.\n");
        free(bitacoraSalida);
        free(bitacoraEntrada);
        free(puerto);
        abort();
    }
    
    
    if (bind(socketSrvdrClt, dirServ->ai_addr, dirServ->ai_addrlen) != 0){
        fprintf(stderr,"No se pudo enlazar el socket.\n");
        free(bitacoraSalida);
        free(bitacoraEntrada);
        free(puerto);
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
            free(bitacoraSalida);
            free(bitacoraEntrada);
            free(puerto);
            abort();
        }
        
        solicitud[strlen(solicitud)] = '\0';
        printf("Solicitud: %s\n", solicitud);
        
        if (solicitud[0]=='s'){
            break;
        }
        
    }
    
    /*
    ssize_t numBytesEnviados=-1;
    int i=0;
    while((i < NUM_INTENTOS) && (numBytesEnviados==-1)){
        numBytesEnviados = sendto(socketCltSrvdr, mensaje, (size_t)strlen(mensaje), 0, dirServ->ai_addr, dirServ->ai_addrlen);
        i++;
    }
    
    if (!socketSrvdrClt){
        fprintf(stderr," Problema al enviar información.\n");
        free(puerto);
        free(identificador);
        free(accion);
        free(direccion);
        abort();
    } else if (numBytesEnviados != (size_t)strlen(mensaje)) {
        fprintf(stderr," No se envió el número correcto de bytes.\n");
        free(puerto);
        free(identificador);
        free(accion);
        free(direccion);
        abort();
    }
    
    
    struct sockaddr_storage dirOrigenServ;
    char mensajeRecibido[LON_MAX_MENSAJE + 1];
    socklen_t tamanoSocket = sizeof(dirOrigenServ);
    ssize_t numBytesRecibidos = recvfrom(socketCltSrvdr, mensajeRecibido, 
                                    LON_MAX_MENSAJE, 0,
                                    (struct sockaddr *) &dirOrigenServ, 
                                    &tamanoSocket);
    
    
    if (!numBytesRecibidos ){
        fprintf(stderr," No se recibió ningún mensaje.\n");
        free(puerto);
        free(identificador);
        free(accion);
        free(direccion);
        abort();
    } else if (numBytesRecibidos != numBytesEnviados) {
        fprintf(stderr," No se recibió el número correcto de bytes.\n");
        free(puerto);
        free(identificador);
        free(accion);
        free(direccion);
        abort();
    }
    
    mensajeRecibido[strlen(mensaje)] = '\0';
    printf("Mensaje Recibido %s\n", mensajeRecibido);
    
    freeaddrinfo(dirServ);
    
    close(socketCltSrvdr);
    
    printf("Mensaje: %s\n", mensaje);
    
    free(mensaje);
    */
    
    
    free(bitacoraEntrada);
    free(bitacoraSalida);
    free(puerto);
    
    return 0;
}
