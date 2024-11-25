#include "send_pdu.h"

extern int debug;

int send_pdu(int socket, char* buffer, unsigned int length, int max_write_size, int usec_wait_between_writes) {
  int pid =  getpid();
  int nbytes = 0;
  if (max_write_size == 0) {
    /* there is no max write size */
    nbytes = send(socket, buffer, length, 0);
    if (debug) hexdump(buffer, nbytes);
    fprintf(stderr, "[%d] writing %d bytes\n", pid, nbytes);
  }
  else if (max_write_size > 0) {
    int remaining_bytes = length;
    int write_size, aux_nbytes;

    while (remaining_bytes > 0) {
      /* calculate write size */
      if (remaining_bytes > max_write_size) {
        write_size = max_write_size;
      }
      else {
        write_size = remaining_bytes;
      }
      aux_nbytes = send(socket, buffer, write_size, 0);
      if (debug) hexdump(buffer, aux_nbytes);

      remaining_bytes = remaining_bytes - aux_nbytes;

      char aux_str[1000];
      memset(aux_str, 0, sizeof(aux_str));
      strncpy(aux_str, buffer, aux_nbytes);

      fprintf(stderr, "[%d] writing %d bytes, remaining %d '%s'\n", pid, aux_nbytes, remaining_bytes, aux_str);
      buffer +=aux_nbytes; /* advance ptr*/
      usleep(usec_wait_between_writes);

    }
  }
  else {
    /* something went wrong, called with negative values*/
    fprintf(stderr, "Cannot call send_pdu with negativa max_write_size %d\n", max_write_size);
    exit(3);
  }

  return nbytes;

}