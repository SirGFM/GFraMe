#include <GFraMe/wavtodata/chunk.h>
#include <GFraMe/wavtodata/readbytes.h>

void readChunkInfo(FILE *f, struct chunk *c) {
	char tmp[4];
	
	fread(c->id, sizeof(char), 4, f);
	fread(tmp, sizeof(char), 4, f);
	c->len = readWORD(tmp);
	c->read = 0;
}

int readChunkBytes(FILE *f, struct chunk *c, char *buffer, int max) {
	int ret = fread(buffer, sizeof(char), max, f);
	c->read += ret;
	return ret;
}

void printChunk(struct chunk *c) {
	printf("ID: %c%c%c%c\n", c->id[0], c->id[1], c->id[2], c->id[3]);
	printf("Length: %i\n", c->len);
	printf("Bytes read: %i\n", c->read);
}

