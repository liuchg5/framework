#ifndef CSOCKET_H
#define CSOCKET_H

#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <error.h>

#include "../msg/StMsgBuffer.h"
#include "../msg/CMsgPara.h"
#include "../msg/CMsgHead.h"
#include "../queue/CShmQueueSingle.h"
#include "../common/common.h"
#include "../common/CStatistics.h"

class CSocketCli
{
public:
	int fd;
	
	int send_flag;
	int recv_n, send_n;
	char * recv_buf, * send_buf;
	
	CShmQueueSingle *inq;
	CShmQueueSingle *outq;
	
	CStatistics sta;
	
	CSocketCli();
	~CSocketCli();
	
	int prepare(const char *servAddr, int portNumber, CShmQueueSingle * inQ, CShmQueueSingle *outQ);
	int run();
private:	
	int handle_close();
	int handle_recv();
	int handle_send();
	
	
	int is_connected();
	
/* 
	int recvflag, sendflag; // 接收和发送消息的标志, 0未接受到完整消息，1接受到完整消息
    StMsgBuffer recvmsg; // 缓存接收到的消息
    StMsgBuffer sendmsg; // 缓存要发送的消息

	
	
	CSocketCli();
	~CSocketCli();
	int open(const char * servAddr, int portNumber);
	int myclose();
	int recv_and_send(CShmQueueSingle * precvQ, CShmQueueSingle * psendQ);
	int is_connected();

	int recv_and_send_debug(CShmQueueSingle * precvQ, CShmQueueSingle * psendQ); */
};








#endif
