#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include "tcp_parser.h"
#include "tcp_server.h"


#include <sys/types.h>

#define DBG_RECEIVED_DATA

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
        client_fd = (int *)malloc(sizeof(int));
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
        printf("Chat ID: %d\nMensaje recibido: %s\n", client_id, buffer);

        // Procesar el mensaje
        PDUData pdu_data;
        procesar_datos_tcp(buffer, bytes_read, &pdu_data, client_id); 
        // if (pdu_data.is_valid) {
        //     // Datos PDU parseados correctamente
        //     printf("Datos PDU: Usuario: %s, Timestamp: %s, Mensaje: %s\n", pdu_data.usuario, pdu_data.timestamp, pdu_data.mensaje);

        // } else {
        //     // Error de formato en la PDU
        //     printf("Error de formato en la PDU\n");
        // }

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