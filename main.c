
#include "header.h"

#include <stdio.h>
#include <pthread.h>
#include "wav.h"
#include "g711.h"

FileSink *sink;

static void readSocketData(int sockNum)
{
	int nbytes;
	char *buf[1024];

	nbytes = recv(sockNum, buf, sizeof(buf), 0);

	short sData[160];

	alaw_expand(160,(Byte*)buf+12,sData);

	writeData(sink,(char*)sData,320);

}

static void channelCreated(int portNumber, char *callID)
{
	printf("a port has been opened: %i\n",portNumber);
	char tmpNum[10];
	sprintf(tmpNum,"%i",portNumber);
	char file[64];
	sprintf(file,"file_%s.wav",tmpNum);

	sink = createWavSink(file,1);
}

static void channelClosed(int portNumber)
{
	printf("A port has been closed: %i\n",portNumber);
	closeWavSink(sink);
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
