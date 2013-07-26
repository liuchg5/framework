#ifndef COMMON_H
#define COMMON_H


#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

#include "../msg/CMsgHead.h"
#include "../msg/CMsgPara.h"

// 设为非阻塞socket函数
int setnonblocking(int sock) ;


// 验证是否是BE的函数
int isBigEndian() ;

//set操作要在connect()或者listen()之前
int get_snd_size(int socketfd);
int get_rcv_size(int socketfd);
int set_snd_size(int socketfd, int size);
int set_rcv_size(int socketfd, int size);

// 信号处理函数，考虑可重入
void sig_SIGUSR1_op(int signum, siginfo_t *info, void *myact);
// 封装了安装信号函数
int my_sigaction(int sig,  void (*sig_op)(int, siginfo_t*, void*));

// 打开阻塞模型的socket
int open_socket_cli(const char *serv_addr, int port_number);

//
unsigned int GetRandomInteger(int low, int up) ;

// 打开测试用
int test_socket_srv(const char *serv_addr, int port_number);

// 封包  解包
void enpack_index(char * buf, int index);
void unpack_index(char * buf, int * pindex);
void get_index(char * buf, int * pindex);
void set_index(char * buf, int index);
#endif

