
#include "../common/common.h"
#include "../queue/CShmQueueSingle.h"
#include "../engine/CDBEngine.h"

int main(int argc, char const *argv[])
{
	printf("Befin db_midsrv !!!\n");
	
	CShmQueueSingle inq, outq;

	outq.crt(1024 * 1024 * 16, 6666);
    outq.get();
    outq.init();
    outq.clear();

    inq.crt(1024 * 1024 * 16, 5555);
    inq.get();
    inq.init();
    inq.clear();

    CDBEngine dbe;
    dbe.prepare(&inq, &outq);//(CShmQueueSingle * inQ, CShmQueueSingle * outQ);
    dbe.run();

	
	return 0;
}
