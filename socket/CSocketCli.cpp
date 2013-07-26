#include "CSocketCli.h"


CSocketCli::CSocketCli()
{
	fd = -1;
	send_flag = 0;
	recv_n = send_n = 0;
	recv_buf = new char[1024];
	send_buf = new char[1024];
	if (recv_buf == NULL || send_buf == NULL)
	{
		fprintf(stderr, "Err: CSocketCli::CSocketCli()\n");
		fprintf(stderr, "Err: new failed \n");
	}
    memset(recv_buf, 0, 1024);
    memset(send_buf, 0, 1024);
}

CSocketCli::~CSocketCli()
{
	delete [] recv_buf;
	delete [] send_buf;
	recv_buf = send_buf = NULL;
}

int CSocketCli::prepare(const char *servAddr, int portNumber, CShmQueueSingle * inQ, CShmQueueSingle *outQ)
{
	inq = inQ;
	outq = outQ;
	
    int port_number = portNumber;
    char srv_addr[30];
    strcpy(srv_addr, servAddr);

    struct sockaddr_in serveraddr;

    serveraddr.sin_family = AF_INET; //设置为IP通信
    serveraddr.sin_addr.s_addr = inet_addr(srv_addr); //服务器IP地址
    serveraddr.sin_port = htons(port_number); //服务器端口号

    /*创建客户端套接字--IPv4协议，面向连接通信，TCP协议*/
    if ((fd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        fprintf(stderr, "Err: CSocketCli socket(PF_INET, SOCK_STREAM, 0) failed! \n ");
        fprintf(stderr, "Err: errno = %d (%s) \n ", errno, strerror(errno));
        return -1;
    }

    // 设为非阻塞
    if (setnonblocking(fd) != 0)
    {
        fprintf(stderr, "Err: CSocketCli setnonblocking() failed!\n ");
        fprintf(stderr, "Err: errno = %d (%s) \n ", errno, strerror(errno));
    }

    /*将套接字绑定到服务器的网络地址上*/
    if (connect(fd, (struct sockaddr *) &serveraddr, sizeof(struct sockaddr)) < 0)
    {
        if (errno != EINPROGRESS)
        {
            fprintf(stderr, "Err: CSocketCli connect() failed! \n ");
            fprintf(stderr, "Err: errno = %d (%s) \n ", errno, strerror(errno));
            return -1;
        }
        else
        {
            fprintf(stderr, "Warn: CSocketCli connect() is in progress... \n");
        }
    }
    else
    {
        fprintf(stdout, "Info: CSocketCli connect() finished! \n");
    }
    return 0;
}


int CSocketCli::run()
{
	while (1)
	{
		if (handle_recv() != 0)
            return -1;
	
		if (handle_send() != 0)
            return -1;
		
		usleep(10);
	}
	return 0;
}

int CSocketCli::handle_recv()
{
	int n;
	while(1)
	{
		if (recv_n < 4)
        {
            n = recv(fd, recv_buf + recv_n, 4 - recv_n, 0);
            if (n > 0)
            {
                recv_n += n;
            }
            else if (n < 0)
            {
                if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)
                {
                    return 0;
                }
                else if (errno == ECONNRESET)
                {
                    fprintf(stderr, "Err: CSocketCli ECONNRESET \n ");
                    handle_close();
                    return -1;
                }
                else
                {
                    fprintf(stderr, "Err: CSocketCli recv n < 0 but donot why \n ");
                    fprintf(stderr, "Err: CSocketCli errno = %d (%s) \n ", errno, strerror(errno));
                    handle_close();
                    return -1;
                }
            }
            else if (n == 0)
            {
                fprintf(stderr, "Err: CSocketCli recv n == 0 \n ");
                handle_close();
                return -1;
            }
        }
        if (recv_n >= 4) // recvmsg.n >= 4
        {
            int msglen = *(int *)recv_buf;
            n = recv(fd, recv_buf + recv_n, msglen - recv_n, 0);
            if (n > 0)
            {
                recv_n += n;
                if (msglen == recv_n) // 读取完了msg
                {
                    if (outq->push(recv_buf) < 0)
                    {
                        fprintf(stderr, "Err: outq is full !!!\n");
                        return -1;
                    }
                    recv_n = 0;
                    sta.check("CSocketCli");
                }
            }
            else if (n < 0)
            {
                if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)
                {
                    return 0;
                }
                else if (errno == ECONNRESET)
                {
                    fprintf(stderr, "Err: CSocketCli ECONNRESET \n ");
                    handle_close();
                    return -1;
                }
                else
                {
                    fprintf(stderr, "Err: CSocketCli recv n < 0 but donot why \n ");
                    fprintf(stderr, "Err: CSocketCli errno = %d (%s) \n ", errno, strerror(errno));
                    handle_close();
                    return -1;
                }
            }
            else if (n == 0)
            {
                fprintf(stderr, "Err: CSocketCli recv n == 0 \n ");
                handle_close();
                return -1;
            }
        }
	}// end while(1)
}


