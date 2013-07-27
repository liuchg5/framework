#ifndef CSOCKETMSGHEAD_H
#define CTSOCKETMSGHEAD_H

#pragma pack(push) //保存对齐状态 
#pragma pack(1)//设定为1字节对齐 
class CSocketMsgHead
{
public:
	int len;
	int index;
	int sendPos;

	CSocketMsgHead();
	CSocketMsgHead(int len, int index, int sendPos);
	~CSocketMsgHead();

	void enpack(char * base, int len, int index, int sendPos);
	void unpack(char * base, int *len, int *index, int *sendPos);
	void enpack(char * base);
	void unpack(char * base);

	int is_close_msg(char * base);
};
#pragma pack(pop)//恢复对齐状态




#endif
