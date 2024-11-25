#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h> /* strncpy */
#include <unistd.h> /* usleep */
#include "hexdump.h"

int send_pdu(int socket, char* buffer, unsigned int length, int max_size, int usec_wait_between_writes);