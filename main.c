#include <stdio.h>
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
	short sData[160];
	
	struct sockaddr  address;

	// Read the data from the socket provided.
	nbytes = recv(sockNum, buf, sizeof(buf), 0);


	// Convert the encoded data into raw PCM.
	// TODO: The wav object should be able to handle PCMA
	alaw_expand(160,(Byte*)buf+12,sData);

	len = sizeof ( address); 
	if ( getsockname ( sockNum, &address, &len ) < 0 ) 
		perror ( "getsockname" ); 
	else 
		portNum= ntohs ( ((struct sockaddr_in *)&address)->sin_port );


	// Write the data from the socket to file.
	writeData(fileList[MIN_PORT_NO - portNum].sink,(char*)sData,320);

}

static void channelCreated(int portNumber, char *callID)
{
	char file[64];
	char tmpNum[10];
	
	sprintf(tmpNum,"%i",portNumber);
	sprintf(file,"./recordings/%s_%s.wav",callID,tmpNum);

	fileList[MIN_PORT_NO - portNumber].sink  = createWavSink(file,1);
}

static void channelClosed(int portNumber)
{
	closeWavSink(fileList[MIN_PORT_NO-portNumber].sink);
}

static void init()
{
	fileList = (FileNames*)malloc(sizeof(FileNames) * CHANNELS_PER_SERVER);
}

int main()
{
	init();
	pthread_t thread;
	pthread_mutex_init(&mutex,NULL);
	pthread_create(&thread,NULL,readCommands,NULL);
	callBacks.readSocketData = readSocketData;
	callBacks.channelCreated = channelCreated;
	callBacks.channelClosed  = channelClosed;
	watchForIncommingData();
	return 0;
}
