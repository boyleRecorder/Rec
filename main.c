#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <pthread.h>


#include "header.h"

#include "wav.h"
#include "g711.h"

typedef struct
{
	FileSink  *sink;
	int       portNumber;
	char      chanID[128];
}FileNames;

static FileNames *fileList;

static void readSocketData(int sockNum)
{
	unsigned len;
	int nbytes;
	int portNum;
	char *buf[1024];
	
	struct sockaddr  address;

	// Read the data from the socket provided.
	nbytes = recv(sockNum, buf, sizeof(buf), 0);


	len = sizeof ( address); 
	if ( getsockname ( sockNum, &address, &len ) < 0 ) 
		perror ( "getsockname" ); 
	else 
		portNum= ntohs ( ((struct sockaddr_in *)&address)->sin_port );


	// Write the data from the socket to file.
	writeData(fileList[MIN_PORT_NO - portNum].sink,(char*)buf+RTP_HEADER,nbytes-RTP_HEADER);

}

static void channelCreated(int portNumber, char *callID,int codec)
{
	char file[64];
	char tmpNum[10];
	
	sprintf(tmpNum,"%i",portNumber);
	sprintf(file,"./recordings/%s_%s.wav",callID,tmpNum);

	fileList[MIN_PORT_NO - portNumber].sink  = createWavSink(file,1,codec);
}

static void channelClosed(int portNumber)
{
	closeWavSink(fileList[MIN_PORT_NO-portNumber].sink);
}

static void init()
{
	fileList = (FileNames*)malloc(sizeof(FileNames) * CHANNELS_PER_SERVER);
}

#include "iniparser.h"
void create_example_ini_file(void);
int  parse_ini_file(char * ini_name);

int main()
{
	int		status ;
	init();
	status = parse_ini_file(CONFIG_FILE);
	pthread_t thread;
	pthread_mutex_init(&mutex,NULL);
	pthread_create(&thread,NULL,readCommands,NULL);
	callBacks.readSocketData = readSocketData;
	callBacks.channelCreated = channelCreated;
	callBacks.channelClosed  = channelClosed;
	watchForIncommingData();
	return 0;
}

void create_example_ini_file(void)
{
	FILE	*	ini ;

	ini = fopen("example.ini", "w");
	fprintf(ini,
    "#\n"
    "# This is an example of ini file\n"
    "#\n"
    "\n"
    "[Pizza]\n"
    "\n"
    "Ham       = yes ;\n"
    "Mushrooms = TRUE ;\n"
    "Capres    = 0 ;\n"
    "Cheese    = Non ;\n"
    "\n"
    "\n"
    "[Wine]\n"
    "\n"
    "Grape     = Cabernet Sauvignon ;\n"
    "Year      = 1989 ;\n"
    "Country   = Spain ;\n"
    "Alcohol   = 12.5  ;\n"
    "\n");
	fclose(ini);
}


int parse_ini_file(char * ini_name)
{
	dictionary	*	ini ;

	/* Some temporary variables to hold query results */
	int				b ;
	int				i ;

	ini = iniparser_load(ini_name);
	if (ini==NULL) {
		fprintf(stderr, "cannot parse file: %s\n", ini_name);
		return -1 ;
	}
	iniparser_dump(ini, stderr);

	b = iniparser_getboolean(ini, "Rec Servers:numServers", -1);
	printf("numServers:       [%d]\n", b);

	for(i=0;i<b;i++)
	{

	}
	
	iniparser_freedict(ini);
	return 0 ;
}



