#include "configuracion.h"
#include "cliente.h"

// Autores: Georvic Tur           12-11402
//          Alfredo Fanghella     12-10967

//Constantes definidas en el archivo de configuración

int main(int argc, char **argv){
    
    char uso_correcto[] ="sem_cli -d <dirección IP o nombre de dominio>\n\t\
-p <puerto>\n\t\
-c <acción>\n\t\
-i <identificador de vehículo>\n";
    
    if (argc != 9){
        fprintf(stderr,"Número incorrecto de argumentos.\nUso correcto:\n");
        fprintf(stderr, uso_correcto);
        exit(EXIT_FAILURE);
    }
    
    //Argumentos del cliente
    char *direccion = NULL; //Nombre de dominio o IP
    char *puerto = NULL; //Número de Puerto
    unsigned long identificador;
    
    mensaje_c msj; // mensaje que se enviará al servidor
    mensaje_s structRespuesta;
    
    int opcn; //Se parsean los argumentos del cliente
              //optarg es una variable global con un apuntador al argumento actual
    
    // Variables para el chequeo de la entrada
    char * err;
	long numPuerto;
	bool flag_i, flag_d, flag_p, flag_c;
    flag_i = flag_d = flag_p = flag_c = false;
    while ((opcn = getopt(argc,argv, "d:p:c:i:")) != -1){
        switch(opcn){
            case 'd':
                direccion = optarg;
                flag_d = true;
                break;
            case 'p':
				numPuerto = strtol(optarg, &err, 10);
				if ((*err != '\0') || (numPuerto < 1) || (numPuerto > MAX_PUERTO)) {
					fprintf(stderr, "El puerto no es válido\n");
					exit(EXIT_FAILURE);
				}
                puerto = optarg;
                flag_p = true;
                break;
            case 'c':
                if ((strlen(optarg) != 1) || ((*optarg != 'e') && (*optarg != 's'))){
                    fprintf(stderr,"Tipo de acción equivocada. Debe ser 'e' para entrada o 's' para salida\n");
                    exit(EXIT_FAILURE);
                }
                msj.accion = *optarg;
                flag_c = true;
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
                flag_i = true;
                break;
            default:
				fprintf(stderr,"Formato de argumentos incorrecto.\nUso correcto:\n");
				fprintf(stderr, uso_correcto);
                exit(EXIT_FAILURE);
        }
    }
    
    if (!(flag_d && flag_c && flag_i && flag_p)) {
		fprintf(stderr,"Formato de argumentos incorrecto.\nUso correcto:\n");
		fprintf(stderr, uso_correcto);
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
    socklen_t tamanoSocket = sizeof(dirOrigenServ);
    ssize_t numBytesRecibidos = 0;
    int i=0;
    
    //No se necesita
    //msj.ack=0;
    int identificadorRcbd=-1;
    //Se repite hasta que se envíe y reciba la info o se agoten los intentos.
    while((i < NUM_INTENTOS) && ((numBytesEnviados==-1) || (numBytesRecibidos < 0) || (identificadorRcbd != identificador))){
        //La identidad del cliente también ha de estar en el mensaje de respuesta del server
        
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
        identificadorRcbd = ntohl(structRespuesta.ident);
        
        
    }
    
    
    if (!socketCltSrvdr){
        fprintf(stderr,"Problema al enviar información.\n");
    }
    
    if (numBytesRecibidos == -1){
        fprintf(stderr,"No se recibió ningún mensaje.\n");
    }
    
    if (identificadorRcbd != identificador){
        fprintf(stderr, "No se recibió un mensaje correspondiente al cliente.\n");
    }
    
    if (i==NUM_INTENTOS){
        fprintf(stderr,"Tiempo de respuesta agotado.\n");
        exit(EXIT_FAILURE);
    }
    
    
    char permiso = structRespuesta.accion;
    
    int dia = (int) (structRespuesta.dia);
    int mes = (int) (structRespuesta.mes);
    int anyo = ((int) (structRespuesta.anyo))-100+2000;
    int hora = (int) (structRespuesta.hora);
    int minuto = (int) (structRespuesta.minuto);
    int precio = (int) ntohl(structRespuesta.precio);
    
    
    
    if ((msj.accion == 'e') && (permiso == 's')){
    	printf("Puede Pasar\n");
    	printf("-----------Ticket---------\n");
    	printf("ID: %d\n", identificadorRcbd);
    	printf("Hora: %d:%d\n", hora, minuto);
    	printf("Fecha: %d/%d/%d\n", dia,mes,anyo);
    	printf("---------------------------\n");
    	
    } else if ((msj.accion == 'e') && (permiso == 'n')){
    	printf("En este momento no hay puestos\n");
    	
    } else if ((msj.accion == 's') && (permiso == 's')){
    	printf("---------------------------------\n");
    	printf("ID: %d\n", identificadorRcbd);
    	printf("Debe cancelar: Bs. %d\n", precio);
    	printf("---------------------------------\n");
    	
    } else if ((msj.accion == 's') && (permiso == 'n')){
        printf("<<<<<<<<Factura Reenviada>>>>>>>>\n");
    	printf("---------------------------------\n");
    	printf("ID: %d\n", identificadorRcbd);
    	printf("Debe cancelar: Bs. %d\n", precio);
    	printf("---------------------------------\n");
    	
    } else if ((msj.accion == 'e') && (permiso == 'f')){
    	printf("<<<<<<<Ticket Reenviado>>>>>>>\n");
    	printf("Puede Pasar\n");
    	printf("-----------Ticket----------\n");
    	printf("ID: %d\n", identificadorRcbd);
    	printf("Hora: %d:%d\n", hora, minuto);
    	printf("Fecha: %d/%d/%d\n", dia,mes,anyo);
    	printf("---------------------------\n");
    }
    

    freeaddrinfo(dirServ);
    
    close(socketCltSrvdr);
    
    
    return 0;
}
