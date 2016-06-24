#include "configuracion.h"
#include "cliente.h"

// Autores: Georvic Tur           12-11402
//          Alfredo Fanghella     12-10967

//Constantes definidas en el archivo de configuración
#define MAX_PUERTO 65535
int main(int argc, char **argv){
    
    if (argc != 9){
        fprintf(stderr,"Número incorrecto de argumentos.\nUso correcto:\n");
        fprintf(stderr,"\tsem_cli \
                -d <dirección IP o nombre de dominio>\n \t\t\
                -p <puerto>\n \t\t\
                -c <acción>\n \t\t\
                -i <identificador de vehículo>\n");
        exit(EXIT_FAILURE);
    }
    
    //Argumentos del cliente
    char *direccion = NULL; //Nombre de dominio o IP
    char *puerto = NULL; //Número de Puerto
    unsigned long identificador;
    char *identificadorStr;
    
    mensaje msj; // mensaje que se enviará al servidor
    mensaje structRespuesta;
    
    int opcn; //Se parsean los argumentos del cliente
              //optarg es una variable global con un apuntador al argumento actual
    char * err;
	long numPuerto;
    while ((opcn = getopt(argc,argv, "d:p:c:i:")) != -1){
        switch(opcn){
            case 'd':
                if (LON_MAX_DIR < strlen(optarg)){
                    fprintf(stderr,"Longitud máxima de dirección sobrepasada.\n");
                    exit(EXIT_FAILURE);
                }
                direccion = optarg;
                break;
            case 'p':
				numPuerto = strtol(optarg, &err, 10);
				if ((*err != '\0') || (numPuerto < 1) || (numPuerto > MAX_PUERTO)) {
					fprintf(stderr, "El puerto no es válido\n");
					exit(EXIT_FAILURE);
				}
                puerto = optarg;
                break;
            case 'c':
                if ((strlen(optarg) != 1) || ((*optarg != 'e') && (*optarg != 's'))){
                    fprintf(stderr,"Tipo de acción equivocada. Debe ser 'e' para entrada o 's' para salida\n");
                    exit(EXIT_FAILURE);
                }
                msj.accion = *optarg;
                break;
            case 'i':
                if (optarg[0] == '-'){
                    fprintf(stderr,"El identificador debe ser un número positivo.\n");
                    exit(EXIT_FAILURE);
                } 
                errno = 0;
                identificador = strtoul(optarg, &err, 10);
                if (*err!= '\0') {
					fprintf(stderr,"El número de identificación no es válido.\n");
					exit(EXIT_FAILURE);
				} else if ((errno == ERANGE) || (identificador > UINT32_MAX)){
                    fprintf(stderr,"El número de identificación es demasiado grande.\n");
                    exit(EXIT_FAILURE);
                }
                msj.ident = htonl(identificador);
                identificadorStr = optarg;
                break;
             
             default:
                exit(EXIT_FAILURE);
        }
        
    }
    
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
        exit(EXIT_FAILURE);
    }
    
    
    //Creación del socket
    int socketCltSrvdr = socket(dirServ->ai_family, dirServ->ai_socktype, dirServ->ai_protocol);
    if (socketCltSrvdr == -1){
        fprintf(stderr," Problema al crear el socket.\n");
        exit(EXIT_FAILURE);
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
        exit(EXIT_FAILURE);
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
                                    &msj, 
                                    sizeof msj - sizeof msj.pad, 
                                    0, 
                                    dirServ->ai_addr, 
                                    dirServ->ai_addrlen);
        
        numBytesRecibidos = recvfrom(socketCltSrvdr, &structRespuesta, 
                                    sizeof structRespuesta - sizeof structRespuesta.pad, 0,
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
        exit(EXIT_FAILURE);
    }
    
    /*
    //Respuestas del servidor
    char *separador =";";
    char permisoEntrar = mensajeRecibido[0];
    char *identificadorRcbd = strdup(strtok(&mensajeRecibido[1], separador));
    char *horaFecha = strdup(strtok(NULL, separador));
    */
    
    char permisoEntrar = structRespuesta.accion;
    int identificadorRcbd = ntohl(structRespuesta.ident);//(unsigned long) ntohl(structRespuesta.ident);
    int dia = (int) ntohl(structRespuesta.dia);
    int mes = (int) ntohl(structRespuesta.mes);
    int anyo = (int) ntohl(structRespuesta.anyo);
    int hora = (int) ntohl(structRespuesta.hora);
    int minuto = (int) ntohl(structRespuesta.minuto);
    int precio = (int) ntohl(structRespuesta.precio);
    
    printf("RESPUESTA SERVER: Permiso: %c, Identificador: %d, Hora: %d\n", permisoEntrar, identificadorRcbd, hora);
    
    //Se verifica que el identificador recibido corresponda al cliente actual
    
    printf("%d %lu\n", identificadorRcbd, identificador);
    if (identificadorRcbd != identificador){
        fprintf(stderr," Respuesta recibida no corresponde al conductor.\n");
        abort();
    }
    
    
    mensajeRecibido[LON_MAX_MENSAJE] = '\0';
    printf("Permiso: %c, Identificador: %d, Hora: %d\n", permisoEntrar, identificadorRcbd, hora);
    freeaddrinfo(dirServ);
    
    close(socketCltSrvdr);
    
    //printf("Mensaje: %s\n", mensaje);
    
    //free(mensaje);
    
    //free(horaFecha);
    //free(identificadorRcbd);
    
    //printf("ID:%s, Puerto:%s, Accion:%s, Direccion:%s\n", identificador, puerto,(char) permisoEntrar, direccion);
    
    return 0;
}
