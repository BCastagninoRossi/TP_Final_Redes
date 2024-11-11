#include "tcp_server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>


#include <sys/types.h>


#define MAX_PDU_SIZE         10000

#define PDU_CANDIDATE_LINE_OK	 1
#define PDU_ERROR_BAD_FORMAT	-1
#define PDU_NEED_MORE_DATA       0


// #define DBG_RECEIVED_DATA
int tcp_parser(int rec_bytes, char buffer[MAX_PDU_SIZE] );
int processReceivedData(char *buffer, int buffersize, int *buffer_ptr, char *pdu_candidate, int *pdu_candidate_ptr);

int tcp_parser(int rec_bytes, char buffer[MAX_PDU_SIZE])
{
    /*mostrar el buffer*/
    printf("Buffer: %s\n", buffer);
  /* PDU candidate status */
  int pdu_status;
  /* Pointers used for PDU parsing */
  int buffer_ptr, pdu_candidate_ptr;
  /* Buffer used to store PDU candidate, it can be filled by several TCP segments */
  char pdu_candidate[MAX_PDU_SIZE+1];

  /* Initialization */
  pdu_candidate_ptr = 0;
  memset(pdu_candidate,0,sizeof(pdu_candidate));

//   /* Buffer array for TCP read mockup */
//   char *test_buffers[ARRAY_SIZE] = {
//     "usuario1\x02timestamp1\x02mensaje1\x04",
//     "usuario2\x02timestamp2\x02mensaje2\x04usuario3\x02timestamp3\x02mensaje3\x04",
//     "usuario4\x02timestamp4\x02mensaje4\x04usuario5\x02timestamp5\x02mensaje5\x04usuario6\x02timestamp6\x02mensaje6\x04",
//     "usuario7\x02timestamp7\x02mensaje7_incomplete"
//   };

  /*  This for() emulates the reading loop */


// Read mockup of a TCP socket new_s

    buffer_ptr = 0; 

    // String para guardar los campos de la PDU
    char PDU_data[10000];
    // This loop processes data and its existence provides
    // - complete and partial PDU parsing
    while (rec_bytes - buffer_ptr -1  > 0) // la cantidad de bytes que se leyeron menos la cantidad de bytes que ya se procesaron
    {
        pdu_status = processReceivedData((char *) buffer, rec_bytes, &buffer_ptr, (char *) pdu_candidate, &pdu_candidate_ptr);  // ---> Aca ve si hay un PDU completo, detecta los delimitadores y corta el buffers
        if (pdu_status == PDU_CANDIDATE_LINE_OK)  // ---> Aca se fija si el PDU es correcto
        {
        // Here we should try to parse the candidate PDU
        printf( "try_parse PDU\n");    // Just for debug, erase it if convenient
        printf("%s\n", pdu_candidate); // Just for debug, erase it if convenient
        
        // Parse PDU with ' ' as delimiter
        char *usuario = strtok(pdu_candidate, " ");
        char *timestamp = strtok(NULL, " ");
        char *mensaje = strtok(NULL, " ");

        // Guardar los campos de la PDU en un string
        sprintf(PDU_data, "Usuario: %s, Timestamp: %s, Mensaje: %s", usuario, timestamp, mensaje);
        printf("PDU_data: %s\n", PDU_data);
        
        // Memory cleaning
        pdu_candidate_ptr = 0;
        memset(pdu_candidate,0,sizeof(pdu_candidate));
        }
        else if (pdu_status == PDU_ERROR_BAD_FORMAT) // ---> caso en el que el PDU no es correcto
        {
        printf("ERROR clean memory buffer\n"); // Just for debug, erase it if convenient
        // Memory cleaning
        pdu_candidate_ptr = 0; 
        memset(pdu_candidate,0,sizeof(pdu_candidate));
        }
        else // ---> caso en el que no se detecta un delimitador, posiblemente porque el mensaje sigue en otro buffer
        {
        // Just for debug, erase it if convenient
        printf("%s\n", pdu_candidate);
        printf("No delimiter found. Probably need another buffer read\n");
        }
        // Just for debug, erase it if convenient
        printf("\n");
        fprintf(stderr,"processReceivedData = %d\n", pdu_status);
        fprintf(stderr,"inbytes=%d  buffer_ptr=%d\n", rec_bytes, buffer_ptr);
    }


    return 0;
}

