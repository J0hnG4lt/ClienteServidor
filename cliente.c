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
#define LON_MAX_MENSAJE 100
#define TAM_MAX_ID 1000

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
                    abort();
                }
                direccion = optarg;
                break;
             case 'p':
                if (strlen(optarg) != 4){
                    fprintf(stderr,"Número de puerto de tamaño equivocado.\n");
                    abort();
                } else if (!atoi(optarg)){
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
    
    char *separador = (char *)malloc(sizeof(char));
    *separador = ';';
    char *mensaje = malloc(strlen(accion)+strlen(identificador)+3);
    strncat(mensaje, accion, strlen(accion));
    strncat(mensaje, separador, strlen(separador));
    strncat(mensaje, identificador, strlen(identificador));
    strncat(mensaje, separador, strlen(separador));
    
    struct addrinfo infoDir;
    memset(&infoDir, 0, sizeof(infoDir));
    infoDir.ai_family = AF_UNSPEC;
    infoDir.ai_socktype = SOCK_DGRAM;
    infoDir.ai_protocol = IPPROTO_UDP;
    
    struct addrinfo *dirServ;
    int codigoErr;
    if ((codigoErr =getaddrinfo(direccion, puerto, &infoDir, &dirServ)) != 0){
        fprintf(stderr," Problema al obtener información sobre el servidor.\n");
        gai_strerror(codigoErr);
        abort();
    }
    
    int socketCltSrvdr = socket(dirServ->ai_family, dirServ->ai_socktype, dirServ->ai_protocol);
    if (!socketCltSrvdr){
        fprintf(stderr," Problema al crear el socket.\n");
        abort();
    }
    
    
    ssize_t numBytesEnviados=-1;
    int i=0;
    while((i < NUM_INTENTOS) && (numBytesEnviados==-1)){
        numBytesEnviados = sendto(socketCltSrvdr, mensaje, LON_MAX_MENSAJE, 0, dirServ->ai_addr, dirServ->ai_addrlen);
        i++;
    }
    
    if (!socketCltSrvdr){
        fprintf(stderr," Problema al enviar información.\n");
        abort();
    }// else if (numBytesEnviados != (size_t)strlen(mensaje)) {
     //   fprintf(stderr," No se envió el número correcto de bytes.\n");
     //   abort();
     //}
    
    
    struct sockaddr_storage dirOrigenServ;
    char mensajeRecibido[LON_MAX_MENSAJE + 1];
    socklen_t tamanoSocket = sizeof(dirOrigenServ);
    ssize_t numBytesRecibidos = recvfrom(socketCltSrvdr, mensajeRecibido, 
                                    LON_MAX_MENSAJE, 0,
                                    (struct sockaddr *) &dirOrigenServ, 
                                    &tamanoSocket);
    
    
    if (!numBytesRecibidos ){
        fprintf(stderr," No se recibió ningún mensaje.\n");
        abort();
    }// else if (numBytesRecibidos != numBytesEnviados) {
     //   fprintf(stderr," No se recibió el número correcto de bytes.\n");
     //   abort();
     //}
    
    char *permisoEntrar = strdup(strtok(mensajeRecibido, separador));
    char *identificadorRcbd = strdup(strtok(NULL, separador));
    char *horaFecha = strdup(strtok(NULL, separador));
    
    if (strcmp(identificadorRcbd, identificador) != 0){
        fprintf(stderr," Respuesta recibida no corresponde al conductor.\n");
        abort();
    }
    
    mensajeRecibido[LON_MAX_MENSAJE] = '\0';
    printf("Mensaje Recibido %s\n", mensajeRecibido);
    printf("Permiso: %s, Identificador: %s, Hora: %s\n", permisoEntrar, identificadorRcbd, horaFecha);
    freeaddrinfo(dirServ);
    
    close(socketCltSrvdr);
    
    printf("Mensaje: %s\n", mensaje);
    
    free(mensaje);
    
    printf("ID:%s, Puerto:%s, Accion:%s, Direccion:%s\n", identificador, puerto, accion, direccion);
    
    return 0;
}
