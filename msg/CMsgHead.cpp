#include "CMsgHead.h"


void CMsgHead::print()
	{
		printf("msglen = %d\n", msglen);
		printf("msgid = %d\n", msgid);
		printf("msgtype = %d\n", msgtype);
		printf("msgseq = %d\n", msgseq);
		printf("srcfe = %d\n", srcfe);
		printf("dstfe = %d\n", dstfe);
		printf("srcid = %d\n", srcid);
		printf("dstid = %d\n", dstid);
	}
void CMsgHead::encode()
{
    ::encode_int(&msglen);
    ::encode_short(&msgid);
    ::encode_short(&msgtype);
    ::encode_int(&msgseq);
    ::encode_short(&srcid);
    ::encode_short(&dstid);
}