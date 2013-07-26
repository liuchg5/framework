#include "CDBEngine.h"




CDBEngine::CDBEngine()
{

}
CDBEngine::~CDBEngine()
{

}
int CDBEngine::prepare(CShmQueueSingle * inQ, CShmQueueSingle * outQ)
{
	inq = inQ;
	outq = outQ;
	return 0;
}

int CDBEngine::run()
{
	char in[1024], out[1024];
	while (1)
	{
		while (inq->pop(in) >= 0)
		{
			handle(in, out);
			route(out, outq);
		}
		usleep(10);
	}
	return 0;
}

int CDBEngine::handle(char * in, char * out)
{
    CMsgHead *phead = (CMsgHead *)in;

    if (phead->msglen == 2*sizeof(int)) // 说明是close(fd)
    {
        fprintf(stdout, "Info: CDBEngine::handle()\n");
        fprintf(stdout, "Info: client close fd = %d\n", *(int *)(in + sizeof(int)));
        phead = (CMsgHead *)out;
        phead->dstfe = FE_NULL;
        return 0;
    }

	switch (phead->msgid)
    {
    case MSGID_REQUESTUSERINFO:
        return handle_RequestUserInfo(in, out);
        break;
		
    default:
		fprintf(stderr, "Err: CDBEngine::handle() wrong msgid !!\n");
		fprintf(stderr, "Err: CDBEngine msgid = %d !!\n", phead->msgid);
        phead->print();
        return -1;
        break;
    }
    return 0;
}
	
int CDBEngine::handle_RequestUserInfo(char * in, char * out)
{
	int index, old_index;
	unpack_index(in, &index);  // !!!!!!!!!!!!!!!!!这里要解开两层的包，来取得最初的index
    unpack_index(in, &old_index);
    // printf("in index = %d\n", index);//debug 
    

    CMsgHead *phead = (CMsgHead *)in;
    // phead->print(); // debug

    CRequestUserInfoPara *pinpara =
        (CRequestUserInfoPara *)(in + sizeof(CMsgHead)); //char        m_szUserName[64];
    CResponseUserInfoPara *poutpara =
        (CResponseUserInfoPara *)(out + sizeof(CMsgHead));

    strcpy(poutpara->m_stPlayerInfo.m_szUserName, pinpara->m_szUserName);
    poutpara->m_stPlayerInfo.m_unUin = 1;
    poutpara->m_stPlayerInfo.m_bySex = 0;
    poutpara->m_stPlayerInfo.m_unLevel = 99;
    poutpara->m_stPlayerInfo.m_unWin = 90;
    poutpara->m_stPlayerInfo.m_unLose = 8;
    poutpara->m_stPlayerInfo.m_unRun = 1;
    strcpy(poutpara->m_szPwd, "password");
    poutpara->m_bResultID = 3;


    phead = (CMsgHead *)out;
    phead->msglen = sizeof(CMsgHead) + sizeof(CResponseUserInfoPara);
    // printf("phead->msglen = %d\n", phead->msglen);
    phead->msgid = MSGID_REQUESTUSERINFO; //16位无符号整型，消息ID
    phead->msgtype = Response;   //16位无符号整型，消息类型，当前主要有Requst、Response以及Notify三种类型
    phead->msgseq = 1234567890;     //32位无符号整型，消息序列号
    phead->srcfe = FE_DBSVRD ;       //8位无符号整型，消息发送者类型，当前主要有FE_CLIENT、FE_GAMESVRD以及FE_DBSVRD三种
    phead->dstfe = FE_GAMESVRD;     //8位无符号整型，消息接收者类型 同上
    phead->srcid = index;   //16位无符号整型，当客户端向游戏服务器发送消息时ScrID为SessionID
    phead->dstid = index;   //16位无符号整型，当游戏服务器向客户端发送消息是DstID为SessionID

    // printf("sendmsg srcid = %d\n", phead->srcid);
	
    enpack_index(out, old_index);// !!!!!!!!!!!!!!!!!这里要打回两层的包，写入最初的index
	enpack_index(out, index);
    // printf("out index = %d\n", index); //debug
    // phead->print();
    
	return 0;
}


