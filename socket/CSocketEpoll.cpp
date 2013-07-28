

#include "CSocketEpoll.h"

#include "../msg/CMsgHead.h"
#include "../msg/CMsgPara.h"


//------------------------------------------
//------------------------------------------
//------------------------------------------
CSocketEpoll::CSocketEpoll(int epoll_Size, int epoll_Timeout, int Listenq, int clientIsBigEndian, int serverIsBigEndian, int flag_timeout): list(epoll_Size), heap(epoll_Size, &list)//qs(epoll_Size, &list)
{
    port_number = -1;
    epoll_size = epoll_Size + 1;
    epoll_timeout = epoll_Timeout;
    listenq = Listenq;
    events = new struct epoll_event[epoll_size];
    if (events == NULL)
    {
        fprintf(stderr, "Err: CSocketEpoll::CSocketEpoll()\n");
        fprintf(stderr, "Err: new return NULL \n");
        fprintf(stderr, "Err: errno = %d (%s) \n ", errno, strerror(errno));
    }
    epfd = 0;
    cliIsBigEndian = clientIsBigEndian;
    srvIsBigEndian = serverIsBigEndian;
    this->flag_timeout = flag_timeout;
}

CSocketEpoll::~CSocketEpoll()
{
    delete [] events;
}

int CSocketEpoll::prepare(const char *serv_addr, int port_num, CShmQueueSingle *poutq, CShmQueueSingle *pinq)
{
    port_number = port_num;
    outq = poutq;
    inq = pinq;

    q1.crt(1024 * 1024 * 1);
    q1.init();
    q1.clear();

    q2.crt(1024 * 1024 * 1);
    q2.init();
    q2.clear();

    flag_q1 = 1;

    int ret;
    // 创建epoll
    epfd = epoll_create(epoll_size);
    if (epfd < 0)
    {
        fprintf(stderr, "Err: CSocketEpoll::prepare() \n ");
        fprintf(stderr, "Err: epoll_create() failed \n ");
        fprintf(stderr, "Err: errno = %d (%s) \n ", errno, strerror(errno));
        return epfd;
    }

    // 打开监听的socket描述符
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0)
    {
        fprintf(stderr, "Err: CSocketEpoll::prepare() \n ");
        fprintf(stderr, "Err: socket() failed \n ");
        fprintf(stderr, "Err: errno = %d (%s) \n ", errno, strerror(errno));
        return listenfd;
    }

    // 设为非阻塞
    if (setnonblocking(listenfd) != 0)
    {
        fprintf(stderr, "Err: CSocketEpoll::prepare() \n ");
        fprintf(stderr, "Err: setnonblocking() failed \n ");
        fprintf(stderr, "Err: errno = %d (%s) \n ", errno, strerror(errno));
        return -1;
    }

    // 注册epoll事件
    ev.data.u32 = 0; // index = 0
    ev.events = EPOLLIN  ;//响应读事件
    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev);
    if (ret < 0)
    {
        fprintf(stderr, "Err: CSocketEpoll::prepare() \n ");
        fprintf(stderr, "Err: epoll_ctl() failed \n ");
        fprintf(stderr, "Err: errno = %d (%s) \n ", errno, strerror(errno));
        return ret;
    }

    // 注册socketlist
    list.pv[0].used = 1;
    list.pv[0].fd = listenfd;

    // 处理服务器地址
    struct sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    inet_aton(serv_addr, &(serveraddr.sin_addr));
    serveraddr.sin_port = htons(port_number);

    // 将socket描述符和服务器地址端口绑定
    ret = bind(listenfd, (sockaddr *) &serveraddr, sizeof(serveraddr));
    if (ret < 0)
    {
        fprintf(stderr, "Err: CSocketEpoll::prepare() \n ");
        fprintf(stderr, "Err: bind() failed \n ");
        fprintf(stderr, "Err: errno = %d (%s) \n ", errno, strerror(errno));
        return ret;
    }

    // 开始监听
    ret = listen(listenfd, listenq);
    if (ret < 0)
    {
        fprintf(stderr, "Err: CSocketEpoll::prepare() \n ");
        fprintf(stderr, "Err: listen() failed \n ");
        fprintf(stderr, "Err: errno = %d (%s) \n ", errno, strerror(errno));
        return ret;
    }
    return 0;
}

