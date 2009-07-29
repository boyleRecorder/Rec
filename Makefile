
OBJS=main.o server.o wav.o g711.o

recServer: $(OBJS) 
	gcc -o recServer $(OBJS) -lm -lpthread


g711.o : g711.c 
	gcc -c g711.c
wav.o : wav.c 
	gcc -c wav.c
server.o : server.c 
	gcc -c server.c
main.o : main.c 
	gcc -c main.c


clean :
	rm *.o

