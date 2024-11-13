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


// #define DBG_RECEIVED_DATA
void procesar_datos_tcp(char *buffer, int buffer_size);
int processReceivedData(char *buffer, int buffersize, int *buffer_ptr, char *pdu_candidate, int *pdu_candidate_ptr);