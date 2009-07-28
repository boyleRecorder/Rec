#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "wav.h"

#define SECONDS 4 /* produce 10 seconds of noise */
#define PI 3.14159265358979
#define FREQUENCY 8000

static int put_little_short(char *t, unsigned int value)
{
	*(unsigned char *)(t++)=value&255;
	*(unsigned char *)(t)=(value/256)&255;
	return 2;
}

static int put_little_long(char *t, unsigned int value)
{
	*(unsigned char *)(t++)=value&255;
	*(unsigned char *)(t++)=(value/256)&255;
	*(unsigned char *)(t++)=(value/(256*256))&255;
	*(unsigned char *)(t)=(value/(256*256*256))&255;
	return 4;
}

void writeHeader(char *buffer,int numChannels)
{
        strcpy(buffer,"RIFF");
	/* total length will be put in later */
        strcpy(buffer+8,"WAVE"); 
        strcpy(buffer+12,"fmt ");

        put_little_long(buffer+16,16); /* I know the length  of the fmt_ chunk
	     		   */
	put_little_short(buffer+20,1); /* chunk type is always one */
	put_little_short(buffer+22,numChannels); /* two channels */
	put_little_long(buffer+24,FREQUENCY); /* samples per second */
	put_little_long(buffer+28,FREQUENCY*2*numChannels); /* bytes per second */
	put_little_short(buffer+32,2*numChannels); /* bytes pro sample (all channels) */
	put_little_short(buffer+34,16); /* bits per sample */

	/* data chunk, 8 bytes header and XXX bytes payload */
        strcpy(buffer+36,"data"); 
	
}

/**
 * \param length The length of the payload.
 */
void closeHeader(char *buffer, int length)
{
	put_little_long(buffer+40,length);
	put_little_long(buffer+4,length+8+16+8);
}

#ifdef DEBUG_TEST

/* returns the number of bytes written. skips two bytes after
 * each write */
static int fill_data(char *start, int frequency, int seconds)
{
	int i, len=0;
	int value;
	for(i=0; i<seconds*FREQUENCY; i++) {
		value=32767.0 *
			sin(2.0*PI*((double)(i))*(double)(frequency)/FREQUENCY);
		put_little_short(start, value);
		start += 4;
		len+=2;
	}
	printf("len: %i\n",len);
	return len;
}


int main(void)
{
	char *buffer=malloc(SECONDS*FREQUENCY*4+1000);
	char *t=buffer;
	int len;
	int fd;

        writeHeader(buffer,2);

	len=fill_data(buffer+44,450,SECONDS); /* left channel, 450Hz sine */
	len+=fill_data(buffer+44+2,452,SECONDS); /* right channel, 452Hz sine */
	
	closeHeader(buffer,len);
	
	fd=open("test.wav", O_RDWR|O_CREAT, 0644);
	write(fd,buffer,len+8+16+8+8);
	close(fd);
	return 0;
}

#endif
