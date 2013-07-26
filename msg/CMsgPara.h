#ifndef CMSGPARA_H
#define CMSGPARA_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>



#define Request             0
#define Response            1
#define Notify              2
#define Ack                 3
#define Internal            4

#define FE_CLIENT           0
#define FE_DBSVRD           1
#define FE_GAMESVRD         2
#define FE_NULL             3


#define MSGID_I2M_NEW_CONNECT   0xF001
//不带参数，srcid为index值，dstid为srvfd
#define MSGID_I2M_CLO_CONNECT   0xF002
//不带参数，srcid为index值，dstid为srvfd



#pragma pack(push) //保存对齐状态 
#pragma pack(1)//设定为1字节对齐 


typedef struct m_stPlayerInfo
{
    char        m_szUserName[64];   // 用户昵称
    uint32_t    m_unUin;            // 用户ID
    uint8_t     m_bySex;            // 性别 男 - 0 女 - 1
    uint16_t    m_unLevel;          // 级别
    uint16_t    m_unWin;            // 胜利局数
    uint16_t    m_unLose;           // 失败局数
    uint16_t    m_unRun;            // 逃跑局数
} PlayerInfo;

//--------------------------------------------------
/*class CMsgPara
{
public:
    // CMsgPara();
    virtual ~CMsgPara(){}
    virtual int buf2para(char *buf) = 0;
    virtual int para2buf(char *buf) = 0;
    virtual void print() = 0;
};*/

#define MSGID_REQUESTLOGIN     		0x0001

class CMsgRequestLoginPara//: public CMsgPara
{
public:
    char username[64];
    char password[16];

    ~CMsgRequestLoginPara();
    int buf2para(char *buf) ;
    int para2buf(char *buf) ;
    void print() ;
};

class CMsgResponseLoginPara//: public CMsgPara
{
public:
    uint32_t    m_unUin;            // 与用户名一一映射的用户ID
    uint32_t    m_unSessionID;      // 当前会话ID
    int8_t      m_bResultID;        // 0成功，1用户名或密码错误，-1系统错误（系统错误统一用小于0的标识码，应用层的错误用大于0的标识码）
    PlayerInfo  m_stPlayerInfo;     // 用户信息 登录成功时有效

    ~CMsgResponseLoginPara();
    int buf2para(char *buf) ;
    int para2buf(char *buf) ;
    void print() ;
};

#define MSGID_REQUESTUSERINFO     	0x1001

//请求用户信息 
class CRequestUserInfoPara
{
public:
	void print();
	
    char        m_szUserName[64];
};

//回复用户信息 
class CResponseUserInfoPara
{
public:
	void print();

	PlayerInfo  m_stPlayerInfo;
    char        m_szPwd[16];
    int8_t      m_bResultID;      // //0成功，否则 失败 
};


#pragma pack(pop)//恢复对齐状态

#endif
