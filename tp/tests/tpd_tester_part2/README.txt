1. COMPILATION
==============
Just run make

You should see something like this:
nmatsunaga@ubuntu:~/tpd$ make
nmatsunaga@ubuntu:~/tpd_tester_part2$ make
gcc -Wall -c main.c
gcc -Wall -c hexdump.c
gcc -Wall -c send_pdus.c
gcc -o tester_program_2 main.o hexdump.o send_pdus.o

Runs fine on gcc version 9.4.0

The executable filename is tester_program_2

2. RUN
======
You can run the program with several arguments
-H FQDN_or_IP    (server address or resolvable name)
-d DEBUG_LEVEL   (must be greater than 0 to print)
-c CASE_NUMBER
-w USEC_SLEEP_BETWEEN_WRITES

Examples: 
Ex1) sends write of case number 2 (see file pdu_examples.h) with default wait between writes
nmatsunaga@ubuntu:~/tpd_tester_part2$ ./tester_program_2 -H 127.0.0.1 -c 2 -d 1

Ex2) sends write of case number 1 (see file pdu_examples.h) with 1 second wait between writes
nmatsunaga@ubuntu:~/tpd_tester_part2$ ./tester_program_2 -H 127.0.0.1 -c 1 -d 1 -w 1000000

3. TCP SEGMENTS
===============

You can write your own cases by editing file pdu_examples.h

if you add cases you must reflect it on file macros.h CASES macro

#define CASES 3

Also array size establishes the number of writes per case
#define ARRAY_SIZE 3

Recompile after changing this 2 files: 
  make clean && make