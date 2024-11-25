1. COMPILATION
==============
Just run make

You should see something like this:
nmatsunaga@ubuntu:~/tpd$ make
gcc -Wall   -c -o chat_stream_tester.o chat_stream_tester.c
gcc -Wall -c hexdump.c
gcc -Wall -c send_pdu.c
gcc -Wall -c chat_send.c
gcc -o tester_program chat_stream_tester.o hexdump.o send_pdu.o chat_send.o

Runs fine on gcc version 9.4.0

The executable filename is tester_program

2. RUN
======
You can run the program with several arguments
-H FQDN_or_IP    (server address or resolvable name)
-d DEBUG_LEVEL   (must be greater than 0 to print)
-a WRITE_LIMIT   
-b USEC_SLEEP_BETWEEN_WRITES
-f CHAT_FILENAME  (this parameter can be used several times, upto MAX_FILES defined in macros.h)

Examples: 
Ex1) sends chat in file chat-03.txt with DEBUG enabled
nmatsunaga@ubuntu:~/tpd$ ./tester_program  -H 127.0.0.1 -f chat-03.txt     -d 1

Ex2) sends concurrently chats in file chat-01.txt chat-02.txt chat-03.txt
nmatsunaga@ubuntu:~/tpd$ ./tester_program  -H 127.0.0.1 -f chat-03.txt  -f chat-01.txt -f chat-02.txt

Ex3) send chat in file chat-01 with DEBUG enabled, a maximum write size of 100 Bytes and a wait between writes of 200000 usecs
nmatsunaga@ubuntu:~/tpd$ ./tester_program  -H 127.0.0.1 -f chat-01.txt  -d 1 -a 100 -b 200000
max_write_size = 100, usec_wait_between_writes = 200000, destination = 127.0.0.1
Option -f 0 with value 'chat-01.txt'


3. FILE CHAT CODING
===================
Chat is coded with one line belonging to one message

A chat line contains 3 fields
- Username
- timestamp 
- Message

Separator is ¿ (0xBF in Extended ASCII, use ISO 8859-1 editor in order to generate chat text files)