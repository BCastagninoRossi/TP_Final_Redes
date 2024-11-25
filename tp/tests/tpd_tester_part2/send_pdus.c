#include "send_pdus.h"

extern int debug;
 
int send_pdus(const char* destination, char *pdus_str[], int usec_wait_between_writes) {
  int s; /* socket file descriptor*/
  struct addrinfo hints;
  struct addrinfo *servinfo; /* es donde van los resultados */
  int status;
 
  memset(&hints,0,sizeof(hints));     /* es necesario inicializar con ceros */
  hints.ai_family = AF_INET;          /* Address Family */
  hints.ai_socktype = SOCK_STREAM;    /* Socket Type */
  if ( (status = getaddrinfo(destination, CHAT_PORT, &hints, &servinfo))!=0)
  {
    fprintf(stderr, "Error en getaddrinfo: %s\n",gai_strerror(status));
    return 1;
  }
  s = socket( servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);

  switch (connect(s, servinfo->ai_addr, servinfo->ai_addrlen))
  {
    case -1: 
      fprintf(stderr,"Error %i: %s \n",errno,strerror(errno));
      return 1;
      break;
    case 0:  
      printf("Connected (^C to exit)\n\n");
      break;
  }

  int nbytes;
  for (int i=0; i < ARRAY_SIZE; i++) {
    if (debug) hexdump(pdus_str[i], strlen(pdus_str[i]));
    nbytes = send(s, pdus_str[i], strlen(pdus_str[i]), 0);
    printf("Wrote %d bytes\n", nbytes);
    printf("\n");
    usleep(usec_wait_between_writes);
  }
  sleep(1);
 
  return 0;
}