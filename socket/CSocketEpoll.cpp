

#include "CSocketEpoll.h"

//------------------------------------------
CSocketInfo::CSocketInfo()
{
    used = 0; fd = -1; writable = 1;
    recv_n = 0; send_n = 0;
    recv_buf = new char[1024];
    send_buf = new char[1024];
    if (recv_buf == NULL || send_buf == NULL)
    {
        fprintf(stderr, "Err: CSocketInfo::CSocketInfo()\n");
        fprintf(stderr, "Err: new failed !!!\n");
    }
    memset(recv_buf, 0, 1024);
    memset(send_buf, 0, 1024);
}
CSocketInfo::~CSocketInfo()
{
    used = 0; fd = -1; writable = 1;
    recv_n = 0; send_n = 0;
    delete [] recv_buf;
    delete [] send_buf;
    recv_buf = NULL; send_buf = NULL;
}
void CSocketInfo::erase()
{
    used = 0; fd = -1; writable = 1;
    recv_n = 0; send_n = 0;
    memset(recv_buf, 0, 1024);
    memset(send_buf, 0, 1024);
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
//------------------------------------------
//------------------------------------------
//------------------------------------------
CSocketEpoll::CSocketEpoll(int epoll_Size, int epoll_Timeout, int Listenq, int flag_Inside):list(epoll_Size)
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
    flag_inside = flag_Inside;
}

CSocketEpoll::~CSocketEpoll()
{
    delete [] events;
}

int CSocketEpoll::prepare(const char * serv_addr, int port_num, CShmQueueSingle *poutq, CShmQueueSingle *pinq)
{
    port_number = port_num;
    outq = poutq;
	inq = pinq;

	q.crt(1024 * 1024 * 1);
    q.init();
    q.clear();

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

        // 处理recv
        handle_recv_and_set_writable();

        // 处理send
        handle_send();

        // 处理超时

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
    // set_MSGID_I2M_NEW_CONNECT(&tmpmsgbuf, index, connfd);
    //     if (pqs->pushmsg(&tmpmsgbuf) <= 0)
    //     {
    //         fprintf(stderr, "Err: CSocketSrvEpoll new connect msg cannot push, queue full !!!!!!\n ");
    //         return -1;
    //     }
    // } // end while ( accept() )
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
        }
        if (psi->recv_n >= 4)  // 已经获取到msglen
        {
            int msglen = *(int *)psi->recv_buf;
            ssize_t n = recv(socketfd, psi->recv_buf + psi->recv_n, msglen - psi->recv_n, 0); // 保证读取的是一个msg
            if (n > 0)
            {
                psi->recv_n += n;
                if (psi->recv_n == msglen)  // 收取了一个完整msg
                {
                    // TODO 压入共享内存区
                    // !!! need to convert !!!
                    // if (!flag_inside)    // 只有对外使用才enpack!!!!!!!!!!!!!!!!!!!!!!!!!!!!
					   enpack_index(psi->recv_buf, index);
                    // int *pi = (int *)(psi->recv_buf + psi->recv_n);
                    // *pi = index;
                    // pi = (int *)(psi->recv_buf);
                    // *pi = *pi + sizeof(int);
                    if (outq->push(psi->recv_buf) >= 0)
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
    char buf[2 * sizeof(int)];
    int *pi = (int *)buf;
    *pi++ = 2 * sizeof(int);
    *pi = index;
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
    handle_send_inq();
    handle_send_q();
    return 0;
}

int CSocketEpoll::handle_send_inq() //
{
    int index, socketfd;
    char tmp[1024];
    while (inq->pop(tmp) > 0)
    {
        int *pi = (int *)tmp; // msglen
        // if (!flag_inside)// !!!!!!!!!!!!!!!!!!!!!!
            unpack_index(tmp, &index); // !!!!!!!!!!!!!!!!!!!!!!
        // else
        //     index = 1;  // !!!!!!!!!!!!!!!!!!!!!!
        
        if (list.pv[index].used != 1)
        {
            fprintf(stdout, "Err: CSocketEpoll::handle_send_inq()\n");
            fprintf(stdout, "Err: index[%d] is not used \n", index);
            continue;
        }
        socketfd = list.pv[index].fd;
        int n = send(socketfd, tmp, *pi, 0);
        if (n < 0)
        {
            if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)   // 没有发送完
            {
                // if (!flag_inside)//!!!!!!!!!!!!!!!!!!!!!!!
                    enpack_index(tmp, index);

                if (q.push(tmp) < 0)
                {
                    fprintf(stderr, "Err: q.push() is full !!!\n" );
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
        else if (n < *pi) // 只发送了一部分
        {
            *pi = *pi - n;
            // if (!flag_inside)//!!!!!!!!!!!!!!!!
                enpack_index(tmp, index);
            
            // *(int *)(tmp + *pi) = index;
            // *pi = *pi + sizeof(int);
            if (q.push(tmp) < 0)
            {
                fprintf(stderr, "Err: q.push() is full !!!\n" );
                return -1;
            }
			fprintf(stderr, "Test: inq send part of msg !!!\n" );
            continue;
        }
    }// end while()
    return 0;
}

int CSocketEpoll::handle_send_q() //
{
    

    int index, socketfd;
    char tmp[1024];
    int cnt = 10;
    while (q.pop(tmp) > 0 && cnt-- > 0)
    {
        printf("CSocketEpoll::handle_send_q()\n");

        int *pi = (int *)tmp; // msglen
        // if (!flag_inside)
            unpack_index(tmp, &index); // !!!!!!!!!!!!!!!!!!!!!!
        // else
        //     index = 1;//!!!!!!!!!!!!!!!!!
        
        if (list.pv[index].used != 1)
        {
            fprintf(stdout, "Err: CSocketEpoll::handle_send_inq()\n");
            fprintf(stdout, "Err: index is not used \n");
            continue;
        }
        socketfd = list.pv[index].fd;
        int n = send(socketfd, tmp, *pi, 0);
        if (n < 0)
        {
            if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)   // 没有发送完
            {
                // if (!flag_inside)
                    enpack_index(tmp, index);

                if (q.push(tmp) < 0)
                {
                    fprintf(stderr, "Err: q.push() is full !!!\n" );
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
        else if (n < *pi) // 只发送了一部分
        {
            *pi = *pi - n;
            // if (!flag_inside)
                enpack_index(tmp, index);
            // *(int *)(tmp + *pi) = index;
            // *pi = *pi + sizeof(int);
            if (q.push(tmp) < 0)
            {
                fprintf(stderr, "Err: q.push() is full !!!\n" );
                return -1;
            }
            fprintf(stderr, "Test: inq send part of msg !!!\n" );
            continue;
        }
    }// end while()
    return 0;
}
