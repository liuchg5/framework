#ifndef CDBENGINE_H
#define CDBENGINE_H

#include "../msg/StMsgBuffer.h"
#include "../msg/CMsgPara.h"
#include "../msg/CMsgHead.h"
#include "../queue/CShmQueueSingle.h"
#include "../common/common.h"



class CDBEngine
{
public:
	CShmQueueSingle * inq, * outq;
	
	CDBEngine();
	~CDBEngine();
	
	int prepare(CShmQueueSingle * inQ, CShmQueueSingle * outQ);
	int run();
	
private:
	int handle(char * in, char * out);
	int route(char * out, CShmQueueSingle * outq);


	int handle_RequestUserInfo(char * in, char * out);
};

#endif
