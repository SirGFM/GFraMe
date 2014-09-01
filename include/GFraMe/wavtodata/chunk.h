#ifndef __CHUNK_H
#define __CHUNK_H

#include <stdio.h>

struct chunk {
	char id[4];
	int len;
	int read;
};

void readChunkInfo(FILE *f, struct chunk *c);
int readChunkBytes(FILE *f, struct chunk *c, char *buffer, int max);
void printChunk(struct chunk *c);

#endif

