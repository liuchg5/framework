#ifndef CGMENGINE_H
#define CGMENGINE_H

#include "../msg/StMsgBuffer.h"
#include "../msg/CMsgPara.h"
#include "../msg/CMsgHead.h"

#include "../queue/CShmQueueSingle.h"
#include "../common/common.h"

#include "../socket/CSocketMsgHead.h"



class CGMEngine
{
public:
	CShmQueueSingle * outq, *inq;
	CShmQueueSingle * db_outq, *db_inq;
	int cliIsBigEndian; // 
	int srvIsBigEndian;
	
	CGMEngine(int cliIsBigEndian, int srvIsBigEndian);
	~CGMEngine();

	int prepare(CShmQueueSingle * inQ, CShmQueueSingle * outQ, CShmQueueSingle * db_inQ, CShmQueueSingle * db_outQ);
	int run();
	
private:
	int handle_client(char * in, char * out);
	int handle_db(char * in, char * out);
	int route(char * out, CShmQueueSingle * outq, CShmQueueSingle * db_outq);

	int handle_RequestLogin(char * in, char * out);
	int handle_RequestUserInfo(char * in, char * out);


	 // int handle_debug(StMsgBuffer * pinmsg, CShmQueueMulti * pshmqm);  //debug
	 // int handle_MSGID_I2M_LOGIN(StMsgBuffer * pinmsg, CShmQueueMulti * pshmqm);//debug
};

#endif
