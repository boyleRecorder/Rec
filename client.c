#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#include <stdlib.h>
#include <string.h>

#include "header.h"

void error(char *msg)
{
    perror(msg);
    exit(0);
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

void setupRTPServers()
{
  rtpServers.numServers = 1;
  rtpServers.server = (struct rtpServer*)malloc(sizeof(struct rtpServer));
  
  rtpServers.server[0].minPort = 32000;
  rtpServers.server[0].maxPort = 32500;
  rtpServers.server[0].numActiveChannels = 0;
  strcpy(rtpServers.server[0].ip,"10.0.0.5");
    
}

int requestChannel(int chanID, struct rtpServer* mediaServer, header *msg)
{
    int ret = 0;
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
    
    if (connect(sockfd,&serv_addr,sizeof(serv_addr)) < 0) 
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
#ifdef ACK_TCP
    bzero(msg->buffer,256);

    n = read(sockfd,msg->buffer,255);
    if (n < 0)
    { 
         error("ERROR reading from socket");
	 return CHANNEL_CREATION_FAILED;
    }
#endif    
    close(sockfd);
    printf("%s\n",msg->buffer);
    return CHANNEL_CREATED;
}


int main(int argc, char *argv[])
{
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];
    /// Setup the test message.
    header myHeader;
    strcpy(myHeader.buffer,"test message");
    myHeader.codec = G711A;
    myHeader.portNumber = 9876;
    myHeader.command = CREATE_CHANNEL;

    // Init the servers
    setupRTPServers();

    // Request a channel
    int i;
    requestChannel(1,getFreeServer(),&myHeader);
    myHeader.command = DELETE_CHANNEL;

    myHeader.portNumber;
    requestChannel(1,getFreeServer(),&myHeader);
    
    return 0;
}
