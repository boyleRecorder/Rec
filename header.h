
/*
This code is LGPL'ed
*/

#ifndef HEADER_H_
#define HEADER_H_

#define CHANNELS_PER_SERVER 500
#define COMMS_PORT          12345 

enum codecList {G711A,G711U} ;

enum channelCreationMessages {CHANNEL_CREATED,CHANNEL_CREATION_FAILED};
enum commands {CREATE_CHANNEL,DELETE_CHANNEL};

/**
These are the server callbacks.
*/
struct callBacks
{
        /**
	Called when there is data to be read on the named socket.
	*/
	void (*readSocketData)(int socket);

        /**
	  Called when the channel is created.
	*/
	void (*channelCreated)(int portNum);
	
        /**
	  Called when the channel is destroyed.
	*/
	void (*channelClosed)(int portNum);
}callBacks;


/**
The message that is used in communication between the client and server.
*/
typedef struct
{
	char      buffer[256];
	int       command;
	int       portNumber;
	int       codec;

}header;


/* RTP Server */
struct rtpServer
{
	char   ip[128];
	int    minPort;
	char   portActive[500];
	int    numActiveChannels;
};

/** List of rtpServers */
struct rtpServers
{
   struct rtpServer* server;
   int        numServers;
}rtpServers;

/** Setups the list of external servers */
void setupRTPServers();

struct rtpServer* getFreeServer();
int getFreePort(struct rtpServer *);

/**********************************
  * Client command functions      *
  *********************************/

/** Request a channel. */
int requestChannel(int chanID, struct rtpServer*, header *);

/** Close a channel. */
void closeChannel(int chanID, struct rtpServer*);


/**
Watches for any incoming RTP data and triggers the 'readSocketData'
call back when there is data ready.

This funciton should never return.

*/
void watchForIncommingData();

/**
Watches for the commands from the client.
Should never return;
*/
void* readCommands(void *arg);


#endif