int CSocketEpoll::run()
{
    while (1)
    {
        // 获取本次时间
        gettimeofday(&now_tm, NULL);

        // 处理recv
        handle_recv_and_set_writable();

        // 处理send
        handle_send();

        // 处理超时
        if (flag_timeout)
            handle_timeout();

        // sleep
        usleep(10);

    } // end while(1)
    return 0;
}

int CSocketEpoll::handle_recv_and_set_writable()
{
    int index;
    int socketfd;
    int listenfd = list.pv[0].fd;

    int nfds = epoll_wait(epfd, events, epoll_size, epoll_timeout);
    if ( nfds < 0 )
    {
        fprintf(stderr, "Err: CSocketEpoll::handle_recv_and_set_writable() \n");
        fprintf(stderr, "Err: epoll_wait() return neg \n");
        fprintf(stderr, "Err: errno = %d (%s) \n", errno, strerror(errno));// system interupter ?
        // return -1;  // for system interrupt so donot return
    }

    for (int i = 0; i < nfds; ++i)
    {
        index = events[i].data.u32;  // 获取fd
        socketfd = list.pv[index].fd;

        if (index < 0)
        {
            fprintf(stderr, "Err: CSocketEpoll::handle_recv_and_set_writable() \n");
            fprintf(stderr, "Err: index is neg \n ");
            return -1;
        }
        //-------------------- 一个新socket用户连接到监听端口，建立新的连接
        if (socketfd == listenfd)
        {
            if (handle_new_connect(listenfd) != 0)
            {
                fprintf(stderr, "Err: CSocketEpoll::handle_recv_and_set_writable() \n");
                fprintf(stderr, "Err: handle_new_connect() != 0 \n ");
                return -1;
            }
            continue;
        }
        // ------------------- 读取数据
        if (events[i].events & EPOLLIN)
        {
            if (handle_recv(index) != 0)
            {
                fprintf(stderr, "Err: CSocketEpoll::handle_recv_and_set_writable() \n");
                fprintf(stderr, "Err: handle_recv() != 0 \n ");
                return -1;
            }
        }
        // ------------------------ 发送数据
        if (events[i].events & EPOLLOUT)
        {
            list.pv[index].writable = 1;
        }
    } // end for
    return 0;
}
int CSocketEpoll::handle_new_connect(int listenfd)
{
    // 建立新连接
    struct sockaddr_in clientaddr;
    socklen_t clientAddrLen =  sizeof(struct sockaddr_in);
    int connfd;

    // while ((connfd = accept(listenfd, (sockaddr *) &clientaddr, &clientAddrLen )) > 0)
    // {
    connfd = accept(listenfd, (sockaddr *) &clientaddr, &clientAddrLen);
    if (connfd < 0)
    {
        fprintf(stderr, "Err: CSocketEpoll::handle_new_connect()\n");
        fprintf(stderr, "Err: accept() failed !!!\n");
        fprintf(stderr, "Err: errno = %d (%s) \n", errno, strerror(errno));
        return -1;
    }

    // 设为非阻塞
    if (setnonblocking(connfd) != 0)
    {
        fprintf(stderr, "Err: CSocketEpoll::handle_new_connect()\n");
        fprintf(stderr, "Err: setnonblocking() failed !!!\n");
        fprintf(stderr, "Err: errno = %d (%s) \n", errno, strerror(errno));
        return -1;
    }

    // 新连接的上下文
    int tmp;
    if (list.find_idle(&tmp) != 0)
    {
        fprintf(stderr, "Err: CSocketEpoll::handle_new_connect()\n");
        fprintf(stderr, "Err: list.find_idle() != 0 !!!\n");
        return -1;
    }
    list.pv[tmp].used = 1;
    list.pv[tmp].fd = connfd;

    // 超时处理
    memcpy(&(list.pv[tmp].tm), &now_tm, sizeof(struct timeval));
    // qs.insert(tmp); // insert index
    heap.insert_just(tmp);

    // 注册新的连接
    ev.data.u32 = tmp; // 找到空闲的socketinfo
    ev.events = EPOLLIN | EPOLLOUT | EPOLLET;

    printf("Info: new connect fd = %d, index = %d \n", connfd, tmp);

    if (epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev) < 0)
    {
        fprintf(stderr, "Err: CSocketEpoll::handle_new_connect()\n");
        fprintf(stderr, "Err: epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev) < 0 \n");
        fprintf(stderr, "Err: errno = %d (%s) \n", errno, strerror(errno));
        return -1;
    }
    return 0;

    // 不通知新连接，默认index就是session，但是close时需要通知
}

