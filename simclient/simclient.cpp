#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

#include <string.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>

#include "../common/common.h"
#include "../msg/StMsgBuffer.h"
#include "../msg/CMsgHead.h"
#include "../msg/CMsgPara.h"

#include "../common/CStatistics.h"

struct ST_thr_fn_arg
{
    long time;
    int up;
    char addr[30];
};

void *thr_fn(void *argv)
{
    struct timeval st_time;
    int fd;
    StMsgBuffer msgbuf;
    CMsgHead *phead = (CMsgHead *)msgbuf.buf;

    char buf[1024];

    // 打开阻塞模型的socket
    fd = open_socket_cli(((struct ST_thr_fn_arg *)argv)->addr, 10203);
    if (fd == -1)
    {
        fprintf(stderr, "Err: open_socket() failed! \n");
        return NULL;
    }

    phead->msglen = sizeof(CMsgHead) + sizeof(CMsgRequestLoginPara);
    int llll = phead->msglen;
    phead->msgid = MSGID_REQUESTLOGIN; //16位无符号整型，消息ID
    phead->msgtype = Request;   //16位无符号整型，消息类型，当前主要有Requst、Response以及Notify三种类型
    phead->msgseq = 1234567890;     //32位无符号整型，消息序列号
    phead->srcfe = FE_CLIENT;       //8位无符号整型，消息发送者类型，当前主要有FE_CLIENT、FE_GAMESVRD以及FE_DBSVRD三种
    phead->dstfe = FE_GAMESVRD;     //8位无符号整型，消息接收者类型 同上
    phead->srcid = 0;   //16位无符号整型，当客户端向游戏服务器发送消息时ScrID为SessionID
    phead->dstid = 0;   //16位无符号整型，当游戏服务器向客户端发送消息是DstID为SessionID

    char index[10];
    static int tmp = 0;

phead->encode();

    do
    {
        sprintf(index, "%07d", tmp++);
        if (tmp > (((struct ST_thr_fn_arg *)argv)->up - 1))
            tmp = 0;

        strcpy(msgbuf.buf + sizeof(CMsgHead), "liuchang");
        strcat(msgbuf.buf + sizeof(CMsgHead), index);
        strcpy(msgbuf.buf + sizeof(CMsgHead) + 64, "password");


        ssize_t n = send(fd, msgbuf.buf, llll, 0);  // 被阻塞
        if (n < 0)
        {
            fprintf(stderr, "Err: socket: %d send() failed! \n", fd);
            return NULL;
        }

        int rtn = 0;
        // do {
        // rtn += recv(fd, &tmp, 4, 0); //读取后丢弃
        // fprintf(stdout, "Info: recv msglen = %d \n", rtn);
        // } while (rtn < 106);
        rtn = recv(fd, buf, 104, 0); //读取后丢弃  // 被阻塞
        if (rtn != 104)
        {
            fprintf(stdout, "Warn: recv n = %d \n", rtn);
            fprintf(stderr, "Warn: fd = %d  \n ", fd);
            fprintf(stderr, "Warn: errno = %d (%s) \n ", errno, strerror(errno));
        }
        CMsgResponseLoginPara *poutpara =
            (CMsgResponseLoginPara *)(buf + sizeof(CMsgHead));
poutpara->encode();
        fprintf(stdout, "Info: get username = %s \n", poutpara->m_stPlayerInfo.m_szUserName);
        fprintf(stdout, "Info: get m_unWin = %d \n", poutpara->m_stPlayerInfo.m_unWin);
        //rtn = recv(fd, &tmp, 102, 0); //读取后丢弃


        st_time.tv_sec = 0;
        st_time.tv_usec = (((struct ST_thr_fn_arg *)argv)->time) * 1000;
        // fprintf(stdout, "Info: time = %d \n", st_time.tv_usec);
        select(0, NULL, NULL, NULL, &st_time);
        //usleep(st_time.tv_usec);
    }
    while (1);

    return NULL;
}

int main(int argc, char **argv)
{
    printf("Info: Begin simclient \n");

    // 安装信号， 为了gprof
    // 使用kill -s SIGUSR1 pid
    my_sigaction(SIGUSR1, sig_SIGUSR1_op);

    struct timeval st_main_time;
    int n_thread;

    struct ST_thr_fn_arg thr_fn_arg;

    if (argc == 5)
    {
        n_thread = atoi(argv[1]);
        strcpy(thr_fn_arg.addr, argv[2]);
        thr_fn_arg.time = atoi(argv[3]);
        thr_fn_arg.up = atoi(argv[4]);
    }
    else if (argc == 4)
    {
        n_thread = atoi(argv[1]);
        strcpy(thr_fn_arg.addr, argv[2]);
        thr_fn_arg.time = atoi(argv[3]);
        thr_fn_arg.up = 1000;
    }
    else if (argc == 3)
    {
        n_thread = atoi(argv[1]);
        strcpy(thr_fn_arg.addr, argv[2]);
        thr_fn_arg.time = 1000;
        thr_fn_arg.up = 1000;
    }
    else if (argc == 2)
    {
        n_thread = atoi(argv[1]);
        strcpy(thr_fn_arg.addr, "192.168.234.128");
        thr_fn_arg.time = 1000;
        thr_fn_arg.up = 1000;
    }
    else
    {
        n_thread = 10; //默认10个线程，每个1000ms
        strcpy(thr_fn_arg.addr, "192.168.234.128");
        thr_fn_arg.time = 1000;
        thr_fn_arg.up = 1000;
    }

    pthread_t *pthr = new pthread_t[n_thread];
    if (pthr == NULL)
    {
        fprintf(stderr, "Err: new pthread_t[] failed! \n");
        exit(-1);
    }

    for (int i = 0; i < n_thread; ++i)
    {
        st_main_time.tv_sec = 0;
        st_main_time.tv_usec = 30000; // 30ms启动一个线程，200个需要6000ms，即6s
        select(0, NULL, NULL, NULL, &st_main_time);
        if (0 != pthread_create(pthr + i, NULL, thr_fn, &thr_fn_arg))
        {
            fprintf(stderr, "Err: new pthread_t[] failed! \n");
            exit(-1);
        }
    }

    for (int i = 0; i < n_thread; ++i)
    {
        pthread_join(*(pthr + i), NULL);
    }

    delete pthr;

    return 0;
}
