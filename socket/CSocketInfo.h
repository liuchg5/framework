#ifndef CSOCKETINFO_H
#define CSOCKETINFO_H

#include <sys/time.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

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
	
	struct timeval tm; // 超时
	int tm_sec;
	int tm_is_update;
	
	CSocketInfo();
	~CSocketInfo();
	void erase();
	void update_tm(struct timeval * now);
	void cal_tm_sec(struct timeval * now)
	{
		tm_sec = now->tv_sec - tm.tv_sec;
	}
};
//------------------------------------------
class CSocketInfoList
{
public:
	CSocketInfo * pv;
	int size;

	int * active_index;
	int active_index_n;
public:	
	CSocketInfoList(int Size);
	~CSocketInfoList();
	int find_idle(int * pindex);
	void erase(int index);
};


#endif
