CC=gcc
OPTS=-g -Wall
OBJS=main.o server.o wav.o g711.o
EXEC=recServer 
LIBS= -lm -lpthread

recServer: $(OBJS) 
	$(CC) -o $(EXEC) $(OBJS) $(LIBS) 


g711.o : g711.c 
	$(CC) -c $(OPTS) g711.c
wav.o : wav.c 
	$(CC) -c $(OPTS) wav.c
server.o : server.c 
	$(CC) -c $(OPTS) server.c
main.o : main.c 
	$(CC) -c $(OPTS) main.c


clean :
	rm *.o *~ $(EXEC)

