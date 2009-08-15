CC=gcc
OPTS=-g -Wall
OBJS=main.o server.o wav.o g711.o dictionary.o iniparser.o
EXEC=recServer 
LIBS= -lm -lpthread

MACHINE= $(shell uname -s)
ifeq ($(MACHINE),Darwin)
PJINCLUDES= -I/Users/paulboyle/pjproject-1.3/pjlib/include -I/Users/paulboyle/pjproject-1.3/pjlib-util/include -I/Users/paulboyle/pjproject-1.3/pjnath/include -I/Users/paulboyle/pjproject-1.3/pjmedia/include -I/Users/paulboyle/pjproject-1.3/pjsip/include

PJLIBS=-L/Users/paulboyle/pjproject-1.3/pjlib/lib -L/Users/paulboyle/pjproject-1.3/pjlib-util/lib -L/Users/paulboyle/pjproject-1.3/pjnath/lib -L/Users/paulboyle/pjproject-1.3/pjmedia/lib -L/Users/paulboyle/pjproject-1.3/pjsip/lib -L/Users/paulboyle/pjproject-1.3/third_party/lib  -lpjsua-i386-apple-darwin8.11.1 -lpjsip-ua-i386-apple-darwin8.11.1 -lpjsip-simple-i386-apple-darwin8.11.1 -lpjsip-i386-apple-darwin8.11.1 -lpjmedia-codec-i386-apple-darwin8.11.1 -lpjmedia-i386-apple-darwin8.11.1 -lpjmedia-audiodev-i386-apple-darwin8.11.1 -lpjnath-i386-apple-darwin8.11.1 -lpjlib-util-i386-apple-darwin8.11.1 -lresample-i386-apple-darwin8.11.1 -lmilenage-i386-apple-darwin8.11.1 -lsrtp-i386-apple-darwin8.11.1 -lgsmcodec-i386-apple-darwin8.11.1 -lspeex-i386-apple-darwin8.11.1 -lilbccodec-i386-apple-darwin8.11.1 -lg7221codec-i386-apple-darwin8.11.1 -lportaudio-i386-apple-darwin8.11.1 -lpj-i386-apple-darwin8.11.1 -lm -lpthread  -framework CoreAudio -framework CoreServices -framework AudioUnit -framework AudioToolbox -lssl -lcrypto
else
PJINCLUDES= 

PJLIBS=-
endif

CLIENT_OBJS=server.o siprtp.o dictionary.o iniparser.o

all: client recServer

client: $(CLIENT_OBJS)
	$(CC) -g -o client $(CLIENT_OBJS) $(PJLIBS) $(LIBS)

recServer: $(OBJS) 
	$(CC) -o $(EXEC) $(OBJS) $(LIBS) 

siprtp.o: siprtp.c
	$(CC) -c $(OPTS) -DPJ_AUTOCONF=1 -O2   $(PJINCLUDES) siprtp.c 
iniparser.o : iniparser.c 
	$(CC) -c $(OPTS) iniparser.c
dictionary.o : dictionary.c 
	$(CC) -c $(OPTS) dictionary.c
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

