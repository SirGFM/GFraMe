#ifndef __READBYTES_H
#define __READBYTES_H

typedef short hword;
#define readHWORD(h) (((h)[0]&0xff) | (((h)[1]<<8)&0xff00))
#define readWORD(w)	(((w)[0]&0xff) | (((w)[1]<<8)&0xff00) | (((w)[2]<<16)&0xff0000) | (((w)[3]<<24)&0xff000000))

#endif

