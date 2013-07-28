#include "CGMEngine.h"




CGMEngine::CGMEngine(int cliIsBigEndian, int srvIsBigEndian)
{
    this->cliIsBigEndian = cliIsBigEndian;
    this->srvIsBigEndian = srvIsBigEndian;
}
CGMEngine::~CGMEngine()
{

}

int CGMEngine::prepare(CShmQueueSingle * inQ, CShmQueueSingle * outQ, CShmQueueSingle * db_inQ, CShmQueueSingle * db_outQ)
{
	inq = inQ;
	outq = outQ;
	db_inq = db_inQ;
	db_outq = db_outQ;
	return 0;
}

int CGMEngine::run()
{
	char in[1024];
	char out[1024];
    CSocketMsgHead head;
	while (1)
	{
		while (inq->pop(in) >= 0)
		{
            if (head.is_close_msg(in))
            {
                fprintf(stderr, "Warn: CGMEngine::run()\n");
                fprintf(stderr, "Warn: is_close_msg()\n");
                continue;
            }
			handle_client(in, out);
			route(out, outq, db_outq);
		}
		while (db_inq->pop(in) >= 0)
		{
			handle_db(in, out);
			route(out, outq, db_outq);
		}
		usleep(10);
	}
	return 0;
}

int CGMEngine::handle_client(char * in, char * out)
{
    CMsgHead *phead = (CMsgHead *)(in + sizeof(CSocketMsgHead)); // 1次封包
    short msgid = phead->msgid;
    if (cliIsBigEndian != srvIsBigEndian)
    {
        ::encode_short(&msgid);
    }
    switch (msgid)
    {
    case MSGID_REQUESTLOGIN:
        return handle_RequestLogin(in, out);
        break;

    default:
        fprintf(stderr, "Err: CGMEngine handle_client wrong msgid = %d !!\n", phead->msgid);
        return -1;
        break;
    }
    return 0;
}


int CGMEngine::handle_db(char * in, char * out)
{
    CMsgHead *phead = (CMsgHead *)(in + sizeof(CSocketMsgHead)); // 1次封包

    switch (phead->msgid)
    {
    case MSGID_REQUESTUSERINFO:
        return handle_RequestUserInfo(in, out);
        break;

    default:
        fprintf(stderr, "Err:CGMEngine handle_db wrong msgid = %d !!\n", phead->msgid);
        return -1;
        break;
    }
    return 0;
}

int CGMEngine::route(char * out, CShmQueueSingle * outq, CShmQueueSingle * db_outq)
{
	CMsgHead *phead = (CMsgHead *)(out + sizeof(CSocketMsgHead));// 1次封包
	switch (phead->dstfe)
	{
		case FE_CLIENT:
			if (outq->push(out) < 0)
			{
				fprintf(stderr, "Err:CGMEngine route() outq.push(out) < 0 !!\n");
				return -1;
			}
		break;
		
		case FE_DBSVRD:
			if (db_outq->push(out) < 0)
			{
				fprintf(stderr, "Err:CGMEngine route() outq.push(out) < 0 !!\n");
				return -1;
			}
		break;
		
		default:
			fprintf(stderr, "Err:CGMEngine route() wrong dstfd = %d !!\n", phead->dstfe);
			return -1;
		break;
	}
	return 0;
}