int CDBEngine::route(char * out, CShmQueueSingle * outq)
{
    CMsgHead *phead = (CMsgHead *)out;
    switch (phead->dstfe)
    {
        case FE_CLIENT:
                fprintf(stderr, "Err:CDBEngine route() FE_CLIENT !!\n");
                return -1;
        break;
        
        case FE_GAMESVRD:
            if (outq->push(out) < 0)
            {
                fprintf(stderr, "Err:CDBEngine route() outq.push(out) < 0 !!\n");
                return -1;
            }
        break;

        case FE_NULL:
        break;
        
        default:
            fprintf(stderr, "Err:CDBEngine route() wrong dstfd = %d !!\n", phead->dstfe);
            return -1;
        break;
    }
    return 0;
}












/* int CDBEngine::handle(StMsgBuffer *pmsg, CShmQueueMulti *pshmqm)
{
    CMsgHead *phead = (CMsgHead *)pmsg;
    uint16_t srcid = phead->srcid;
    uint16_t dstid = phead->dstid;
    switch (phead->msgid)
    {
    case MSGID_I2M_NEW_CONNECT:
        if (map.reg(srcid, dstid) < 0)
        {
            fprintf(stderr, "Err: map.reg() return neg val !!!\n");
        }
        break;

    case MSGID_I2M_CLO_CONNECT:
        if (map.del(srcid) < 0)
        {
            fprintf(stderr, "Err: map.del() return neg val !!\n");
        }
        break;

    case MSGID_I2M_LOGIN:
        return handle_MSGID_I2M_LOGIN(pmsg, pshmqm);
        break;

    default:
        return -1;
        break;
    }
    return 0;
}


int CDBEngine::handle_MSGID_I2M_LOGIN(StMsgBuffer *pmsg, CShmQueueMulti *pshmqm)
{
    StMsgBuffer tmpmsgbuf;
    tmpmsgbuf.n = 0;

    int index;

    CMsgHead *phead = (CMsgHead *)pmsg;
    // uint16_t srcid = phead->srcid;
    uint16_t dstid = phead->dstid;

    CMsgRequestLoginPara *pinpara =
        (CMsgRequestLoginPara *)(pmsg->buf + sizeof(CMsgHead));
    CMsgResponseLoginPara *poutpara =
        (CMsgResponseLoginPara *)(tmpmsgbuf.buf + sizeof(CMsgHead));
    // inpara.buf2para(pmsg->buf + sizeof(CMsgHead));
    // pinpara->print();

    poutpara->m_unUin = 1;
    poutpara->m_unSessionID = 2;
    poutpara->m_bResultID = 3;
    strcpy(poutpara->m_stPlayerInfo.m_szUserName, pinpara->username);
    // printf("pinpara->username %s\n", pinpara->username);
    poutpara->m_stPlayerInfo.m_unUin = 1;
    poutpara->m_stPlayerInfo.m_bySex = 0;
    poutpara->m_stPlayerInfo.m_unLevel = 99;
    poutpara->m_stPlayerInfo.m_unWin = 90;
    poutpara->m_stPlayerInfo.m_unLose = 8;
    poutpara->m_stPlayerInfo.m_unRun = 1;

    phead = (CMsgHead *)tmpmsgbuf.buf;
    phead->msglen = sizeof(CMsgHead) + sizeof(CMsgResponseLoginPara);
    // printf("phead->msglen = %d\n", phead->msglen);
    phead->msgid = MSGID_I2M_LOGIN; //16位无符号整型，消息ID
    phead->msgtype = Response;   //16位无符号整型，消息类型，当前主要有Requst、Response以及Notify三种类型
    phead->msgseq = 1234567890;     //32位无符号整型，消息序列号
    phead->srcfe = FE_GAMESVRD ;       //8位无符号整型，消息发送者类型，当前主要有FE_CLIENT、FE_GAMESVRD以及FE_DBSVRD三种
    phead->dstfe = FE_CLIENT;     //8位无符号整型，消息接收者类型 同上
    phead->srcid = dstid;   //16位无符号整型，当客户端向游戏服务器发送消息时ScrID为SessionID
    phead->dstid = 0;   //16位无符号整型，当游戏服务器向客户端发送消息是DstID为SessionID


    if (map.fnd(&index, dstid) < 0)
    {
        fprintf(stderr, "Err: map.fnd() return neg val !!\n" );
        return -1;
    }

    // printf("CDBEngine::    msglen = %d \n", phead->msglen);
    if (pshmqm->pushmsg(index, &tmpmsgbuf) < 0)
    {
        fprintf(stderr, "Err: pshmqm->pushmsg() return neg val !!!\n");
        return -1;
    }
    return 0;
} */