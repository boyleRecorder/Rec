
#include "header.h"

#include <stdio.h>
#include <pthread.h>

FILE *stream;

static void readSocketData(int sockNum)
{
	int nbytes;
	char *buf[1024];

	nbytes = recv(sockNum, buf, sizeof(buf), 0);
	if(stream != NULL)
	fwrite(buf+12,sizeof(char),nbytes-12,stream);
}

static void channelCreated(int portNumber, char *callID)
{
	printf("a port has been opened: %i\n",portNumber);
	char tmpNum[10];
	sprintf(tmpNum,"%i",portNumber);
	char file[64];
	sprintf(file,"file_%s.pcm",tmpNum);
	stream = fopen(file,"w");
}

static void channelClosed(int portNumber)
{
	printf("A port has been closed: %i\n",portNumber);
	if(stream != NULL)
	fclose(stream);
}

int main()
{
	pthread_t thread;
	pthread_create(&thread,NULL,readCommands,NULL);
	callBacks.readSocketData = readSocketData;
	callBacks.channelCreated = channelCreated;
	callBacks.channelClosed  = channelClosed;
	watchForIncommingData();
	return 0;
}
