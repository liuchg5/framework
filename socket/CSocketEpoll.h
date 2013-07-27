#ifndef CSOCKETEPOLL_H
#define CSOCKETEPOLL_H

#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>




#include "../queue/CShmQueueSingle.h"
#include "../queue/CMemQueueSingle.h"
#include "../common/common.h"

#include "CSocketMsgHead.h"




//------------------------------------------
class CSocketInfo
{
public:
	int used;
	int fd;
	int writable;
	int recv_n;
	char * recv_buf;
	char * recv_base; //base才是new后地址，buf是指预留len和index位置后的地址
	
	// recv_time, send_time; // 
	
	CSocketInfo();
	~CSocketInfo();
	void erase();
};
//------------------------------------------
class CSocketInfoList
{
public:
	CSocketInfo * pv;
	int size;
public:	
	CSocketInfoList(int Size);
	~CSocketInfoList();
	int find_idle(int * pindex);
	void erase(int index);
};
//------------------------------------------
class CSocketEpoll
{
public:
	int port_number; 
	int epoll_size;
	int epoll_timeout;
	int listenq;
	struct epoll_event ev;
	struct epoll_event * events;
	int epfd;

	int cliIsBigEndian; // 
	int srvIsBigEndian;
	
	CSocketInfoList list;
	CShmQueueSingle *outq;
	CShmQueueSingle *inq;
	CMemQueueSingle q1, q2;
	int flag_q1;

public:
	CSocketEpoll(int epoll_Size, int epoll_Timeout, int Listenq, int clientIsBigEndian, int serverIsBigEndian);
	~CSocketEpoll();
	int prepare(const char * serv_addr, int port_num, CShmQueueSingle *poutq, CShmQueueSingle *pinq);
	int run();
private:	
	int handle_recv_and_set_writable();
    int handle_new_connect(int listenfd); // evdata 放 index
    int handle_recv(int index); // 0接收完数据，-1出错或关闭
    int handle_close(int index);
    int handle_info_close(int index);
	
	int handle_send();
	int handle_send_inq_tmpq(CShmQueueSingle *inQ, CMemQueueSingle *tmpQ);	
	int handle_send_inq_tmpq(CMemQueueSingle *inQ, CMemQueueSingle *tmpQ);	
};


#endif
