

#include "../queue/CShmQueueSingle.h"
#include "../socket/CSocketEpoll.h"
#include "../common/common.h"

int main(int argc, char const *argv[])
{
    printf("Befin insrv !!!\n");
	printf("serverIsBigEndian == %d !!!\n", isBigEndian());
	
	CShmQueueSingle inq, outq;

	outq.crt(1024 * 1024 * 16, 1111);
    outq.get();
    outq.init();
    outq.clear();

    inq.crt(1024 * 1024 * 16, 2222);
    inq.get();
    inq.init();
    inq.clear();

    CSocketEpoll epoll(8000, 0, 1000, 0, 1);//(int epoll_Size, int epoll_Timeout, int Listenq, int clientIsBigEndian, int serverIsBigEndian)
    epoll.prepare("0.0.0.0", 10203, &outq, &inq);//(const char * serv_addr, int port_num, CShmQueueSingle *poutq, CShmQueueSingle *pinq);
    epoll.run();
	
	
	// test_socket_srv("192.168.190.131", 10203);
	
	return 0;
}
