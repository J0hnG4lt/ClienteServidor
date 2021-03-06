/**
 * Programa principal, que operaría en el computador central del estacionamiento.
 * Lleva la cuenta de la cantidad de carros dentro, y guarda sus identificadores
 * en un conjunto. Responde las solicitudes de los clientes y las registra en
 * las bitácoras.
 * 
 * @author Alfredo Fanghella, 12-10967
 * @author Georvic Tur, 12-11402
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include "mensajes.h"
#include "conjunto_hash.h"
#include "configuracion.h"

//Variables Globales
FILE *archivoBitacoraEntrada = NULL;
FILE *archivoBitacoraSalida = NULL;
int socketSrvdrClt = -1;

void manejadorINTERRUPT(int num);
uint32_t obtenerPrecio(time_t duracion);

/**
 * Rutina principal del proceso servidor.
 * 
 * @param argc Número de argumentos
 * @param argv Debe contener el puerto a utilizar y los nombres de las bitácoras
 */
int main(int argc, char **argv){
    
    char uso_correcto[] = "sem_svr\n\t-l <puerto a servir>\n\t\
-i <bitácora de entrada>\n\t\
-o <bitácora de salida>\n";
    
    if (argc != 7){
        fprintf(stderr,"Número incorrecto de argumentos.\nUso correcto:\n");
        fprintf(stderr, uso_correcto);
        exit(EXIT_FAILURE);
    }
    
    //Argumentos del servidor
    char *bitacoraEntrada=NULL;
    char *bitacoraSalida=NULL;
    char *puerto=NULL;

    //Se registra el manejador
    signal(SIGINT, manejadorINTERRUPT);
    
    //Estructura para obtener el tiempo. Luego se pone en el mensaje de respuesta.
    time_t tiempo;
    char *tiempoString;
    struct tm *structTiempo;
    
    // Variables para chequeo de la entrada
    bool flag_i, flag_l, flag_o;
    flag_i = flag_l = flag_o = 0;
    char *err;
    long numPuerto;
    
    //Se parsean los argumentos
    int opcn;
    while ((opcn = getopt(argc,argv, "l:i:o:")) != -1){
        switch(opcn){
            case 'i':
                bitacoraEntrada = optarg;
                flag_i = true;
                break;
             case 'l':
                numPuerto = strtol(optarg, &err, 10);
                if ((*err != '\0') || (numPuerto < 1) || (numPuerto > MAX_PUERTO)) {
                    fprintf(stderr, "El puerto no es válido\n");
                    exit(EXIT_FAILURE);
                }
                puerto = optarg;
                flag_l = true;
                break;
             case 'o':
                 bitacoraSalida = optarg;
                 flag_o = true;
                 break;
            default:
                fprintf(stderr,"Formato de argumentos incorrecto.\nUso correcto:\n");
                fprintf(stderr, uso_correcto);
                exit(EXIT_FAILURE);
        }
    }
    
    if (!(flag_o && flag_l && flag_i)) {
        fprintf(stderr,"Formato de argumentos incorrecto.\nUso correcto:\n");
        fprintf(stderr, uso_correcto);
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
    socketSrvdrClt = socket(dirServ->ai_family, dirServ->ai_socktype, dirServ->ai_protocol);
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
    mensaje_c solicitud;
    mensaje_s structRespuesta;
    char accion;                //Entrar o Salir del estacionamiento
    uint32_t identificador;     //ID del carro actual
    int numPuestosOcupados = 0; //Número de puestos ocupados
    conjunto carros;            //Conjunto de carros en el estacionamiento
    inicializarConj(&carros);
    time_t tiempoEstacionado;
    //Se repite por cada mensaje entrante
    
    while(1){
        
        //Se recibe el mensaje de algún cliente
        struct sockaddr_storage dirClnt;
        socklen_t tamanoSocket = sizeof(dirClnt);
        ssize_t numBytesRecibidos = recvfrom(
			socketSrvdrClt, 
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
        identificador = ntohl(solicitud.ident);
               
        //Se obtiene el tiempo actual
        tiempo = time(NULL);
        tiempoString = asctime(localtime(&tiempo));
        structTiempo = localtime(&tiempo);
        
        printf("------------------------------------\n");
        printf(
            "Recibida solicitud de %lu en %s\n", 
            (unsigned long)identificador, tiempoString
        );
        printf("------------------------------------\n");
        
        //Se construye el mensaje de respuesta
        ///////////////////////////////////////////
        structRespuesta.ident = solicitud.ident;
        structRespuesta.dia = ((uint8_t) structTiempo->tm_mday);
        structRespuesta.mes = ((uint8_t) structTiempo->tm_mon)+1;
        structRespuesta.anyo = (((uint8_t) structTiempo->tm_year)); //tm_year tiene el anyo mal
        structRespuesta.hora = ((uint8_t) structTiempo->tm_hour);
        structRespuesta.minuto = ((uint8_t) structTiempo->tm_min);
        //Si el cliente quiere salir o entrar
        if (accion == 'e'){
                //Si quedan puestos y el carro no está ya en el conjunto
            if ((numPuestosOcupados < NUM_MAX_PUESTOS) && 
                (insertarEnConj(&carros, identificador, tiempo) == true))
            {
                numPuestosOcupados++;
                structRespuesta.accion = 's';//Sí se puede ejecutar la acción
            
            } else if (numPuestosOcupados < NUM_MAX_PUESTOS){
                structRespuesta.accion = 'f';//Se reenvía el ticket
                
            } else {
                structRespuesta.accion = 'n';
            }
          
            structRespuesta.precio = htonl((uint32_t) 0);
            
            //Se registra la operación de entrada
            fprintf(archivoBitacoraEntrada, 
				"Entrada: %c. Identificador: %lu. Tiempo: %s",
                structRespuesta.accion,
                (unsigned long)identificador,
                tiempoString);
        } else if (accion == 's'){
            //Si hay carros y el carro que va a salir está en el conjunto
            if ((numPuestosOcupados > 0) && 
                (eliminarEnConj(&carros,identificador,&tiempoEstacionado)==true))
            {
                numPuestosOcupados--;
                structRespuesta.accion = 's';//Sí se puede ejecutar la acción
                structRespuesta.precio = htonl(obtenerPrecio(tiempo - tiempoEstacionado));
                //printf("PRECIO: %d\n", obtenerPrecio(tiempo-*tiempoEstacionado));
                
                //Se registra la operación de salida
                fprintf(archivoBitacoraSalida, 
					"Salida: %c. Identificador: %lu. Monto: Bs. %d.  Tiempo: %s",
                    structRespuesta.accion,
                    (unsigned long)identificador,
                    obtenerPrecio(tiempo-tiempoEstacionado),
                    tiempoString);
            }
            else{
                structRespuesta.accion = 'v';//No se puede ejecutar la acción
                
                fprintf(archivoBitacoraSalida, 
					"Ticket inválido con Identificador: %lu Recibido el %s",
                    (unsigned long)identificador,
                     tiempoString);
            }
        }
 
        //Se envía el mensaje de respuesta
        ssize_t numBytesEnviados = sendto(
			socketSrvdrClt, 
			&structRespuesta,
			(sizeof structRespuesta - sizeof structRespuesta.pad),
			0,
			(struct sockaddr *) &dirClnt,
			tamanoSocket);
        
        if (numBytesEnviados < 0){
            fprintf(stderr,"Error al enviar respuesta: %s.\n", strerror(errno));
            exit(EXIT_FAILURE);
        }        
    }
    
    return 0;
}

/**
 * Calcula cuanto debe pagar el cliente al salir del estacionamiento.
 * 
 * @param duracion Tiempo total de estadía del cliente
 * @return monto que el cliente debe pagar
 */
uint32_t obtenerPrecio(time_t duracion){
    int horas = duracion / 3600;
    if (duracion % 3600 != 0)
        horas++;
    return 80 + 30*(horas - 1);
}

/**
 * Manejador de señales para manejar el SIGINT que generá el Ctrl-c y 
 * terminar el programa limpiamente.
 * 
 * @param código de señal. Parámetro requerido por el SO.
 */
void manejadorINTERRUPT(int num){
    fclose(archivoBitacoraEntrada);
    fclose(archivoBitacoraSalida);
    close(socketSrvdrClt);
    printf("\n----------------\n");
    printf("Servidor apagado\n");
    printf("----------------\n");
    exit(0);
}
