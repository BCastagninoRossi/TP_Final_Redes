
#include "chat_send.h"

static char *skip_single(char **buf, const char *delimiters);

 
int openAndSendFile(const char* destination, const char* filename, int max_write_size, int usec_wait_between_writes) {
  int s; /* socket file descriptor*/
  struct addrinfo hints;
  struct addrinfo *servinfo; /* es donde van los resultados */
  int status;
  FILE *fp;

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

  /* Abrimos el archivo de chat */
  fp = fopen(filename, "r");
  if (fp == NULL) {
    fprintf(stderr, "Error opening file %s\n", filename);
    return 2;
  }

  /* Leemos el archivo linea por linea*/
  char *c;
  c = (char *)malloc(BUFFER_LINE_SIZE);
  int i = 0;
  int nbytes = 0;
  while ( (c=fgets(c, BUFFER_LINE_SIZE, fp) ) != NULL) {
    // printf("%d %s", i++, c); // Just for debug
    // Trim \n and \r
    int original_length = strlen(c); 
    for (int j=0 ; j< original_length ; j++) {
      if ( c[j] == '\n' || c[j] == '\r') {
        c[j] = '\0';
      }
    }

    // Build PDU
    char pdu_buffer[264]; // 40 + 1 + 20 + 1 + 200 + 1 = 263
    memset(pdu_buffer, 0, sizeof(pdu_buffer));
    // printf( "Hexadecimal bytes: \x41\x42\x43\n");

    char *username, *datetime, *message;
    username = skip_single(&c, "¿");
    datetime = skip_single(&c, "¿");
    message  = skip_single(&c, "¿");

    if (strlen(username)==0) {
      fprintf(stderr, "Username empty at file %s at line %d\n", filename, i);
      exit(4);
    }
    if (strlen(datetime)==0) {
      fprintf(stderr, "Datetime empty at file %s at line %d\n", filename, i);
      exit(4);
    }
    if (strlen(message)==0) {
      fprintf(stderr, "Message empty at file %s at line %d\n", filename, i);
      exit(4);
    }

    nbytes = sprintf(pdu_buffer, "%s\x02%s\x02%s\x4", username, datetime, message);

    // Send PDU
    send_pdu(s, pdu_buffer, nbytes, max_write_size, usec_wait_between_writes);
    // sleep between full PDUs
    usleep(3e5);
  }

  sleep(3);
  close(s);

  return 0;
}


// Skip the characters until the SINGLE DELIMITER character is found
// 0-terminate resulting word
// Advance pointer to buffer to the last skipped position + delimiter.
// Return found 0-terminated wor.
static char *skip_single(char **buf, const char *delimiters)
{
  char *p, *begin_word, *end_word, *end_delimiters;

  begin_word = *buf;

  end_word = begin_word + strcspn(begin_word, delimiters);

  // Take only ONE delimiter
  end_delimiters = end_word + 1;

  for (p = end_word; p < end_delimiters; p++) {
    *p = '\0';
  }

  *buf = end_delimiters;

  return begin_word;
}

