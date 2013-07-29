#include "CSocketInfo.h"

#include "CSocketMsgHead.h"




//------------------------------------------
CSocketInfo::CSocketInfo()
{
    used = 0; fd = -1; writable = 1;
    recv_n = 0;
    recv_base = new char[1024];
    if (recv_base == NULL)
    {
        fprintf(stderr, "Err: CSocketInfo::CSocketInfo()\n");
        fprintf(stderr, "Err: new failed !!!\n");
    }
    recv_buf = recv_base + sizeof(CSocketMsgHead);
    memset(recv_base, 0, 1024);
}
CSocketInfo::~CSocketInfo()
{
    delete [] recv_base;
}
void CSocketInfo::erase()
{
    used = 0; fd = -1; writable = 1;
    recv_n = 0;
    memset(recv_base, 0, 1024);

    // memset(tm, 0, sizeof(struct timeval));
}

void CSocketInfo::update_tm(struct timeval *now)
{
    memcpy(&tm, now, sizeof(struct timeval));
    tm_is_update = 1;
}

//------------------------------------------
CSocketInfoList::CSocketInfoList(int Size)
{
    size = Size + 1;
    pv = new CSocketInfo[size];
    active_index = new int[size];
    active_index_n = 0;
    if (pv == NULL || active_index == NULL)
    {
        fprintf(stderr, "Err: CSocketInfoList::CSocketInfoList(int Size)\n");
        fprintf(stderr, "Err: new return NULL \n");
    }
}
CSocketInfoList::~CSocketInfoList()
{
    delete [] pv;
    delete [] active_index;
}
int CSocketInfoList::find_idle(int *pindex)
{
    for (int i = 0; i < size; i++)
    {
        if (pv[i].used == 0)
        {
            * pindex = i;
            active_index[active_index_n++] = i;//加入激活队列
            return 0;
        }
    }
    fprintf(stderr, "Err: CSocketInfoList::find_idle(int * pindex)\n");
    fprintf(stderr, "Err: is full !!! \n");
    fprintf(stderr, "Err: errno = %d (%s) \n ", errno, strerror(errno));
    *pindex = -1;
    return -1;
}
void CSocketInfoList::erase(int index)
{
    pv[index].erase();

    // 在激活的数组中删除
    for (int j = 0; j < active_index_n; j++)
    {
        if (active_index[j] != index) // 找到j
            continue;
        if (j == active_index_n - 1)//最后一个元素
        {
            active_index[--active_index_n] = -3;
            return;
        }
        for (int i = j; i < active_index_n - 1; i++)
        {
            active_index[i] = active_index[i + 1];
        }
        active_index[--active_index_n] = -3;
        return;
    }
    fprintf(stderr, "CSocketInfoList::erase()\n");
    fprintf(stderr, " cannot find index position!\n");


}

