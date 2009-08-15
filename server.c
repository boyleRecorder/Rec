#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#include <stdlib.h>
#include <string.h>

#include <limits.h>
#include <string.h>

#include <pthread.h>

#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>

#include "header.h"
#include "iniparser.h"

#define COMMAND_BACKLOG	10

static int highSock;	
static int minSock;	
static fd_set rfds;


static void error(char *msg)
{
    perror(msg);
    exit(1);
}

struct mySockets
{
	int socket;
	int portNumber;
        struct sockaddr_in sa; 
}mySockets;


static int selectList[CHANNELS_PER_SERVER];

static void addSockToSelectList(int sock)
{
	int i;
	for(i=0;i<CHANNELS_PER_SERVER;i++)
	{
		if(selectList[i] == 0)
		{
			pthread_mutex_lock(&mutex);
			selectList[i] = sock;
			pthread_mutex_unlock(&mutex);
			break;
		}
	}
}

static void removeSockFromSelectList(int sock)
{
	int i;
	for(i=0;i<CHANNELS_PER_SERVER;i++)
	{
		if(selectList[i] == sock)
		{
			pthread_mutex_lock(&mutex);
			close(sock);
			selectList[i] = 0;
			pthread_mutex_unlock(&mutex);
			break;
		}
	}
}

static struct mySockets openSockets[CHANNELS_PER_SERVER];

static void setupUDPSocket(int port)
{
  int sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
  printf("created udp sock: %i\n",sock);

  int i;
  for(i=0;i<CHANNELS_PER_SERVER;i++)
  {
	  if(openSockets[i].portNumber == 0)
	  {
		  openSockets[i].portNumber = port;
		  openSockets[i].socket = sock;
		  break;
	  }
  }
 
  memset(&openSockets[i].sa, 0, sizeof(struct sockaddr_in));
  openSockets[i].sa.sin_family = AF_INET;
  openSockets[i].sa.sin_addr.s_addr = INADDR_ANY;
  openSockets[i].sa.sin_port = htons(port);
 
  if (-1 == bind(sock,(struct sockaddr *)&openSockets[i].sa, sizeof(struct sockaddr)))
  {
    perror("UDP error bind failed");
    close(sock);
    openSockets[i].portNumber = 0;
    openSockets[i].socket = 0;
    return ;
  }

  addSockToSelectList(sock);
}

static void closeUDPSocket(int port)
{
	int i;
	for(i=0;i<CHANNELS_PER_SERVER;i++)
	{
		if(openSockets[i].portNumber == port)
		{
			openSockets[i].portNumber = 0;
			break;
		}
	}
	removeSockFromSelectList(openSockets[i].socket);
			openSockets[i].socket = 0;

}

/**
 * We received a TCP packet.
 */
static void serverReceivedPacket(int sock)
{
	int n;
	header myHeader;

	char *ptr;
	ptr = (char*)&myHeader;

	bzero(ptr,sizeof(header));
	n = read(sock,ptr,sizeof(header));
	if (n < 0) 
		error("ERROR reading from socket");

	printf("Here is the message: %i\n",myHeader.portNumber);

	switch(myHeader.command)
	{
		case CREATE_CHANNEL:
			printf("Received create_channel.\n");	  
			setupUDPSocket(myHeader.portNumber);   
                        callBacks.channelCreated(myHeader.portNumber,myHeader.callID,myHeader.codec);
			break;
		case DELETE_CHANNEL:
			printf("Received delete_channel.\n");	 
			closeUDPSocket(myHeader.portNumber);    
                        callBacks.channelClosed(myHeader.portNumber);
			break;
	}
}

/**
This is the TCP server which reads incomming commands.
*/
static void runServer()
{
	int sockfd, newsockfd, portno ;
	unsigned clilen;
	struct sockaddr_in serv_addr, cli_addr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
		error("ERROR opening socket");
	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = COMMS_PORT;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	if (bind(sockfd, (struct sockaddr *) &serv_addr,
				sizeof(serv_addr)) < 0) 
		error("ERROR on binding");

	listen(sockfd,COMMAND_BACKLOG);
	clilen = sizeof(cli_addr);
	while (1) {
		printf("Blocking for new command.\n");
		newsockfd = accept(sockfd, 
				(struct sockaddr *) &cli_addr, &clilen);
		if (newsockfd < 0) 
			error("ERROR on accept");
		serverReceivedPacket(newsockfd);
		close(newsockfd);
	} /* end of while */
	close(sockfd);
}

/**
  Entry point for command handling thread.
*/
void* readCommands(void *arg)
{
	runServer();

	return NULL;
}

static void updateSockStruct()
{
	int i;
	highSock = 0;
	minSock = INT_MAX;
	for(i=0;i<CHANNELS_PER_SERVER;i++)
	{
		if(selectList[i] != 0 )
		{
			if(selectList[i] < minSock)
				minSock = selectList[i];
			FD_SET(selectList[i],&rfds);
			if(selectList[i] >= highSock)
			{
				highSock = selectList[i] + 1;
			}
		}
	}
}

