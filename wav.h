
#ifndef WAV_H__
#define WAV_H__

#define WAV_HDR 44

typedef struct
{
	FILE *stream;
	int  length;
	char header[WAV_HDR];
	char fileName[128];
}FileSink;

FileSink *createWavSink(char *, int channels);

void closeWavSink(FileSink *);

void writeData(FileSink *, char *, int );

void writeHeader(char *buffer,int numChannels);
void closeHeader(char *buffer, int length);


#endif
