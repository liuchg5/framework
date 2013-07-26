
#include "../common/common.h"
#include "../queue/CShmQueueSingle.h"
#include "../engine/CGMEngine.h"


int main(int argc, char const *argv[])
{
	printf("Befin midsrv !!!\n");
	
	CShmQueueSingle inq, outq, db_inq, db_outq;

	outq.crt(1024 * 1024 * 16, 2222);
    outq.get();
    outq.init();
    outq.clear();

    inq.crt(1024 * 1024 * 16, 1111);
    inq.get();
    inq.init();
    inq.clear();

    db_outq.crt(1024 * 1024 * 16, 4444);
    db_outq.get();
    db_outq.init();
    db_outq.clear();

    db_inq.crt(1024 * 1024 * 16, 3333);
    db_inq.get();
    db_inq.init();
    db_inq.clear();

    CGMEngine eng;
    eng.prepare(&inq, &outq, &db_inq, &db_outq);//(CShmQueueSingle * inQ, CShmQueueSingle * outQ, CShmQueueSingle * db_inQ, CShmQueueSingle * db_outQ);
    eng.run();
    
	return 0;
}