int CGMEngine::handle_RequestLogin(char * in, char * out)
{
    CSocketMsgHead head;
    head.unpack(in);
	
    int len;
    handle_RequestLogin_origin(in, out, &len, head.index);
 	
    head.len = len + sizeof(CSocketMsgHead);
    head.sendPos = 0;
    head.enpack(out);

    return 0;
}
int CGMEngine::handle_RequestLogin_origin(char * in, char * out, int *len, int index)
{
    CMsgHead *pinhead = (CMsgHead *)(in + sizeof(CSocketMsgHead));// 1次封包
    CMsgHead *pouthead = (CMsgHead *)(out + sizeof(CSocketMsgHead));// 1次封包

    CMsgRequestLoginPara *pinpara =
        (CMsgRequestLoginPara *)((char *)pinhead + sizeof(CMsgHead));
    CRequestUserInfoPara *poutpara =
        (CRequestUserInfoPara *)((char *)pouthead + sizeof(CMsgHead));

    if (cliIsBigEndian != srvIsBigEndian) 
    {
        pinhead->encode();
        pinpara->encode();
    }

    strcpy(poutpara->m_szUserName, pinpara->username);

    *len = pouthead->msglen = sizeof(CMsgHead) + sizeof(CRequestUserInfoPara);
    pouthead->msgid = MSGID_REQUESTUSERINFO; //16位无符号整型，消息ID
    pouthead->msgtype = Request;   //16位无符号整型，消息类型，当前主要有Request、Response以及Notify三种类型
    pouthead->msgseq = 1234567890;     //32位无符号整型，消息序列号
    pouthead->srcfe = FE_GAMESVRD ;       //8位无符号整型，消息发送者类型，当前主要有FE_CLIENT、FE_GAMESVRD以及FE_DBSVRD三种
    pouthead->dstfe = FE_DBSVRD;     //8位无符号整型，消息接收者类型 同上
    pouthead->srcid = index;   //16位无符号整型，当客户端向游戏服务器发送消息时ScrID为SessionID
    pouthead->dstid = 0;   //16位无符号整型，当游戏服务器向客户端发送消息是DstID为SessionID

    return 0;
}


int CGMEngine::handle_RequestUserInfo(char * in, char * out)
{
    CSocketMsgHead head;
    head.unpack(in);

    int len;
    handle_RequestUserInfo_origin(in, out, &len, head.index);
 
	head.len = len + sizeof(CSocketMsgHead);
    head.sendPos = 0;
    head.enpack(out);

    return 0;
}
int CGMEngine::handle_RequestUserInfo_origin(char * in, char * out, int *len, int index)
{
    CMsgHead *pinhead = (CMsgHead *)(in + sizeof(CSocketMsgHead));// 1次封包
    CMsgHead *pouthead = (CMsgHead *)(out + sizeof(CSocketMsgHead));// 1次封包

    CResponseUserInfoPara *pinpara =
        (CResponseUserInfoPara *)((char *)pinhead + sizeof(CMsgHead));
    CMsgResponseLoginPara *poutpara =
        (CMsgResponseLoginPara *)((char *)pouthead + sizeof(CMsgHead));

    poutpara->m_unUin = pinpara->m_stPlayerInfo.m_unUin;
    poutpara->m_unSessionID = index;
    poutpara->m_bResultID = 0;
    memcpy(&poutpara->m_stPlayerInfo, &pinpara->m_stPlayerInfo, sizeof(m_stPlayerInfo));

    *len = pouthead->msglen = sizeof(CMsgHead) + sizeof(CMsgResponseLoginPara);
    pouthead->msgid = MSGID_REQUESTLOGIN; //16位无符号整型，消息ID
    pouthead->msgtype = Response;   //16位无符号整型，消息类型，当前主要有Requst、Response以及Notify三种类型
    pouthead->msgseq = 1234567890;     //32位无符号整型，消息序列号
    pouthead->srcfe = FE_GAMESVRD ;       //8位无符号整型，消息发送者类型，当前主要有FE_CLIENT、FE_GAMESVRD以及FE_DBSVRD三种
    pouthead->dstfe = FE_CLIENT;     //8位无符号整型，消息接收者类型 同上
    pouthead->srcid = index;   //16位无符号整型，当客户端向游戏服务器发送消息时ScrID为SessionID
    pouthead->dstid = 0;   //16位无符号整型，当游戏服务器向客户端发送消息是DstID为SessionID

    if (cliIsBigEndian != srvIsBigEndian)
    {
        pouthead->encode();
        poutpara->encode();
    }

    return 0;
}