int CSocketEpoll::handle_recv(int index)
{
    CSocketInfo *psi = list.pv + index;
    int socketfd = psi->fd;
    while (1)
    {
        if (psi->recv_n < 4)  // 还未获取到msglen
        {
            ssize_t n = recv(socketfd, psi->recv_buf + psi->recv_n, 4 - psi->recv_n, 0); //
            if (n > 0)
            {
                psi->recv_n += n;
                // 超时处理
                memcpy(&(psi->tm), &now_tm, sizeof(struct timeval));
            }
            else if (n < 0)
            {
                if (errno == EWOULDBLOCK || errno == EAGAIN || errno == EINTR)  // 跳出，等待下次接收
                {
                    return 0;//
                }
                else if (errno == ECONNRESET)     // 对方关闭了socket
                {
                    fprintf(stdout, "Warn: recv() ECONNRESET \n ");
                    if (handle_close(index) != 0)
                    {
                        fprintf(stdout, "Err: handle_close() failed!!! \n ");
                        return -1;
                    }
                    return 0;
                }
                else
                {
                    fprintf(stderr, "Err: recv() n < 0 but donot know why \n ");
                    fprintf(stderr, "Err: errno = %d (%s) \n", errno, strerror(errno));
                    if (handle_close(index) != 0)
                    {
                        fprintf(stdout, "Err: handle_close() failed!!! \n ");
                        return -1;
                    }
                    return 0;
                }
            }
            else if (n == 0)    // 对端关闭
            {
                fprintf(stdout, "Warn: recv() n == 0 \n ");
                if (handle_close(index) != 0)
                {
                    fprintf(stdout, "Err: handle_close() failed!!! \n ");
                    return -1;
                }
                return 0;
            }
        }
        if (psi->recv_n >= 4)  // 已经获取到msglen
        {
            int msglen = *(int *)psi->recv_buf;
            if (cliIsBigEndian != srvIsBigEndian)
                encode_int(&msglen);
            ssize_t n = recv(socketfd, psi->recv_buf + psi->recv_n, msglen - psi->recv_n, 0); // 保证读取的是一个msg
            if (n > 0)
            {
                // 超时处理
                memcpy(&(psi->tm), &now_tm, sizeof(struct timeval));

                psi->recv_n += n;
                if (psi->recv_n == msglen)  // 收取了一个完整msg
                {
                    CSocketMsgHead head(msglen + sizeof(CSocketMsgHead), index, 0);
                    head.enpack(psi->recv_base);
                    if (outq->push(psi->recv_base) >= 0)
                    {
                        psi->recv_n = 0;
                    }
                    else
                    {
                        fprintf(stderr, "Err: outq is full !!!!\n");
                        return -1;
                    }
                }
            }
            else if (n < 0)
            {
                if (errno == EWOULDBLOCK || errno == EAGAIN || errno == EINTR)  // 跳出，等待下次接收
                {
                    return 0;//
                }
                else if (errno == ECONNRESET)     // 对方关闭了socket
                {
                    fprintf(stdout, "Warn: recv() ECONNRESET \n ");
                    if (handle_close(index) != 0)
                    {
                        fprintf(stdout, "Err: handle_close() failed!!! \n ");
                        return -1;
                    }
                    return 0;
                }
                else
                {
                    fprintf(stdout, "Warn: recv() n < 0 but donot know why \n ");
                    if (handle_close(index) != 0)
                    {
                        fprintf(stdout, "Err: handle_close() failed!!! \n ");
                        return -1;
                    }
                    return 0;
                }
            }
            else if (n == 0)    // 对端关闭
            {
                fprintf(stdout, "Warn: recv() n == 0 \n ");
                if (handle_close(index) != 0)
                {
                    fprintf(stdout, "Err: handle_close() failed!!! \n ");
                    return -1;
                }
                return 0;
            }
        }  // if (prmsg->n >= 4)
    } // while (1)
}
int CSocketEpoll::handle_close(int index)
{
    CSocketInfo *psi = list.pv + index;
    int socketfd = psi->fd;

    ev.data.u32 = index; // 找到空闲的socketinfo
    ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
    if (epoll_ctl(epfd, EPOLL_CTL_DEL, socketfd, &ev) < 0)
    {
        fprintf(stderr, "Err: CSocketEpoll::handle_close()\n");
        fprintf(stderr, "Err: epoll_ctl(epfd, EPOLL_CTL_DEL, socketfd, &ev) < 0 \n");
        fprintf(stderr, "Err: errno = %d (%s) \n", errno, strerror(errno));
        return -1;
    }

    close(socketfd);
    list.pv[index].erase();

    // 超时处理
    // qs.del(index);//统一根据used来处理，由于超时还调用这里关闭，所以不处理超时这里

    // TODO
    // 通知后台index或session已经失效
    if (handle_info_close(index) != 0)
    {
        fprintf(stderr, "Err: handle_info_close() != 0 !!!!\n");
        return -1;
    }
    return 0;
}
int CSocketEpoll::handle_info_close(int index)
{
    char buf[sizeof(CSocketMsgHead)]; // 只有CSocketMsgHead表示关闭
    CSocketMsgHead head(sizeof(CSocketMsgHead), index, 0);
    head.enpack(buf);
    // TODO 压入共享内存区
    if (outq->push(buf) < 0)
    {
        fprintf(stderr, "Err: outq is full !!!!\n");
        return -1;
    }
    return 0;
}
//------------------------------------------
int CSocketEpoll::handle_send()
{
    if (flag_q1 == 1)
    {
        flag_q1 = 0;
        handle_send_inq_tmpq(inq, &q1);
        handle_send_inq_tmpq(&q2, &q1);
    }
    else
    {
        flag_q1 = 1;
        handle_send_inq_tmpq(inq, &q2);
        handle_send_inq_tmpq(&q1, &q2);
    }
    return 0;
}

