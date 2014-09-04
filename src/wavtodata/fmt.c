#include <stdio.h>
#include <GFraMe/wavtodata/readbytes.h>
#include <GFraMe/wavtodata/fmt.h>

void getFmt(struct fmt *f, char *data) {
	f->fmt = readHWORD(data);
	f->nchan = readHWORD(data+2);
	f->smprate = readWORD(data+4);
	f->btrate = readWORD(data+8);
	f->bpb = readHWORD(data+12);
	f->bps = readHWORD(data+14);
}

void printFmt(struct fmt *f) {
	printf("Format: ");
	switch (f->fmt) {
		case 1: printf("PCM"); break;
		case 3: printf("IEEE FLOAT"); break;
		case 6: printf("???"); break;
		case 7: printf("???"); break;
		default: 
			if ((int)f->fmt == 0xfffe)
				printf("CUSTOM");
		break;
	}
	
	printf("\nNumber of channels: %i\n", f->nchan);
	printf("Sample rate: %i\n", f->smprate);
	printf("Byte rate: %i\n", f->btrate);
	printf("Bytes per block: %i (%i bits)\n", f->bpb, f->bpb*8);
	printf("Bits per sample: %i\n", f->bps);
}

