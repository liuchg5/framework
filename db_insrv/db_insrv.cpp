
#include "../common/common.h"
#include "../queue/CShmQueueSingle.h"
#include "../socket/CSocketEpoll.h"

int main(int argc, char const *argv[])
{
	printf("Befin db_insrv !!!\n");
	
	CShmQueueSingle inq, outq;

	outq.crt(1024 * 1024 * 16, 5555);
    outq.get();
    outq.init();
    outq.clear();

    inq.crt(1024 * 1024 * 16, 6666);
    inq.get();
    inq.init();
    inq.clear();

    CSocketEpoll epoll(8, 0, 4, 1);//(int epoll_Size, int epoll_Timeout, int Listenq, int flag_Inside)
    epoll.prepare("192.168.234.128", 12345, &outq, &inq);//(const char * serv_addr, int port_num, CShmQueueSingle *poutq, CShmQueueSingle *pinq);
    epoll.run();

	
	return 0;
}
