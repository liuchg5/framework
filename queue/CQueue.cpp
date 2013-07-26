
#include "CQueue.h"
//  不再受最大msg的限制，完全使用可用的字节空间




int CQueue::push(char *base, char *buf)   // 原子操作，压入完整msg 压入msg比较简单，因为指定了len
{
    int n, empty_n;
    int len = *((int *)buf);
    n = size();
    empty_n = NMAX - n;
    if (empty_n < len)
        return -1; //剩余的空间不足
    if (buf == NULL)
    {
        return -2;
    }
    //分两种情况压入
    if (w >= r)
    {
        n = (NMAX - w);
        if (n >= len)  // 可以直接写入
        {
            memcpy(base + w, buf, len);
            w += len;
            return len;// 返回写入的长度
        }
        // 要分两次写入
        memcpy(base + w, buf, n);
        memcpy(base, buf + n, len - n);
        w = len - n;
        return len; // 返回写入的长度
    }
    else  // w < r
    {
        // 可以一次写入
        memcpy(base + w, buf, len);
        w += len;
        return len;
    }
}
int CQueue::pop(char *base, char *buf)  // 原子操作，读取完整msg  遵循的规则是只要非空，就一定有msg可以读取
{
    int n, len;
    n = size();
    if (n == 0)
        return -1;//空队列
    if (n < 4)
    {
        fprintf(stderr, "Err: CQueue's n < 4! \n");
        return -1;
    }
    if (w >= r)
    {
        // 直接读即可
        memcpy(&len, base + r, 4); //获取长度值
        if (len > size()) // 只有长度值
        {
            return -1;
        }
        memcpy(buf, base + r, len);
        r += len;
        return (len);
    }
    else   // w < r
    {
        n = (NMAX - r); //r_to_end();
        // 先获取长度值
        if (n >= 4)
        {
            memcpy(&len, base + r, 4); //获取长度值
        }
        else
        {
            memcpy(&len, base + r, n);
            memcpy(((char *)&len + n), base, 4 - n);
        }
        if (len > size()) // 只有长度值
        {
            return -1;
        }
        if (n >= len)
        {
            memcpy(buf, base + r, len);
            r += len;
        }
        else
        {
            memcpy(buf, base + r, n);
            memcpy(buf + n, base, len - n);
            r = len - n;
        }
        return len;
    }
}

int CQueue::pop_just(char *base) // 原子操作，读取完整msg  遵循的规则是只要非空，就一定有msg可以读取
{
    int n, len;
    n = size();
    if (n == 0)
        return -1;//空队列
    if (n < 4)
    {
        fprintf(stderr, "Err: CQueue's n < 4! \n");
        return -1;
    }
    if (w >= r)
    {
        // 直接读即可
        memcpy(&len, base + r, 4); //获取长度值
        if (len > size()) // 只有长度值
        {
            return -1;
        }
        r += len;
        return (len);
    }
    else   // w < r
    {
        n = (NMAX - r); //r_to_end();
        // 先获取长度值
        if (n >= 4)
        {
            memcpy(&len, base + r, 4); //获取长度值
        }
        else
        {
            memcpy(&len, base + r, n);
            memcpy(((char *)&len + n), base, 4 - n);
        }
        if (len > size()) // 只有长度值
        {
            return -1;
        }
        if (n >= len)
        {
            r += len;
        }
        else
        {
            r = len - n;
        }
        return len;
    }
}

int CQueue::top(char *base, char *buf)  // 原子操作，读取完整msg  遵循的规则是只要非空，就一定有msg可以读取
{
    int n, len;
    n = size();
    if (n == 0)
        return -1;//空队列
    if (n < 4)
    {
        fprintf(stderr, "Err: CQueue's n < 4! \n");
        return -1;
    }
    if (w >= r)
    {
        // 直接读即可
        memcpy(&len, base + r, 4); //获取长度值
        if (len > size()) // 只有长度值
        {
            return -1;
        }
        memcpy(buf, base + r, len);
        //r += len;
        return (len);
    }
    else   // w < r
    {
        n = (NMAX - r); //r_to_end();
        // 先获取长度值
        if (n >= 4)
        {
            memcpy(&len, base + r, 4); //获取长度值
        }
        else
        {
            memcpy(&len, base + r, n);
            memcpy(((char *)&len + n), base, 4 - n);
        }
        if (len > size()) // 只有长度值
        {
            return -1;
        }
        if (n >= len)
        {
            memcpy(buf, base + r, len);
            //r += len;
        }
        else
        {
            memcpy(buf, base + r, n);
            memcpy(buf + n, base, len - n);
            // r = len -n;
        }
        return len;
    }
}

int CQueue::top_just(char *base) // 原子操作，读取完整msg  遵循的规则是只要非空，就一定有msg可以读取
{
    int n;
    int len = 0;
    n = size();

    if (n == 0)
        return -1;//空队列

    if (n < 4)
    {
        // fprintf(stderr, "Err: CQueue's n < 4! \n");
        return -1;
    }
    if (w >= r)
    {
        // 直接读即可
        memcpy(&len, base + r, 4); //获取长度值
        if (len > size()) // 只有长度值
        {
            return -1;
        }
        // memcpy(buf, base+r, len);
        //r += len;
        return (len);
    }
    else   // w < r
    {
        n = (NMAX - r); //r_to_end();
        // 先获取长度值
        if (n >= 4)
        {
            memcpy(&len, base + r, 4); //获取长度值
        }
        else
        {
            memcpy(&len, base + r, n);
            memcpy(((char *)&len + n), base, 4 - n);
        }
        if (len > size()) // 只有长度值
        {
            return -1;
        }
        // if (n >= len){
        //           // memcpy(buf, base+r, len);
        //           //r += len;
        //    }else{
        //           memcpy(buf, base+r, n);
        //           memcpy(buf+n, base, len - n);
        //          // r = len -n;
        //    }
        return len;
    }
}
