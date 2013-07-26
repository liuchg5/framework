

#include "CShmQueueSingle.h"



CShmQueueSingle::CShmQueueSingle():
    shmid(-1),
    base(NULL),
    pmsgbase(NULL),
    pq(NULL)
{

}


CShmQueueSingle::~CShmQueueSingle()
{

}

int CShmQueueSingle::crt(int size, int ftolk_val)
{
    SHM_SIZE = size;
    FTOLK_VAL = ftolk_val;
    shmid = shmget(FTOLK_VAL, SHM_SIZE, IPC_CREAT | 0666);
    if (shmid < 0)
    {
        fprintf(stderr, "Err: CShmQueueSingle create shared memory \n");
        fprintf(stderr, "Err: CShmQueueSingle errno = %d (%s) \n", errno, strerror(errno));
		fprintf(stderr, "Err: CShmQueueSingle SHM_SIZE = %d MB \n", SHM_SIZE/1024/1024);
        return shmid;
    }
    fprintf(stdout, "Info: CShmQueueSingle create shared memory \n");
    fprintf(stdout, "Info: CShmQueueSingle shmid = %d \n", shmid);
    return shmid;
}

int CShmQueueSingle::del()
{
    int ret = shmctl( shmid, IPC_RMID, 0 );
    if ( ret == 0 )
        fprintf(stdout, "Info: CShmQueueSingle Shared memory removed \n" );
    else
        fprintf(stderr, "Err: CShmQueueSingle Shared memory remove failed \n" );
    base = NULL;
    return ret;
}

int CShmQueueSingle::get()
{
    if (shmid >= 0)
    {
        base = (char *)shmat(shmid, 0, 0);
        // if (base == ((char *) - 1) )
        // fprintf(stderr, "Err: CShmQueueSingle base == -1, errno = %d   \n", errno);
        // fprintf(stderr, "Err: CShmQueueSingle get(), base = %x  \n", base);
    }
    return 0;
}
int CShmQueueSingle::det()
{
    shmdt(base);
    base = 0;
    return 0;
}







void CShmQueueSingle::init()
{
    int size;
    size = SHM_SIZE - sizeof(CQueue);

    pq = (CQueue *)base;
    pmsgbase = (char *)pq + sizeof(CQueue);

    pq->CQueue_Construction(size);
}

