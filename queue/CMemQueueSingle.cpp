

#include "CMemQueueSingle.h"



CMemQueueSingle::CMemQueueSingle():
    base(NULL),
    pmsgbase(NULL),
    pq(NULL)
{

}


CMemQueueSingle::~CMemQueueSingle()
{

}

int CMemQueueSingle::crt(int Size)
{
	size = Size;
    base = new char[Size];
	if (base == NULL)
	{
		fprintf(stderr, "Err: CMemQueueSingle create failed \n");
        fprintf(stderr, "Err: CMemQueueSingle errno = %d (%s) \n", errno, strerror(errno));
		return -1;
	}
    return 0;
}

void CMemQueueSingle::init()
{
    pq = (CQueue *)base;
    pmsgbase = (char *)pq + sizeof(CQueue);

    pq->CQueue_Construction(size - sizeof(CQueue));
}

