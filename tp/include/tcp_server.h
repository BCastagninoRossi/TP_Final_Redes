#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <netinet/in.h>

#define PORT 8080            // Puerto para escuchar conexiones
#define BUFFER_SIZE 10000     // Tamaño del buffer de lectura
#define MAX_CLIENTS 10        // Número máximo de conexiones pendientes en la cola

// Estructura para almacenar la información de un cliente.
typedef struct {
    int id;                 // Identificador del cliente
    int* fd;                 // Descriptor de archivo del cliente
} client_t;

// Inicializa el servidor TCP y lo configura para escuchar en el puerto especificado.
int init_tcp_server();

// Acepta conexiones de clientes y maneja la comunicación con cada cliente.
void accept_connections(int server_fd);

// Procesa cada mensaje recibido y lo parsea en sus componentes.
void handle_client(int client_fd, int client_id);

#endif // TCP_SERVER_H
