#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> /* getopt */
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h> /* necesario para memset() */
#include <errno.h>  /* necesario para codigos de errores */
#include <netinet/in.h>
#include <netdb.h>  /* necesario para getaddrinfo() */


#define MAX_FILES 10 /* Ajustar a la cantidad maxima de chat files*/
#define CHAT_PORT "15001"
#define BUFFER_LINE_SIZE 10000

int openAndSendFile(const char* destination, const char* filename);

int main (int argc, char **argv)
{
  int aflag = 0;
  int bflag = 0;
  int index;
  char* destination = NULL;
  int c;

  char *filenames[MAX_FILES];
  int file_index = 0;

  opterr = 0;

  while ((c = getopt (argc, argv, "abf:H:")) != -1) {
    switch (c){
      case 'a':
        aflag = 1;
        break;
      case 'b':
        bflag = 1;
        break;
      case 'H':
        destination = optarg;
        break;
      case 'f':
        if (file_index < MAX_FILES) {
          filenames[file_index++] = optarg;
        } 
        else {
          fprintf(stderr, "Too many -f options\n");
          return 1;
        }
        break;
      case '?':
        if (optopt == 'f')
          fprintf(stderr, "Option -%c requires an argument.\n", optopt);
        else if (isprint (optopt))
          fprintf(stderr, "Unknown option `-%c'.\n", optopt);
        else
          fprintf(stderr,"Unknown option character `\\x%x'.\n", optopt);
        return 1;
      default:
        abort ();
    }
  }

  if ( destination == NULL) {
    fprintf(stderr,"Option -H <destination> is mandatory\n");
    return 1;
  }

  printf ("aflag = %d, bflag = %d, destination = %s\n",
        aflag, bflag, destination);
  for (int i=0 ; i < file_index; i++) {
    printf("Option -f %d with value '%s'\n", i, filenames[i]);
  }

  for (index = optind; index < argc; index++)
    printf ("Non-option argument %s\n", argv[index]);

  return openAndSendFile(destination, filenames[0]); 
  return 0;
}


int openAndSendFile(const char* destination, const char* filename) {
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
  // send(s,argv[2],strlen(argv[2]), 0);

  /* Abrimos el archivo de chat */
  fp = fopen(filename, "r");
  if (fp == NULL) {
    fprintf(stderr, "Error opening file %s\n", filename);
    return 2;
  }
  char *c;
  c = (char *)malloc(BUFFER_LINE_SIZE);
  int i = 0;
  while ( (c=fgets(c, BUFFER_LINE_SIZE, fp) ) != NULL) {
    printf("%d %s", i++, c);
  }

  return 0;
}