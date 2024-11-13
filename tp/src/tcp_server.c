#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include "tcp_parser.h"
#include "tcp_server.h"
#include <curl/curl.h>
#include <json-c/json.h>


#define API_URL "http://api.udesa.matsunaga.com.ar:15000/analyze"
#define API_TOKEN "token1"


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


// Función para manejar la respuesta de CURL
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t total_size = size * nmemb;
    strncat((char*)userp, (char*)contents, total_size);
    return total_size;
}

void completar_consulta_http(PDUData *pdu_data) {
    CURL *curl;
    CURLcode res;
    struct curl_slist *headers = NULL;
    char readBuffer[1024] = {0};

    // Inicializar CURL
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if(curl) {
        // Crear los datos del POST en formato JSON
        json_object *jobj = json_object_new_object();
        json_object *jmessage = json_object_new_string(pdu_data->mensaje);
        json_object_object_add(jobj, "message", jmessage);
        const char *json_data = json_object_to_json_string(jobj);

        printf("Enviando mensaje a la API: %s\n", json_data);

        // Configurar headers
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, "X-API-Token: " API_TOKEN);

        // Configurar CURL para realizar una solicitud POST
        curl_easy_setopt(curl, CURLOPT_URL, API_URL);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        // Realizar la solicitud
        res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        } else {
            // Parsear la respuesta JSON
            json_object *parsed_json = json_tokener_parse(readBuffer);
            json_object *sentiment;
            json_object *sentiment_value;

            if (json_object_object_get_ex(parsed_json, "sentiment", &sentiment) &&
                json_object_object_get_ex(parsed_json, "sentiment_value", &sentiment_value)) {
                printf("Sentiment: %s\n", json_object_get_string(sentiment));
                printf("Sentiment Value: %f\n", json_object_get_double(sentiment_value));
            } else {
                printf("Error al parsear la respuesta JSON\n");
                printf("Respuesta de la API: %s\n", readBuffer);
            }

            // Liberar memoria JSON
            json_object_put(parsed_json);
        }

        // Limpiar CURL
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
        json_object_put(jobj); // Liberar memoria JSON
    }

    // Limpiar CURL global
    curl_global_cleanup();
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
        int result = procesar_datos_tcp(buffer, bytes_read, &pdu_data);
        if (result == 1) {
            // Datos PDU parseados correctamente
            printf("Datos PDU: Usuario: %s, Timestamp: %s, Mensaje: %s\n", pdu_data.usuario, pdu_data.timestamp, pdu_data.mensaje);

            // Completar una consulta HTTP con los datos parseados
            completar_consulta_http(&pdu_data);
        } else if (result == -1) {
            // Error de formato en la PDU
            printf("Error de formato en la PDU\n");
        }

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