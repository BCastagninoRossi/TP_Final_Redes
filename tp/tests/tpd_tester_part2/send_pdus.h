#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h> /* strncpy */
#include <unistd.h> /* usleep */
#include <errno.h>  /* necesario para codigos de errores */
#include <netdb.h>  /* necesario para getaddrinfo() */

#include "macros.h"
#include "hexdump.h"


int send_pdus(const char* destination, char *pdus_str[], int usec_wait_between_writes);
