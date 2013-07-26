
#include "../common/common.h"
#include "../queue/CShmQueueSingle.h"
#include "../socket/CSocketCli.h"

int main(int argc, char ** argv)
{
	printf("Befin outsrv !!!\n");
	
	CShmQueueSingle inq, outq ;

	outq.crt(1024 * 1024 * 16, 3333);
    outq.get();
    outq.init();
    outq.clear();

    inq.crt(1024 * 1024 * 16, 4444);
    inq.get();
    inq.init();
    inq.clear();


    CSocketCli cli;
    cli.prepare("192.168.234.128", 12345, &inq, &outq);
    //(const char *servAddr, int portNumber, CShmQueueSingle * inQ, CShmQueueSingle *outQ);
    if (cli.run() != 0)
        return -1;
    
	return 0;
}