void watchForIncommingData()
{
	int i;
	struct timeval tv;
	int retval;
	int safeHigh;
	int safeLow;

        /* Wait up to fifty ms. */
	tv.tv_sec = 0;
	tv.tv_usec = 50000;


	for(;;)
	{

		// Build the list of sockets to watch.
		// Will this mutex become a bottle neck???
		pthread_mutex_lock(&mutex);
		FD_ZERO(&rfds);
		updateSockStruct();
		safeHigh = highSock; // These 2 values may be modified by the other thread
		safeLow  = minSock;  // so take a copy of them.

		retval = select(safeHigh, &rfds, NULL, NULL, &tv);
		pthread_mutex_unlock(&mutex);

		if (retval == -1)
			perror("select()");
		else if (retval)
		{
			// Find the socket(s) that triggered select.
			for(i = safeLow;i<safeHigh;i++)
			{
				if (FD_ISSET(i, &rfds))
				{
					callBacks.readSocketData(i);
				}
			}
		}
	}
}


int requestChannel(int chanID, struct rtpServer* mediaServer, header *msg)
{
    int sockfd,n ;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    { 
        error("ERROR opening socket");
	return CHANNEL_CREATION_FAILED;
    }
    printf("%s\n",mediaServer->ip);
    server = gethostbyname(mediaServer->ip);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
	return CHANNEL_CREATION_FAILED;
    }
    
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(COMMS_PORT);
    
    if (connect(sockfd,(const struct sockaddr*)&serv_addr,sizeof(serv_addr)) < 0) 
    {
        error("requestChannel ERROR connecting");
	return CHANNEL_CREATION_FAILED;
    }
    
    n = write(sockfd,msg,sizeof(header));
    if (n < 0) 
    {
         error("ERROR requestChannel writing to socket");
	 return CHANNEL_CREATION_FAILED;
    }
    close(sockfd);
    return CHANNEL_CREATED;
}

static void clearChannelTag(struct rtpServer *server, int serverChanID)
{
	int i;

	// Remove the channel.
	for(i=0;i<CHANNELS_PER_SERVER;i++)
	{
		if(server->portActive[i])
		{
			if((server->minPort + i) == serverChanID)
			{
				server->portActive[i] = 0;
				serverChanID = 0;
				break;
			}
		}
	}
}

int closeChannel(int chanID, struct rtpServer* mediaServer)
{
    int sockfd,n ;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    { 
        error("ERROR opening socket");
	return CHANNEL_CREATION_FAILED;
    }
    server = gethostbyname(mediaServer->ip);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
	return CHANNEL_CREATION_FAILED;
    }
    
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(COMMS_PORT);
    
    if (connect(sockfd,(const struct sockaddr*)&serv_addr,sizeof(serv_addr)) < 0) 
    {
        error("closeChannel ERROR connecting");
	return CHANNEL_CREATION_FAILED;
    }

    header msg;
    msg.command = DELETE_CHANNEL;
    msg.portNumber = chanID;
    
    n = write(sockfd,&msg,sizeof(header));
    if (n < 0) 
    {
         error("ERROR closeChannel writing to socket");
	 return CHANNEL_CREATION_FAILED;
    }
    close(sockfd);
    clearChannelTag(mediaServer,chanID);
    return CHANNEL_CREATED;
}



void setupRTPServers()
{
	int i,j;
	int numServers;
	dictionary	*ini ;
	int		b ;
	char		*s ;
	ini = iniparser_load(CONFIG_FILE);
	if (ini==NULL) {
		fprintf(stderr, "cannot parse file: %s\n", CONFIG_FILE);
		return -1 ;
	}
	iniparser_dump(ini, stderr);

	numServers = iniparser_getboolean(ini, "Rec Servers:numServers", -1);
	printf("numServers:       [%d]\n", b);


	rtpServers.server = (struct rtpServer*)malloc(sizeof(struct rtpServer)*numServers);
	rtpServers.numServers = numServers;
	
	for(j=0;j<numServers;j++)
	{
		rtpServers.server[j].minPort = MIN_PORT_NO;
		rtpServers.server[j].numActiveChannels = 0;

		for(i=0;i<CHANNELS_PER_SERVER;i++)
			rtpServers.server[j].portActive[i] = 0;
		s = iniparser_getstring(ini,"Rec Servers:ipAddress","NULL");
		strcpy(rtpServers.server[j].ip,s);
	}
}

int getFreePort(struct rtpServer *server)
{
	int i;
	int ret = -1;
	for(i=0;i<CHANNELS_PER_SERVER;i++)
	{
		if(server->portActive[i] == 0)
		{
			ret = server->minPort + i;
			server->portActive[i] = 1;
			break;
		}
	}
	return ret;
}

struct rtpServer* getFreeServer()
{
	int i;
	struct rtpServer *server = NULL;
	for(i=0;i<rtpServers.numServers;i++)
	{
		if(rtpServers.server[i].numActiveChannels < CHANNELS_PER_SERVER)
			server = &rtpServers.server[i];

	}
	return server;
}


