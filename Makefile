
recServer: main.o server.o 
	gcc -o recServer main.o server.o -lm -lpthread


server.o : server.c 
	gcc -c server.c
main.o : main.c 
	gcc -c main.c


clean :
	rm file1.o file2.o file3.o

