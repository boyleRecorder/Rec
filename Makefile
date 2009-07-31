
OPTS=-g -Wall
OBJS=main.o server.o wav.o g711.o

recServer: $(OBJS) 
	gcc -o recServer $(OBJS) -lm -lpthread


g711.o : g711.c 
	gcc -c $(OPTS) g711.c
wav.o : wav.c 
	gcc -c $(OPTS) wav.c
server.o : server.c 
	gcc -c $(OPTS) server.c
main.o : main.c 
	gcc -c $(OPTS) main.c


clean :
	rm *.o

