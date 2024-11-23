#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* necesario para memset() */
#include "../include/tcp_parser.h"
#include "../utils/syslogger.h"
#include <ctype.h>

#include <curl/curl.h>
#include <json-c/json.h>


#define API_URL "http://api.udesa.matsunaga.com.ar:15000/analyze"
#define API_TOKEN "token1"


#define MAX_PDU_SIZE         10000
#define BUFFER_SIZE          3000

#define PDU_CANDIDATE_LINE_OK	 1
#define PDU_ERROR_BAD_FORMAT	-1
#define PDU_NEED_MORE_DATA       0

// Función para manejar la respuesta de CURL
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t total_size = size * nmemb;
    strncat((char*)userp, (char*)contents, total_size);
    return total_size;
}

void complete_http_request(PDUData *pdu_data, SentimentData *result) {
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
                // Guardar los campos de la respuesta en la estructura
                strncpy(result->sentiment, json_object_get_string(sentiment), sizeof(result->sentiment) - 1);
                result->score = json_object_get_double(sentiment_value);
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

// Función para validar los campos de la PDU
int validate_fields(const char *usuario, const char *timestamp, const char *mensaje) {
    // Validar usuario
    if (strlen(usuario) > 40) {
        return PDU_ERROR_BAD_FORMAT;
    }
    for (int i = 0; i < strlen(usuario); i++) {
        if ((unsigned char)usuario[i] > 127) {
            return PDU_ERROR_BAD_FORMAT;
        }
    }

    // Validar timestamp
    if (strlen(timestamp) != 19) {
        return PDU_ERROR_BAD_FORMAT;
    }
    if (timestamp[4] != '-' || timestamp[7] != '-' || timestamp[10] != ' ' || timestamp[13] != ':' || timestamp[16] != ':') {
        return PDU_ERROR_BAD_FORMAT;
    }
    for (int i = 0; i < 19; i++) {
        if (i == 4 || i == 7 || i == 10 || i == 13 || i == 16) {
            continue;
        }
        if (!isdigit(timestamp[i])) {
            return PDU_ERROR_BAD_FORMAT;
        }
    }

    // Validar mensaje
    if (strlen(mensaje) > 200) {
        return PDU_ERROR_BAD_FORMAT;
    }
    for (int i = 0; i < strlen(mensaje); i++) {
        if ((unsigned char)mensaje[i] > 127) {
            return PDU_ERROR_BAD_FORMAT;
        }
    }

    return 0; // Campos válidos
}

// Función para procesar datos TCP y parsear PDU
void process_tcp_data(char *buffer, int buffer_size, PDUData *pdu_data, int client_id) {
    int inbytes = buffer_size;
    int pdu_status;
    int buffer_ptr = 0, pdu_candidate_ptr = 0;
    char pdu_candidate[MAX_PDU_SIZE + 1];
    memset(pdu_candidate, 0, sizeof(pdu_candidate));

    // Procesar datos recibidos
    while (inbytes - buffer_ptr - 1 > 0) {
        pdu_status = processReceivedData(buffer, inbytes, &buffer_ptr, pdu_candidate, &pdu_candidate_ptr);
        if (pdu_status == PDU_CANDIDATE_LINE_OK) {
            // printf("PDU completo: %s\n", pdu_candidate);

            // Parsear PDU con 'á' como delimitador
            char *usuario = strtok(pdu_candidate, "á");
            char *timestamp = strtok(NULL, "á");
            char *mensaje = strtok(NULL, "á");

            // Validar los campos
            if (validate_fields(usuario, timestamp, mensaje) == PDU_ERROR_BAD_FORMAT) {
                printf("ERROR: Formato de PDU incorrecto\n");
                // Limpiar memoria y continuar con la siguiente PDU
                pdu_candidate_ptr = 0;
                memset(pdu_candidate, 0, sizeof(pdu_candidate));
                continue;
            }

            // Guardar los campos de la PDU en la estructura
            strncpy(pdu_data->usuario, usuario, sizeof(pdu_data->usuario) - 1);
            strncpy(pdu_data->timestamp, timestamp, sizeof(pdu_data->timestamp) - 1);
            strncpy(pdu_data->mensaje, mensaje, sizeof(pdu_data->mensaje) - 1);

            //Muestra los datos de la PDU
            // printf("Usuario: %s\n", pdu_data->usuario);
            // printf("Timestamp: %s\n", pdu_data->timestamp);
            // printf("Mensaje: %s\n", pdu_data->mensaje);

            // Llamar a la API
            SentimentData result;
            complete_http_request(pdu_data, &result);

            // Loguear los resultados con syslog
            char client_id_char[10];
            sprintf(client_id_char, "%d", client_id);
            if (strstr(pdu_data->usuario, "Client") != NULL) {
                log_message_syslog(pdu_data->timestamp,client_id_char, pdu_data->usuario, pdu_data->mensaje, result.sentiment, result.score);
            } else {
                log_message_syslog(pdu_data->timestamp, client_id_char, pdu_data->usuario, pdu_data->mensaje, result.sentiment, result.score);
            }

            // Limpiar memoria
            pdu_candidate_ptr = 0;
            memset(pdu_candidate, 0, sizeof(pdu_candidate));
            memset(&result, 0, sizeof(result));
            
            
        } else if (pdu_status == PDU_ERROR_BAD_FORMAT) {
            printf("ERROR: Formato de PDU incorrecto\n");
            // Limpiar memoria y continuar con la siguiente PDU
            pdu_candidate_ptr = 0;
            memset(pdu_candidate, 0, sizeof(pdu_candidate));
        } else {
            printf("PDU parcial: %s\n", pdu_candidate);
            printf("No se encontró delimitador. Probablemente se necesita leer otro buffer\n");
        }
    }
}

// Función para delimitar y procesar PDU
int processReceivedData(char *buffer, int buffersize, int *buffer_ptr, char *pdu_candidate, int *pdu_candidate_ptr) {
    pdu_candidate += *pdu_candidate_ptr;
    buffer += *buffer_ptr;

    int chunk = (buffersize - *buffer_ptr);
    for (int i = 0; i < chunk; i++) {
        (*buffer_ptr)++;

        // Criterio de PDU: \x04 (MSEP)
        if (*pdu_candidate_ptr >= 1 && *buffer == 0x04) {
            *pdu_candidate++ = 'á';
            (*pdu_candidate_ptr)++;
            return PDU_CANDIDATE_LINE_OK;
        } else if (*buffer != '\x02' && *buffer != '\x04' && (*buffer < 32 || *buffer > 126)) {
            // Más logica de validacion de PDU
            // printf("-------Buffer: %s\n", buffer);
            return PDU_ERROR_BAD_FORMAT;
        } else {
            if (*buffer == '\x02' || *buffer == '\x04') {
                *pdu_candidate = 'á';
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

// // Función principal para probar el procesamiento de datos TCP
// int main() {
//     char *test_buffers[] = {
//         "usuario1\x02timestamp1\x02mensaje1\x04",
//         "usuario2\x02timestamp2\x02mensaje2\x04usuario3\x02timestamp3\x02mensaje3\x04",
//         "usuario4\x02timestamp4\x02mensaje4\x04usuario5\x02timestamp5\x02mensaje5\x04usuario6\x02timestamp6\x02mensaje6\x04",
//         "usuario7\x02timestamp7\x02mensaje7_incomplete"
//     };

//     for (int i = 0; i < 4; i++) {
//         char buffer[BUFFER_SIZE];
//         strcpy(buffer, test_buffers[i]);
//         int inbytes = strlen(buffer);
//         PDUData pdu_data;
//         int result = procesar_datos_tcp(buffer, inbytes, &pdu_data);
//         if (result == 1) {
//             printf("Datos PDU: Usuario: %s, Timestamp: %s, Mensaje: %s\n", pdu_data.usuario, pdu_data.timestamp, pdu_data.mensaje);
//         }
//     }

//     return 0;
// }