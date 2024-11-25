CC = gcc
CFLAGS = -Wall
#CFLAGS = -Wall -Werror

#all: myprogram
all: tester_program

myprogram: main.o beej_pack.o hexdump.o
	$(CC) -o $@ $^

tester_program: chat_stream_tester.o hexdump.o send_pdu.o chat_send.o
	$(CC) -o $@ $^


#chat_stream_tester.o: chat_stream_tester.c
#    $(CC) $(CFLAGS) -c $<

beej_pack.o: beej_pack.c
	$(CC) $(CFLAGS) -c $<

hexdump.o: hexdump.c
	$(CC) $(CFLAGS) -c $<

send_pdu.o: send_pdu.c
	$(CC) $(CFLAGS) -c $<

chat_send.o: chat_send.c
	$(CC) $(CFLAGS) -c $<

main.o: main.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o tester_program
