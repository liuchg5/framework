#include "CSocketMsgHead.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

CSocketMsgHead::CSocketMsgHead()
{
	len = index = sendPos = 0;
}
CSocketMsgHead::CSocketMsgHead(int len, int index, int sendPos)
{
	this->len = len;
	this->index = index;
	this->sendPos = sendPos;
}

CSocketMsgHead::~CSocketMsgHead()
{

}

void CSocketMsgHead::enpack(char * base, int len, int index, int sendPos)
{
	int * p = (int *)base;
	*p++ = len;
	*p++ = index;
	*p = sendPos;
}
void CSocketMsgHead::unpack(char * base, int *len, int *index, int *sendPos)
{
	int * p = (int *)base;
	*len = *p++;
	*index = *p++;
	*sendPos = *p;
}
void CSocketMsgHead::enpack(char * base)
{
	return enpack(base, len, index, sendPos);
}
void CSocketMsgHead::unpack(char * base)
{
	return unpack(base, &len, &index, &sendPos);
}

int CSocketMsgHead::is_close_msg(char * base)
{
    if (*(int *)base == sizeof(CSocketMsgHead))
    {
        fprintf(stderr, "Err: CSocketMsgHead::is_close_msg()\n");
        fprintf(stderr, "Err: index = %d\n", *(int *)(base + sizeof(int)));
        return 1;
    }
    return 0;
}