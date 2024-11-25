#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* strelen */
#include <unistd.h> /* getopt, usleep */

#include "macros.h"
#include "hexdump.h"
#include "pdu_examples.h"
#include "send_pdus.h"

int debug = 0;

int main (int argc, char **argv)
{

  /* Variables for parsing and storing arguments*/
  char* destination = NULL;
  int c;
  int case_number = -1;
  int usec_wait_between_writes = 5e5;

  /* Turn off getopt errors */
  opterr = 0;

  /* Parse arguments with getopt*/
  while ((c = getopt (argc, argv, "c:d:H:w:")) != -1) {
    switch (c){
      case 'c':
        case_number = atoi(optarg);
        break;
      case 'H':
        destination = optarg;
        break;
      case 'w':
        usec_wait_between_writes = atoi(optarg);
        break;
      case 'd':
        debug = atoi(optarg);
        break;
      case '?':
        if (optopt == 'c' ||  optopt == 'd' ||  optopt == 'w')
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

  if (case_number == -1) {
    fprintf(stderr,"Option -c <case_number> is mandatory\n");
    return 1;

  }

  if ( case_number < 0 || case_number >= CASES) {
    fprintf(stderr,"Option -c <case_number> must be between 0 and %d\n", CASES-1);
    return 1;
  }
  printf("Case number %d, usec_wait_between_writes = %d usecs\n", case_number, usec_wait_between_writes);
   
  send_pdus(destination, test_buffers_arr[case_number], usec_wait_between_writes);

  return 0;
}