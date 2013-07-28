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
}

//------------------------------------------
CSocketInfoList::CSocketInfoList(int Size)
{
    size = Size + 1;
    pv = new CSocketInfo[size];
    if (pv == NULL)
    {
        fprintf(stderr, "Err: CSocketInfoList::CSocketInfoList(int Size)\n");
        fprintf(stderr, "Err: new return NULL \n");
    }
}
CSocketInfoList::~CSocketInfoList()
{
    delete [] pv;
}
int CSocketInfoList::find_idle(int *pindex)
{
    for (int i = 0; i < size; i++)
    {
        if (pv[i].used == 0)
        {
            * pindex = i;
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
}

