#ifndef CMEMQUEUESINGLESINGLE_H
#define CMEMQUEUESINGLESINGLE_H

#include <errno.h>
#include <string.h>
#include <stdio.h>

#include <sys/ipc.h>
#include <sys/shm.h>


#include "CQueue.h"

// push, pop操作的结构是{msglen|msgcontent}
class CMemQueueSingle  // 只能使用指针模式，指向共享内存区基址
{
public:
	int size;
    char *base;

    char *pmsgbase;  // 通道1
    CQueue *pq;

public:
    CMemQueueSingle();
    ~CMemQueueSingle();

    int crt(int Size);  // 创建共享内存区

    void init();  // 建立队列在共享内存区上的映射
    void clear()
    {
        pq->clear();
    }

    int full()
    {
        return pq->full();
    }
    int empty()
    {
        return pq->empty();
    }
	int push(char *buf)   // 
    {
        return pq->push(pmsgbase, buf);
    }
    int pop(char *buf)
    {
        return pq->pop(pmsgbase, buf);
    }
    int pop_just()
    {
        return pq->pop_just(pmsgbase);
    }
    int top(char *buf)
    {
        return pq->top(pmsgbase, buf);
    }
	
	
};

#endif
