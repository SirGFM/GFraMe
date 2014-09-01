#ifndef __FMT_H
#define __FMT_H

#include <GFraMe/wavtodata/readbytes.h>

struct fmt {
	hword fmt;
	hword nchan;
	int btrate;
	int smprate;
	hword bpb;
	hword bps;
};

void getFmt(struct fmt *f, char *data);
void printFmt(struct fmt *f);

#endif