int processReceivedData(char *buffer, int buffersize, int *buffer_ptr, char *pdu_candidate, int *pdu_candidate_ptr)
{ 
  // Position pdu_candidate pointer
  pdu_candidate += *pdu_candidate_ptr;
  // Position buffer pointer
  buffer += *buffer_ptr;

  int i;
  int chunk = (buffersize - *buffer_ptr);
  for (i=0; i < chunk; i++)
  {
    (*buffer_ptr)++; // must be before a return gets hit

    #ifdef DBG_RECEIVED_DATA
      if ( *buffer >= 32 && *buffer <=126 )
        fprintf(stderr,"Processing char (%d) '%c'\n", *buffer, *buffer);
      else
        fprintf(stderr,"Processing char (%d) \n", *buffer);
      if ( *pdu_candidate >= 32 && *pdu_candidate <=126 )
        fprintf(stderr,"pdu_candidate char (%d) '%c'\n", *pdu_candidate, *pdu_candidate);
      else
        fprintf(stderr,"pdu_candidate char (%d)\n", *pdu_candidate);
      fprintf(stderr,"buffer_ptr '%d'  pdu_candidate_ptr '%d'\n", *buffer_ptr, *pdu_candidate_ptr);
    #endif

    // PDU candidate criteria: \x04 (MSEP)
    if ( *pdu_candidate_ptr >= 1 && *buffer == 0x04)
    {
      pdu_candidate++; // Advance pointer for next PDU (in 2016 was not a requirement)
      *pdu_candidate++ = ' '; // Replace delimiter with space
      (*pdu_candidate_ptr)++;
      #ifdef DBG_RECEIVED_DATA
         fprintf(stderr,"MSEP detected PDU_CANDIDATE_LINE_OK\n");
      #endif
      return PDU_CANDIDATE_LINE_OK;
    }
    else if (*buffer !='\x02' && *buffer != '\x04' && (*buffer < 32 || *buffer >126) ) // http://www.asciitable.com/
    {
      // Forbidden characters
      // pdu_candidate_ptr must be taken care outside this function
      #ifdef DBG_RECEIVED_DATA
        fprintf(stderr,"Char outside of allowed range PDU_ERROR_BAD_FORMAT\n");
      #endif
      return PDU_ERROR_BAD_FORMAT;
    }
    else
    {
      #ifdef DBG_RECEIVED_DATA
      if ( *buffer >= 32 && *buffer <=126 )
        fprintf(stderr,"Char (%d) '%c' processed OK\n", *buffer, *buffer);
      else
        fprintf(stderr,"Char (%d) processed OK\n", *buffer);
      #endif
      if (*buffer == '\x02' || *buffer == '\x04') {
        *pdu_candidate = ' '; // Replace delimiter with space
      } else {
        *pdu_candidate = *buffer;
      }
      (*pdu_candidate_ptr)++;
      pdu_candidate++;
    }
    buffer++;
  }
  return PDU_NEED_MORE_DATA;
}










int init_tcp_server() {
    int server_fd;
    struct sockaddr_in address;

    // Crear el socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Error al crear el socket");
        exit(EXIT_FAILURE);
    }

    // Configurar la estructura de la dirección
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Enlazar el socket al puerto
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Error al enlazar el socket");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Poner el socket en modo de escucha
    // El segundo argumento es el número máximo de conexiones pendientes en la cola
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("Error al poner en escucha");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Servidor en escucha en el puerto %d\n", PORT);
    return server_fd;
}

void *client_handler(void* client_data) {
    client_t * client = (client_t *)client_data;
    int client_id = client->id;
    int client_fd =*(client->fd);
    free (client->fd);
    handle_client(client_fd, client_id);
    close(client_fd);
    return NULL;
}

void accept_connections(int server_fd) {
    struct sockaddr_in client_address;
    int addr_len = sizeof(client_address);
    int *client_fd;
    
    // iniciar contador de clientes
    int client_id = 0;
    while (1) {
        // Aceptar una nueva conexión
        client_fd = malloc(sizeof(int));
        *client_fd = accept(server_fd, (struct sockaddr *)&client_address, (socklen_t*)&addr_len);
        if (*client_fd < 0) {
            perror("Error al aceptar conexión");
            free(client_fd);
            continue;
        }
        printf("Cliente conectado\n");
        client_id++;

        //estructura para almacenar la informacion del cliente
        client_t client;
        client.id = client_id;
        client.fd = client_fd;

        // Crear un hilo para manejar la conexión del cliente
        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, client_handler, (void *)&client) != 0) {
            perror("Error al crear hilo");
            free(client_fd);
            continue;
        }
        pthread_detach(thread_id); // El hilo se limpiará automáticamente al terminar
    }
}


void handle_client(int client_fd, int client_id) {
    char buffer[BUFFER_SIZE];
    int bytes_read;

    // Recibir datos del cliente
    while ((bytes_read = recv(client_fd, buffer, BUFFER_SIZE, 0)) > 0) {
        buffer[bytes_read] = '\0'; // Agregar terminador de cadena
        printf("Chat ID: %d\nMensaje recibido: %s\n",client_id, buffer);

        // TODO: Procesar el mensaje
        // Aquí puedes llamar a otras funciones para procesar el mensaje,
        // como parsear el contenido o enviar el mensaje a la API de análisis de sentimientos.
        tcp_parser(bytes_read, buffer);
        
        // Enviar respuesta al cliente (por ejemplo, un ACK)
        char *response = "Mensaje recibido\n";
        send(client_fd, response, strlen(response), 0);
    }

    if (bytes_read == 0) {
        printf("Cliente desconectado\n");
    } else if (bytes_read < 0) {
        perror("Error al recibir datos");
    }
}
