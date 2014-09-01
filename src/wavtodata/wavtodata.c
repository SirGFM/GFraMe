#include <GFraMe/wavtodata/chunk.h>
#include <GFraMe/wavtodata/fmt.h>
#include <GFraMe/wavtodata/wavtodata.h>
#include <stdio.h>
#include <stdlib.h>

int my_strcmp(char id[4], const char *s);
int my_strlen(char *str);
void printSeparator();
FILE* promptFile(const char *mode, int force);
FILE* getFile(const char *name, const char *mode, int force);
int checkFmt(struct fmt *f);

//int main(int argc, char *argv[]) {
int wavtodata(char *infile, char *outfile, int force) {
	FILE *file;
	FILE *out = NULL;
	struct chunk fd;
	struct chunk c;
	struct fmt f;
	char *data;
	/*
	if (argc == 2) {
		file = getFile(argv[1], "rb", 1);
		printf("Output File: ");
		out = promptFile("wb", 1);
	}
	else if (argc == 3) {
		file = getFile(argv[1], "rb", 1);
		out = getFile(argv[2], "wb", 1);
	}
	else if (argc > 3) {
		printf("Too many arguments.\nExpected 1 or 2, got %i.\n", argc);
		return 1;
	}
	else {
		printf("Input File: ");
		file = promptFile("rb", 1);
		printf("Output File: ");
		out = promptFile("wb", 1);
	}
	*/
	file = getFile(infile, "rb", force);
	out = getFile(outfile, "wb", force);
	if (!(file && out)) {
		if (file)
			fclose(file);
		if (out)
			fclose(out);
		return 1;
	}
	
	// Gather initial info from the file
	readChunkInfo(file, &fd);
	
	printSeparator();
	printChunk(&fd);
	printSeparator();
	
	// Check if it's indeed a WAVE file
	data = (char*)malloc(sizeof(char)*4);
	fread(data, 4, sizeof(char), file);
	if (!my_strcmp(data, "WAVE")) {
		printf("Not a WAVE file!\n");
		free(data);
		fclose(file);
		return 1;
	}
	free(data);
	
	// Loop through every chunk
	fd.read = 4;
	while (fd.read < fd.len) {
		readChunkInfo(file, &c);
		fd.read += 8;
		if (my_strcmp(c.id, "fmt ")) {
			data = (char*)malloc(sizeof(char)*c.len);
			readChunkBytes(file, &c, data, c.len);
			getFmt(&f, data);
			free(data);
			
			printSeparator();
			printFmt(&f);
			printSeparator();
			
			if (!checkFmt(&f)) {
				fclose(file);
				fclose(out);
				return 1;
			}
		}
		else if (my_strcmp(c.id, "LIST")) {
			printSeparator();
			printf("List chunk safely ignored\n");
			printSeparator();
			
			fseek(file, c.len, SEEK_CUR);
		}
		else if (my_strcmp(c.id, "data")) {
			printSeparator();
			printf("Data found\n");
			printf("Reading samples...\n");
			data = (char*)malloc(sizeof(char)*f.bpb);
			while(c.read < c.len) {
				readChunkBytes(file, &c, data, f.bpb);
				if (f.nchan == 1) {
					fwrite(data, sizeof(char), 2, out);
					fwrite(data, sizeof(char), 2, out);
				}
				else if (f.nchan == 2)
					fwrite(data, sizeof(char), 4, out);
			}
			printSeparator();
			free(data);
		}
		else {
			printSeparator();
			printf("Unexpected chunk\n");
			printSeparator();
			
			fseek(file, c.len, SEEK_CUR);
		}
		fd.read += c.len;
	}
	
	fclose(file);
	fclose(out);
	return 0;
}

int my_strcmp(char id[4], const char *s) {
	int i = 0;
	while (i < 4) {
		if (id[i] != s[i])
			return 0;
		i++;
	}
	return 1;
}

int my_strlen(char *str) {
	int i = -1;
	while (str[++i] != '\0');
	return i;
}

void printSeparator() {
	printf("--------------------------------\n");
}

FILE* promptFile(const char *mode, int force) {
	FILE *tmp;
	char *filename;
	int l;
	printf("Insert the WAVE file name: ");
	filename = (char*)malloc(sizeof(char)*256);
	fgets(filename, 256, stdin);
	l = my_strlen(filename);
	if (filename[l-1] == '\n')
		filename[l-1] = '\0';
	tmp = getFile(filename, mode, force);
	free(filename);
	return tmp;
}

FILE* getFile(const char *name, const char *mode, int force) {
	FILE *tmp;
	
	if (!force && mode[0] == 'w') {
		tmp = fopen(name, "rb");
		if (tmp) {
			printf("File %s already exists!\n", name);
			fclose(tmp);
			return NULL;
		}
	}
	tmp = fopen(name, mode);
	if (!tmp && mode[0] == 'r')
		printf(" File %s not found!\n", name);
	return tmp;
}

int checkFmt(struct fmt *f) {
	if (f->smprate != 44100) {
		printf("Unsuported sample rate (must be 44.1KHz)\n");
		return 0;
	}
	else if (f->nchan != 1 && f->nchan != 2) {
		printf("Unsuported number of channels (either 1 or 2)\n");
		return 0;
	}
	else if (f->bps != 16) {
		printf("Unsuported bits per sample (must be 16 bits)\n");
		return 0;
	}
	return 1;
}
