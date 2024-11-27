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
#define PDU_ERROR_BAD_FORMAT_DELIMITERS	-1
#define PDU_ERROR_BAD_FORMAT_FIELDS	-2
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
        return PDU_ERROR_BAD_FORMAT_FIELDS;
    }

    // Validar timestamp
    if (strlen(timestamp) != 19) {
        return PDU_ERROR_BAD_FORMAT_FIELDS;
    }
    if (timestamp[4] != '-' || timestamp[7] != '-' || timestamp[10] != ' ' || timestamp[13] != ':' || timestamp[16] != ':') {
        return PDU_ERROR_BAD_FORMAT_FIELDS;
    }


    // Validar mensaje
    if (strlen(mensaje) > 200) {
        return PDU_ERROR_BAD_FORMAT_FIELDS;
    }

    return 0; // Campos válidos
}

// Función para procesar datos TCP y parsear PDU
void process_tcp_data(char *buffer, int rec_bytes, PDUData *pdu_data, int client_id) {
    int inbytes = rec_bytes;
    int pdu_status;
    int buffer_ptr = 0;
    int pdu_candidate_ptr = 0;
    char pdu_candidate[MAX_PDU_SIZE + 1];
    memset(pdu_candidate, 0, sizeof(pdu_candidate));

    // Acá deberia volver a leer el buffer en caso de que no se haya procesado todo un PDU? como hacemo'?? 
    // Procesar datos recibidos
    while (inbytes - buffer_ptr - 1 > 0) {
        pdu_status = processReceivedData(buffer, inbytes, &buffer_ptr, pdu_candidate, &pdu_candidate_ptr);
        if (pdu_status == PDU_CANDIDATE_LINE_OK) {
            // printf("PDU completo: %s\n", pdu_candidate);

            // Parsear PDU con 'á' como delimitador
            char *usuario = strtok(pdu_candidate, "\x29");
            char *timestamp = strtok(NULL, "\x29");
            char *mensaje = strtok(NULL, "\x29");


            // Validar los campos
            if (validate_fields(usuario, timestamp, mensaje) == PDU_ERROR_BAD_FORMAT_FIELDS) {
                printf("ERROR: Formato de CAMPOS DEL PDU incorrecto.\n");
                // Limpiar memoria y continuar con la siguiente PDU
                buffer_ptr++;  // Saltear caracter invalido
                pdu_candidate_ptr = 0;
                memset(pdu_candidate, 0, sizeof(pdu_candidate));

                char client_id_char[10];
                sprintf(client_id_char, "%d", client_id);
                log_message_syslog("ERROR: BAD PDU FORMAT | Invalid Fields",client_id_char, "None", "None", "None", 0.0);
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
            
            
        } else if (pdu_status == PDU_ERROR_BAD_FORMAT_DELIMITERS) {
            printf("ERROR: Formato de PDU incorrecto | Error en los chars del PDU\n");
            // Limpiar memoria y continuar con la siguiente PDU

            buffer_ptr++; 
            pdu_candidate_ptr = 0;
            memset(pdu_candidate, 0, sizeof(pdu_candidate));
            char client_id_char[10];
            sprintf(client_id_char, "%d", client_id);
            log_message_syslog("ERROR: BAD PDU FORMAT | PDU Contains invalid chars", client_id_char, "None", "None", "None", 0.0);

        } else {
            // Aca capaz hay que manejar la nueva leida del buffer o algo asi por el PDU incompleto

            char client_id_char[10];
            sprintf(client_id_char, "%d", client_id);
            log_message_syslog("ERROR: INCOMPLETE PDU",client_id_char, pdu_candidate, "None", "None", 0.0);
            printf("ERROR | PDU parcial: %s\n", pdu_candidate);
            
        }
    }
}

// Función para delimitar y procesar PDU
int processReceivedData(char *buffer, int buffersize, int *buffer_ptr, char *pdu_candidate, int *pdu_candidate_ptr) {
    pdu_candidate += *pdu_candidate_ptr;
    buffer += *buffer_ptr;

    int chunk = (buffersize - *buffer_ptr);
    int invalid_chars = 0;
    for (int i = 0; i < chunk; i++) {
        (*buffer_ptr)++;

        // Criterio de PDU: \x04 (MSEP)
        if (*pdu_candidate_ptr >= 1 && *buffer == 0x04) {
            *pdu_candidate++ = '\x29';
            (*pdu_candidate_ptr)++;
            if (invalid_chars) {
                return PDU_ERROR_BAD_FORMAT_DELIMITERS;
            }
            return PDU_CANDIDATE_LINE_OK;
        } else if (*buffer != '\x02' && *buffer != '\x04' && (*buffer < 32 || *buffer > 126)) {
            invalid_chars = 1;
            *pdu_candidate = *buffer;
        } else {
            if (*buffer == '\x02' || *buffer == '\x04') {
                *pdu_candidate = '\x29';
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