int CSocketCli::handle_send()
{
    int n;
	// 处理发送
    if (send_flag == 0)
    {
		if (inq->pop(send_buf) > 0)
		{
			send_flag = 1;
			send_n = 0;
		}
    }
    while (send_flag == 1)
    {
        int msglen = *(int *)send_buf;
        // printf("send msglen = %d\n", msglen); // debug
        n = send(fd, send_buf + send_n, msglen - send_n, 0);
        if (n < 0)
        {
            if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)
            {
                return 0;
            }
            else if (errno == ECONNRESET)
            {
                fprintf(stderr, "Err: CSocketCli ECONNRESET \n ");
                handle_close();
                return -1;
            }
            else
            {
                fprintf(stderr, "Err: CSocketCli recv n < 0 but donot why \n ");
                fprintf(stderr, "Err: CSocketCli errno = %d (%s) \n ", errno, strerror(errno));
                handle_close();
                return -1;
            }
        }
        else if (n == 0)
        {
            fprintf(stderr, "Err: CSocketCli send() n == 0 \n");
			handle_close();
            return -1;
        }
        else if (n > 0)
        {
            send_n += n;
            if (msglen == send_n)  // 发送完了，取新msg
            {
                if (inq->pop(send_buf) > 0)
				{
					send_flag = 1;
					send_n = 0;
				}
				else
				{
					send_flag = 0;
					send_n = 0;
				}
            }
        }
    } // while (send_flag == 1)
	return 0;
}



int CSocketCli::handle_close()
{
	fprintf(stderr, "Err: CSocketCli::handle_close() \n");
	fprintf(stderr, "Err: it mean something happened! \n");
    close(fd);
    //TODO
    fd = -1;
    memset(recv_buf, 0, 1024);
    memset(send_buf, 0, 1024);
    recv_n = 0;
    send_n = 0;
	send_flag = 0;
    return 0;
}



int CSocketCli::is_connected()
{
    fd_set rset, wset;
    struct timeval waitd;
    int ret;

    waitd.tv_sec = 1;     // Make select wait up to 1 second for data
    waitd.tv_usec = 0;    // and 0 milliseconds.
    FD_ZERO(&rset); // Zero the flags ready for using
    FD_ZERO(&wset);
    FD_SET(fd, &rset);
    FD_SET(fd, &wset);

    ret = select(fd + 1, &rset, &wset, NULL, &waitd);
    if (ret == 0) //timeout
    {
        return 0;
    }

    if (FD_ISSET(fd, &rset) || FD_ISSET(fd, &wset))
    {
        //Socket read or write available
        // int len = sizeof(error);
        // if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len) < 0)
        // {
        // return -1;
        // }
    }
    else
    {
        return 0;
    }
    return 1;
}



/* 
int CSocketCli::recv_and_send_debug(CShmQueueSingle *precvQ, CShmQueueSingle *psendQ)
{
    //一般要通过select来判断，但这里简化处理，假设成功
    //一次处理完


    while (psendQ->popmsg(&sendmsg) > 0)
    {

    CMsgHead *phead = (CMsgHead *)sendmsg.buf;
    uint16_t srcid = phead->srcid;
    uint16_t dstid = phead->dstid;
    // printf("recv_and_send_debug sendmsg srcid = %d\n", srcid);
    // printf("recv_and_send_debug sendmsg dstid = %d\n", dstid);

    CResponseUserInfoPara *poutpara =
        (CResponseUserInfoPara *)(recvmsg.buf + sizeof(CMsgHead));

    // pinpara->print(); // debug
	

    strcpy(poutpara->m_stPlayerInfo.m_szUserName, "TestName");
    poutpara->m_stPlayerInfo.m_unUin = 1;
    poutpara->m_stPlayerInfo.m_bySex = 0;
    poutpara->m_stPlayerInfo.m_unLevel = 99;
    poutpara->m_stPlayerInfo.m_unWin = 90;
    poutpara->m_stPlayerInfo.m_unLose = 8;
    poutpara->m_stPlayerInfo.m_unRun = 1;
    strcpy(poutpara->m_szPwd, "password");
    poutpara->m_bResultID = 3;


    phead = (CMsgHead *)recvmsg.buf;
    phead->msglen = sizeof(CMsgHead) + sizeof(CResponseUserInfoPara);
    // printf("phead->msglen = %d\n", phead->msglen);
    phead->msgid = MSGID_REQUESTUSERINFO; //16位无符号整型，消息ID
    phead->msgtype = Response;   //16位无符号整型，消息类型，当前主要有Requst、Response以及Notify三种类型
    phead->msgseq = 1234567890;     //32位无符号整型，消息序列号
    phead->srcfe = FE_DBSVRD ;       //8位无符号整型，消息发送者类型，当前主要有FE_CLIENT、FE_GAMESVRD以及FE_DBSVRD三种
    phead->dstfe = FE_GAMESVRD;     //8位无符号整型，消息接收者类型 同上
    phead->srcid = dstid;   //16位无符号整型，当客户端向游戏服务器发送消息时ScrID为SessionID
    phead->dstid = srcid;   //16位无符号整型，当游戏服务器向客户端发送消息是DstID为SessionID

    // printf("recv_and_send_debug recvmsg srcid = %d\n", phead->srcid);
    // printf("recv_and_send_debug recvmsg dstid = %d\n", phead->dstid);





        if (precvQ->pushmsg(&recvmsg) < 0)
        {
            fprintf(stderr, "Err: CSocketCli recvQ is full < 0 !!!!!!\n ");
        }

    }


    return 0;
} */