int CSocketEpoll::handle_send_inq_tmpq(CShmQueueSingle *inQ, CMemQueueSingle *tmpQ) //
{
    char send_base[1024];
    char *send_buf = send_base + sizeof(CSocketMsgHead);
    CSocketMsgHead head;
    while (inQ->pop(send_base) > 0)
    {
        head.unpack(send_base);
        int len = head.len;
        int index = head.index;
        int sendPos = head.sendPos;

        if (list.pv[index].used != 1)
        {
            fprintf(stdout, "Err: CSocketEpoll::handle_send_inq()\n");
            fprintf(stdout, "Err: index[%d] is not used \n", index);
            continue;
        }
        int socketfd = list.pv[index].fd;
        int sendLen = len - sizeof(CSocketMsgHead) - sendPos;
        if (sendPos != 0)
            printf("1111");
        int n = send(socketfd, send_buf + sendPos, sendLen, 0);
        if (n < 0)
        {
            if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)   // 没有发送完
            {
                printf("55ds55");
                if (tmpQ->push(send_base) < 0)
                {
                    fprintf(stderr, "Err: tmpQ->push() is full !!!\n" );
                    return -1;
                }
                continue;//
            }
            else     // 出错
            {
                fprintf(stderr, "Err: CSocketEpoll::handle_send_inq() \n");
                fprintf(stderr, "Err: errno = %d (%s) \n ", errno, strerror(errno));
                handle_close(index);
                continue;
            }
        }
        else if (n == 0)    // 一般不返回0
        {
            fprintf(stderr, "Err: CSocketEpoll::handle_send_inq() \n");
            fprintf(stderr, "Err: send() n == 0 \n");
            fprintf(stderr, "Err: errno = %d (%s) \n ", errno, strerror(errno));
            handle_close(index);
            continue;
        }
        else // n > 0
        {
            // 超时处理
            memcpy(&(list.pv[index].tm), &now_tm, sizeof(struct timeval));

            if (n < sendLen )
            {
                CSocketMsgHead head2(len, index, sendPos + n);
                head2.enpack(send_base);
                printf("5wef555");
                if (tmpQ->push(send_base) < 0)
                {
                    fprintf(stderr, "Err: q.push() is full !!!\n" );
                    return -1;
                }
                fprintf(stderr, "Test: inq send part of msg !!!\n" );
                continue;
            }
        }
    }// end while()
    return 0;
}

