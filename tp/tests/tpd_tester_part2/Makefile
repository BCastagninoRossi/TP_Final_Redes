CC = gcc
CFLAGS = -Wall
#CFLAGS = -Wall -Werror

all: tester_program_2

tester_program_2: main.o hexdump.o send_pdus.o
	$(CC) -o $@ $^

hexdump.o: hexdump.c
	$(CC) $(CFLAGS) -c $<

main.o: main.c
	$(CC) $(CFLAGS) -c $<

send_pdus.o: send_pdus.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o tester_program_2
