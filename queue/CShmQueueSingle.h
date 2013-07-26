#ifndef CSHMQUEUESINGLESINGLE_H
#define CSHMQUEUESINGLESINGLE_H

#include <errno.h>
#include <string.h>
#include <stdio.h>

#include <sys/ipc.h>
#include <sys/shm.h>


#include "CQueue.h"



// push, pop操作的结构是{msglen|msgcontent}
class CShmQueueSingle  // 只能使用指针模式，指向共享内存区基址
{
public:
    int SHM_SIZE;
    int FTOLK_VAL;
    int shmid;
    char *base;

    char *pmsgbase;  // 通道1
    CQueue *pq;


public:
    CShmQueueSingle();
    ~CShmQueueSingle();

    int crt(int size, int ftolk_val);  // 创建共享内存区
    int del();
    int get();  // 获取共享内存区基址
    int det();

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