int CSocketEpoll::handle_send_inq_tmpq(CMemQueueSingle *inQ, CMemQueueSingle *tmpQ) //
{
    char send_base[1024];
    char *send_buf = send_base + sizeof(CSocketMsgHead);
    CSocketMsgHead head;
    while (inQ->pop(send_base) > 0)
    {
        head.unpack(send_base);
        int len = head.len;
        int index = head.index;
        int sendPos = head.sendPos;

        if (list.pv[index].used != 1)
        {
            fprintf(stdout, "Err: CSocketEpoll::handle_send_inq()\n");
            fprintf(stdout, "Err: index[%d] is not used \n", index);
            continue;
        }
        int socketfd = list.pv[index].fd;
        int sendLen = len - sizeof(CSocketMsgHead) - sendPos;
        if (sendPos != 0)
            printf("22222");
        int n = send(socketfd, send_buf + sendPos, sendLen, 0);
        if (n < 0)
        {
            if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)   // 没有发送完
            {
                printf("44444");
                if (tmpQ->push(send_base) < 0)
                {
                    fprintf(stderr, "Err: tmpQ->push() is full !!!\n" );
                    return -1;
                }
                continue;//
            }
            else     // 出错
            {
                fprintf(stderr, "Err: CSocketEpoll::handle_send_inq() \n");
                fprintf(stderr, "Err: errno = %d (%s) \n ", errno, strerror(errno));
                handle_close(index);
                continue;
            }
        }
        else if (n == 0)    // 一般不返回0
        {
            fprintf(stderr, "Err: CSocketEpoll::handle_send_inq() \n");
            fprintf(stderr, "Err: send() n == 0 \n");
            fprintf(stderr, "Err: errno = %d (%s) \n ", errno, strerror(errno));
            handle_close(index);
            continue;
        }
        else  // n > 0
        {
            // 超时处理
            memcpy(&(list.pv[index].tm), &now_tm, sizeof(struct timeval));

            if (n < sendLen)
            {
                CSocketMsgHead head2(len, index, sendPos + n);
                head2.enpack(send_base);
                printf("5555");
                if (tmpQ->push(send_base) < 0)
                {
                    fprintf(stderr, "Err: q.push() is full !!!\n" );
                    return -1;
                }
                fprintf(stderr, "Test: inq send part of msg !!!\n" );
                continue;
            }
        }
    }// end while()
    return 0;
}

//qs版本
// int CSocketEpoll::handle_timeout()
// {
//     static int cnt = 200;
//     while (cnt-- > 0)
//         return 0;
//     cnt = 200;

//     int index;
//     CSocketInfo *psi;
//     // 遍历当前qs中的fd，如果是已经关闭的则删除，不是则计算tm_sec
//     for (int i=0; i<qs.n; i++)
//     {
//         index = qs.a[i];
//         psi = &(list.pv[index]);
//         if (psi->used == 0) // 已经关闭
//         {
//             qs.del(i);
//             i--;
//         }
//         else
//         {
//             psi->tm_sec = now_tm.tv_sec - psi->tm.tv_sec;
//         }
//     }

//     // 根据tm_sec对qs排序，得到递增数组
//     qs.sort();


//     // 从数组尾部关闭那些超时的fd
//     int i = qs.n - 1;
//     while (i >= 0)
//     {
//         index = qs.a[i];
//         psi = &(list.pv[index]);
//         if (qs.get_tm_sec(index) > 10)  // 10s
//         {
//             fprintf(stderr, "Warn: fd = %d is timeout(10s) !!!\n", psi->fd);
//             handle_close(index);
//             qs.del(i);
//         }
//         else
//         {
//             break;
//         }
//         i--;
//     }
    

//     return 0;
// }

//heap版本
int CSocketEpoll::handle_timeout()
{
    static int cnt = 2000;
    while (cnt-- > 0)
        return 0;
    cnt = 2000;

    int index;
    CSocketInfo *psi;

    for (int i = 0; i < heap.n; i++)
    {
        index = heap.array[i];
        psi = &(list.pv[index]);
        if (psi->used == 0) // 已经关闭
        {
            heap.del_just(i);
            i--;
        }
        else
        {
            psi->tm_sec = now_tm.tv_sec - psi->tm.tv_sec;
            printf("psi->tm_sec = %d\n", psi->tm_sec);
        }
    }
    // 根据tm_sec对qs排序，得到递增数组
    heap.build();

    // 从数组尾部关闭那些超时的fd
    index = heap.array[0];
    printf("heap.n = %d \n", heap.n);
    printf("heap.array[0] = %d \n", index);
    printf("heap.get_timeout_cnt(index) = %d \n", heap.get_timeout_cnt(index));
    while (heap.get_timeout_cnt(index) > 10)
    {
        psi = &(list.pv[index]);
        fprintf(stderr, "Warn: fd = %d is timeout(10s) !!!\n", psi->fd);
        handle_close(index);
        heap.swap(0, heap.n - 1);
        heap.fixdown(heap.array, 0, heap.n - 1);
        heap.del_just(heap.n - 1);
        index = heap.array[0];
    }

    return 0;
}