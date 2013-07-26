#ifndef CMSGHEAD_H
#define CMSGHEAD_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#pragma pack(push) //保存对齐状态 
#pragma pack(1)//设定为1字节对齐 

class CMsgHead // 消息头  msglen是必须解开的  sizeof=18
{
public:
    uint32_t msglen;
	uint16_t msgid;	//16位无符号整型，消息ID
	uint16_t msgtype;	//16位无符号整型，消息类型，当前主要有Requst、Response以及Notify三种类型
	uint32_t msgseq;		//32位无符号整型，消息序列号
	uint8_t srcfe;		//8位无符号整型，消息发送者类型，当前主要有FE_CLIENT、FE_GSVRD以及FE_DBSVRD三种
	uint8_t dstfe;		//8位无符号整型，消息接收者类型 同上
	uint16_t srcid;	//16位无符号整型，当客户端向游戏服务器发送消息时ScrID为SessionID
	uint16_t dstid;	//16位无符号整型，当游戏服务器向客户端发送消息是DstID为SessionID

	void print()
	{
		printf("msglen = %d\n", msglen);
		printf("msgid = %d\n", msgid);
		printf("msgtype = %d\n", msgtype);
		printf("msgseq = %d\n", msgseq);
		printf("srcfe = %d\n", srcfe);
		printf("dstfe = %d\n", dstfe);
		printf("srcid = %d\n", srcid);
		printf("dstid = %d\n", dstid);
	}
};



#pragma pack(pop)//恢复对齐状态

#endif
