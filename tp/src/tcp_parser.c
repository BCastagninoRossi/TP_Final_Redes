#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* necesario para memset() */
#include <unistd.h> /* necesario para close() */
#include "tcp_parser.h"

// Función para procesar datos TCP y parsear PDU
void procesar_datos_tcp(char *buffer, int buffer_size) {
    int inbytes = buffer_size;
    int pdu_status;
    int buffer_ptr = 0, pdu_candidate_ptr = 0;
    char pdu_candidate[MAX_PDU_SIZE + 1];
    memset(pdu_candidate, 0, sizeof(pdu_candidate));

    // Procesar datos recibidos
    while (inbytes - buffer_ptr - 1 > 0) {
        pdu_status = processReceivedData(buffer, inbytes, &buffer_ptr, pdu_candidate, &pdu_candidate_ptr);
        if (pdu_status == PDU_CANDIDATE_LINE_OK) {
            printf("PDU completo: %s\n", pdu_candidate);

            // Parsear PDU con ' ' como delimitador
            char *usuario = strtok(pdu_candidate, " ");
            char *timestamp = strtok(NULL, " ");
            char *mensaje = strtok(NULL, " ");

            // Guardar los campos de la PDU en un string
            char PDU_data[1000];
            sprintf(PDU_data, "Usuario: %s, Timestamp: %s, Mensaje: %s", usuario, timestamp, mensaje);
            printf("PDU_data: %s\n", PDU_data);

            // Limpiar memoria
            pdu_candidate_ptr = 0;
            memset(pdu_candidate, 0, sizeof(pdu_candidate));
        } else if (pdu_status == PDU_ERROR_BAD_FORMAT) {
            printf("ERROR: Formato de PDU incorrecto\n");
            // Limpiar memoria
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
            *pdu_candidate++ = ' ';
            (*pdu_candidate_ptr)++;
            return PDU_CANDIDATE_LINE_OK;
        } else if (*buffer != '\x02' && *buffer != '\x04' && (*buffer < 32 || *buffer > 126)) {
            return PDU_ERROR_BAD_FORMAT;
        } else {
            if (*buffer == '\x02' || *buffer == '\x04') {
                *pdu_candidate = ' ';
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
//         procesar_datos_tcp(buffer, inbytes);
//     }

//     return 0;
// }