#include "CStatistics.h"

CStatistics::CStatistics():
    l_interval(5000000),
    l_count(1),
    flag(1),
    status(0)
{
    // l_interval = 5000000; //默认5s
    // l_count = 0;

    // flag = 1; //enable
    // status = 0;//0-init, 1-check
}

CStatistics::~CStatistics()
{

}

// int CStatistics::check(long *pval, long *ptimeuse)  // 返回0表示到了时间间隔，其他表示未到时间间隔
// {
//     if (flag == 0)
//         return -1;

//     if (status == 1)
//     {
//         l_count++;
//         if (l_count % 200 == 0) // for performance
//         {
//             gettimeofday(&st_end, NULL);
//             long timeuse = 1000000 * ( st_end.tv_sec - st_start.tv_sec ) + st_end.tv_usec - st_start.tv_usec;
//             if (timeuse >= l_interval)  // 到了时间间隔
//             {
//                 *pval = l_count;
//                 *ptimeuse = timeuse;
//                 l_count = 1;
//                 gettimeofday(&st_start, NULL);
//                 return 0; //到了时间间隔
//             }
//         }
//         return 1;
//     }
//     else if (status == 0)
//     {
//         status = 1;
//         l_count = 1;
//         gettimeofday(&st_start, NULL);
//     }
//     return 1;
// }

// void CStatistics::check()
// {
//     long lval, ltimeuse;
//     int rtn = check(&lval, &ltimeuse);
//     if (rtn == -1)
//         return ;
//     if (rtn == 1)
//         return ;
//     if (rtn == 0)
//     {
//         fprintf(stdout, "Statistics: [%ld] Per (%ld) mseconds \n",
//                 lval, (ltimeuse / 1000));
//     }
// }

// void CStatistics::check(const char *str)
// {
//     long lval, ltimeuse;
//     int rtn = check(&lval, &ltimeuse);
//     if (rtn == -1)
//         return ;
//     if (rtn == 1)
//         return ;
//     if (rtn == 0)
//     {
//         fprintf(stdout, "Statistics: { %s } [%ld] Per (%ld) seconds \n",
//                 str, lval, (ltimeuse / 1000) );
//     }
// }
