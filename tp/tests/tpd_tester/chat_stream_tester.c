#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> /* getopt, usleep */
#include <sys/wait.h> /* necesario para wait() */
#include <signal.h>  /* necesario para sigaction */

#include "macros.h"

#include "chat_send.h"

int childs_spawned = 0;
int debug = 0;


/* Signal handler that will reap zombie processes */
static void sigchld_handler(int s)
{
  int pid;
  while( (pid = waitpid(-1,NULL,WNOHANG)) > 0) {
    printf("Reaping zombie PID %d\n", pid); 
    childs_spawned--;
  }
}


int main (int argc, char **argv)
{

  /* set up signal handler that will reap zombie child process*/
  struct sigaction sa;
  sa.sa_handler = sigchld_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  if (sigaction(SIGCHLD, &sa, NULL) == -1) {
    perror("sigaction");
    return 1;
  }

  /* Defaults */
  int max_write_size = 0;
  int usec_wait_between_writes = 0;

  /* Variables for parsing and storing arguments*/
  int index;
  char* destination = NULL;
  int c;
  char *filenames[MAX_FILES];
  int file_index = 0;
  /* Turn off getopt errors */
  opterr = 0;

  /* Parse arguments with getopt*/
  while ((c = getopt (argc, argv, "a:b:f:H:d:")) != -1) {
    switch (c){
      case 'a':
        max_write_size = atoi(optarg);
        break;
      case 'b':
        usec_wait_between_writes = atoi(optarg);
        break;
      case 'd':
        debug = atoi(optarg);
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
        if (optopt == 'f' || optopt == 'a' || optopt == 'b' || optopt == 'd')
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

  /* Print current arguments */
  printf ("max_write_size = %d, usec_wait_between_writes = %d, destination = %s\n",
    max_write_size, usec_wait_between_writes, destination);
  for (int i=0 ; i < file_index; i++) {
    printf("Option -f %d with value '%s'\n", i, filenames[i]);
  }

  for (index = optind; index < argc; index++)
    printf ("Non-option argument %s\n", argv[index]);

  for (int i=0; i< file_index; i++) {
    if (!fork()) 
    { /* this is the child process */
      openAndSendFile(destination, filenames[i], max_write_size, usec_wait_between_writes); 
      return 0;
    }
    childs_spawned++;

  }
  // Father just waits for childs
  while (childs_spawned > 0) {
    if (debug) printf("Childs alive %d\n", childs_spawned);
    sleep(1);
  }
  printf("All my childs have died :(. But all the work is done :)\n");
  return 0;
}

