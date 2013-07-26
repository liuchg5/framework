#include "CGMEngine.h"




CGMEngine::CGMEngine()
{

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
	while (1)
	{
		while (inq->pop(in) >= 0)
		{
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
    CMsgHead *phead = (CMsgHead *)in;
	
	if (phead->msglen == 2*sizeof(int)) // 说明是close(fd)
	{
		fprintf(stdout, "Info: CGMEngine::handle_client()\n");
		fprintf(stdout, "Info: client close fd = %d\n", *(int *)(in + sizeof(int)));
        phead = (CMsgHead *)out;
        phead->dstfe = FE_NULL;
		return 0;
	}
	
    // uint16_t srcid = phead->srcid;
    // uint16_t dstid = phead->dstid;
    switch (phead->msgid)
    {
    // case MSGID_I2M_NEW_CONNECT:
        // if (map.reg(srcid, dstid) < 0)
        // {
            // fprintf(stderr, "Err: CGMEngine map.reg() return neg val !!!\n");
        // }
        // break;

    // case MSGID_I2M_CLO_CONNECT:
        // if (map.del(srcid) < 0)
        // {
            // fprintf(stderr, "Err: CGMEngine map.del() return neg val !!\n");
        // }
        // break;

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
    CMsgHead *phead = (CMsgHead *)in;

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
	CMsgHead *phead = (CMsgHead *)out;
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

        case FE_NULL:
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
	int index;
	unpack_index(in, &index);
	
    CMsgHead *phead = (CMsgHead *)in;

    CMsgRequestLoginPara *pinpara =
        (CMsgRequestLoginPara *)(in + sizeof(CMsgHead));
    CRequestUserInfoPara *poutpara =
        (CRequestUserInfoPara *)(out + sizeof(CMsgHead));

    strcpy(poutpara->m_szUserName, pinpara->username);

    phead = (CMsgHead *)out;
    phead->msglen = sizeof(CMsgHead) + sizeof(CMsgResponseLoginPara);
    phead->msgid = MSGID_REQUESTUSERINFO; //16位无符号整型，消息ID
    phead->msgtype = Request;   //16位无符号整型，消息类型，当前主要有Request、Response以及Notify三种类型
    phead->msgseq = 1234567890;     //32位无符号整型，消息序列号
    phead->srcfe = FE_GAMESVRD ;       //8位无符号整型，消息发送者类型，当前主要有FE_CLIENT、FE_GAMESVRD以及FE_DBSVRD三种
    phead->dstfe = FE_DBSVRD;     //8位无符号整型，消息接收者类型 同上
    phead->srcid = index;   //16位无符号整型，当客户端向游戏服务器发送消息时ScrID为SessionID
    phead->dstid = 0;   //16位无符号整型，当游戏服务器向客户端发送消息是DstID为SessionID
	
	enpack_index(out, index);
    // printf("this msg is going to send to dbsrv. index = %d\n", index);
    // phead->print();

    return 0;
}


int CGMEngine::handle_RequestUserInfo(char * in, char * out)
{
    // printf("CGMEngine::handle_RequestUserInfo()\n"); //debug
    // 
    CMsgHead *phead = (CMsgHead *)in;
    // phead->print();

    int index;
	unpack_index(in, &index);
    // printf("in index = %d \n", index); //debug

    phead = (CMsgHead *)in;
    // phead->print();

    CResponseUserInfoPara *pinpara =
        (CResponseUserInfoPara *)(in + sizeof(CMsgHead));
    CMsgResponseLoginPara *poutpara =
        (CMsgResponseLoginPara *)(out + sizeof(CMsgHead));

    poutpara->m_unUin = pinpara->m_stPlayerInfo.m_unUin;
    poutpara->m_unSessionID = index;
    poutpara->m_bResultID = 0;
    memcpy(&poutpara->m_stPlayerInfo, &pinpara->m_stPlayerInfo, sizeof(m_stPlayerInfo));

    phead = (CMsgHead *)out;
    phead->msglen = sizeof(CMsgHead) + sizeof(CMsgResponseLoginPara);
    // printf("phead->msglen = %d\n", phead->msglen);
    phead->msgid = MSGID_REQUESTLOGIN; //16位无符号整型，消息ID
    phead->msgtype = Response;   //16位无符号整型，消息类型，当前主要有Requst、Response以及Notify三种类型
    phead->msgseq = 1234567890;     //32位无符号整型，消息序列号
    phead->srcfe = FE_GAMESVRD ;       //8位无符号整型，消息发送者类型，当前主要有FE_CLIENT、FE_GAMESVRD以及FE_DBSVRD三种
    phead->dstfe = FE_CLIENT;     //8位无符号整型，消息接收者类型 同上
    phead->srcid = index;   //16位无符号整型，当客户端向游戏服务器发送消息时ScrID为SessionID
    phead->dstid = 0;   //16位无符号整型，当游戏服务器向客户端发送消息是DstID为SessionID
	
	enpack_index(out, index);

    // phead->print();//debug
    // printf("out index = %d \n", index); //debug

    return 0;
}















/* int CGMEngine::handle_debug(StMsgBuffer *pmsg, CShmQueueMulti *pshmqm)
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

    case MSGID_REQUESTLOGIN:
        return handle_MSGID_I2M_LOGIN(pmsg, pshmqm);
        break;

    default:
        return -1;
        break;
    }
    return 0;
}


int CGMEngine::handle_MSGID_I2M_LOGIN(StMsgBuffer *pmsg, CShmQueueMulti *pshmqm)
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
    phead->msgid = MSGID_REQUESTLOGIN; //16位无符号整型，消息ID
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

    // printf("CGMEngine::    msglen = %d \n", phead->msglen);
    if (pshmqm->pushmsg(index, &tmpmsgbuf) < 0)
    {
        fprintf(stderr, "Err: pshmqm->pushmsg() return neg val !!!\n");
        return -1;
    }
    return 0;
} */