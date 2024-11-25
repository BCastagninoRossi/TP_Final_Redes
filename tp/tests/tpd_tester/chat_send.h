#include "macros.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> /* usleep */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>  /* necesario para getaddrinfo() */
#include <string.h> /* necesario para memset() */
#include <errno.h>  /* necesario para codigos de errores */

#include "send_pdu.h"

int openAndSendFile(const char* destination, const char* filename, int max_write_size, int usec_wait_between_writes);