#ifndef STMSGBUFFER_H
#define STMSGBUFFER_H

#include <stdint.h>

#define STMSGBUFFER_SIZE	1024

struct StMsgBuffer
{
	char buf[STMSGBUFFER_SIZE];
	uint32_t n;
};







#endif
