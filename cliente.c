#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>

#define LON_MAX_DIR 50
#define LON_MAX_ID 50
#define NUM_INTENTOS 3
#define LON_MAX_MENSAJE 50

int main(int argc, char **argv){
    
    if (argc != 9){
        fprintf(stderr,"Número incorrecto de argumentos.\nUso correcto:\n");
        fprintf(stderr,"\tsem_cli -d <dirección IP o nombre de dominio>\n \t\t-p <puerto>\n \t\t-c <acción>\n \t\t-i <identificador de vehículo>\n");
        abort();
    }
    
    char *direccion=NULL;
    char *puerto=NULL;
    char *accion=NULL;
    char *identificador=NULL;
    
    int opcn;
    while ((opcn = getopt(argc,argv, "d:p:c:i:")) != -1){
        switch(opcn){
            case 'd':
                if (LON_MAX_DIR < strlen(optarg)){
                    fprintf(stderr,"Longitud máxima de dirección sobrepasada.\n");
                    free(accion);
                    free(puerto);
                    free(identificador);
                    abort();
                }
                direccion = strdup(optarg);
                break;
             case 'p':
                if (strlen(optarg) != 4){
                    fprintf(stderr,"Número de puerto de tamaño equivocado.\n");
                    free(accion);
                    free(direccion);
                    free(identificador);
                    abort();
                } else if (!atoi(optarg)){
                    fprintf(stderr,"El puerto ha de ser un número.\n");
                    free(accion);
                    free(direccion);
                    free(identificador);
                    abort();
                }
                
                puerto = strdup(optarg);
                break;
             case 'c':
                if ((*optarg != 'e') && (*optarg != 's')){
                    fprintf(stderr,"Tipo de acción equivocada. Debe ser 'e' para entrada o 's' para salida\n");
                    free(direccion);
                    free(identificador);
                    free(puerto);
                    abort();
                }
                accion = strdup(optarg);
                break;
             case 'i':
                if (strlen(optarg) > LON_MAX_ID){
                    fprintf(stderr,"Longitud máxima de identificador sobrepasada.\n");
                    free(accion);
                    free(direccion);
                    free(puerto);
                    abort();
                } else if (!atoi(optarg)){
                    fprintf(stderr,"El identificador ha de ser un número.\n");
                    free(accion);
                    free(direccion);
                    free(puerto);
                    abort();
                }
                identificador = strdup(optarg);
                break;
             
             default:
                abort();
        
        }
        
    }
    
    
    char *mensaje = malloc(strlen(accion)+strlen(identificador)+1);
    strncat(mensaje, accion, strlen(accion));
    strncat(mensaje, identificador, strlen(identificador));
    
    
    struct addrinfo infoDir;
    memset(&infoDir, 0, sizeof(infoDir));
    infoDir.ai_family = AF_UNSPEC;
    infoDir.ai_socktype = SOCK_DGRAM;
    infoDir.ai_protocol = IPPROTO_UDP;
    
    struct addrinfo *dirServ;
    
    if (!getaddrinfo(direccion, puerto, &infoDir, &dirServ)){
        fprintf(stderr," Problema al obtener información sobre el servidor.\n");
        free(puerto);
        free(identificador);
        free(accion);
        free(direccion);
        abort();
    }
    
    int socketCltSrvdr = socket(dirServ->ai_family, dirServ->ai_socktype, dirServ->ai_protocol);
    if (!socketCltSrvdr){
        fprintf(stderr," Problema al crear el socket.\n");
        free(puerto);
        free(identificador);
        free(accion);
        free(direccion);
        abort();
    }
    
    
    ssize_t numBytesEnviados=-1;
    int i=0;
    while((i < NUM_INTENTOS) && (numBytesEnviados==-1)){
        numBytesEnviados = sendto(socketCltSrvdr, mensaje, (size_t)strlen(mensaje), 0, dirServ->ai_addr, dirServ->ai_addrlen);
        i++;
    }
    
    if (!socketCltSrvdr){
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
    
    printf("ID:%s, Puerto:%s, Accion:%s, Direccion:%s\n", identificador, puerto, accion, direccion);
    free(direccion);
    free(accion);
    
    return 0;
}
