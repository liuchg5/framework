#ifndef CSTATISTICS_H
#define CSTATISTICS_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

class CStatistics
{
public:
    long l_interval; //us
    long l_count;

    struct timeval st_start;
    struct timeval st_end;

    int flag;
    int status;//0-init, 1-check

    CStatistics();
    ~CStatistics();

    void set_interval(long val_us)
    {
        l_interval = val_us;
    }
    long get_interval()
    {
        return l_interval;
    }
    void enable()
    {
        flag = 1;
        status = 0;
    }
    void disable()
    {
        flag = 0;
    }
    int check(long *pval, long *ptimeuse)  // 返回0表示到了时间间隔，其他表示未到时间间隔
    {
        if (flag == 0)
            return -1;

        if (status == 1)
        {
            l_count++;
            // if (l_count % 200 == 0) // for performance
            // {
                gettimeofday(&st_end, NULL);
                long timeuse = 1000000 * ( st_end.tv_sec - st_start.tv_sec ) + st_end.tv_usec - st_start.tv_usec;
                if (timeuse >= l_interval)  // 到了时间间隔
                {
                    *pval = l_count;
                    *ptimeuse = timeuse;
                    l_count = 1;
                    gettimeofday(&st_start, NULL);
                    return 0; //到了时间间隔
                }
            // }
            return 1;
        }
        else if (status == 0)
        {
            status = 1;
            l_count = 1;
            gettimeofday(&st_start, NULL);
        }
        return 1;
    }
    // void check();
    void check(const char *str)
    {
        long lval, ltimeuse;
        int rtn = check(&lval, &ltimeuse);
        // if (rtn == -1)
        //     return ;
        // if (rtn == 1)
        //     return ;
        if (rtn == 0)
        {
            fprintf(stdout, "Statistics: { %s } [%ld] Per seconds (%ld)  \n",
                    str, lval/5, (ltimeuse / 1000) );
        }
    }
	
	



};





#endif
