#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h> /* needed for memset() */
#include <syslog.h>
#include <curl/curl.h>

#define MAX_PDU_SIZE         10000

#define PDU_CANDIDATE_LINE_OK	 1
#define PDU_ERROR_BAD_FORMAT	-1
#define PDU_NEED_MORE_DATA      0

#define BUFFER_SIZE          3000

typedef struct {
    char usuario[100];
    char timestamp[100];
    char mensaje[1000];
} PDUData;


// #define DBG_RECEIVED_DATA
int procesar_datos_tcp(char *buffer, int buffer_size, PDUData *pdu_data);
int processReceivedData(char *buffer, int buffersize, int *buffer_ptr, char *pdu_candidate, int *pdu_candidate_ptr);