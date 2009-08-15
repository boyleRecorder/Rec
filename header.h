
/*
This code is LGPL'ed
*/

#ifndef HEADER_H_
#define HEADER_H_

#define CHANNELS_PER_SERVER     500
#define COMMS_PORT              12345 
#define MIN_PORT_NO		32000

#define CONFIG_FILE		"config.ini"
#define RTP_HEADER		12

#include "wav.h"

enum channelCreationMessages {CHANNEL_CREATED,CHANNEL_CREATION_FAILED};

/** The commands that can be send to the recording server. */
enum commands {CREATE_CHANNEL,DELETE_CHANNEL};

pthread_mutex_t mutex;

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
	void (*channelCreated)(int portNum, char *chanID, int codec);
	
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
	char      callID[256];
	int       command;
	int       portNumber;
	int       codec;

}header;


/* RTP Server */
struct rtpServer
{
	char   ip[128];
	int    minPort;
	char   portActive[CHANNELS_PER_SERVER];
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

/** Gets the details of a free recording server. */
struct rtpServer* getFreeServer();

/**
 * Get a free port in the named server.
 */
int getFreePort(struct rtpServer *);

/**********************************
  * Client command functions      *
  *********************************/

/** Request a channel. */
int requestChannel(int chanID, struct rtpServer*, header *);

/** Close a channel. */
int closeChannel(int chanID, struct rtpServer*);


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
