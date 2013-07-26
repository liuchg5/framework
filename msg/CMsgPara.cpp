
#include "CMsgPara.h"


CMsgRequestLoginPara::~CMsgRequestLoginPara()
{

}
int CMsgRequestLoginPara::buf2para(char * buf)
{
	memcpy(username, buf, 64);
	memcpy(password, buf+64, 16);
	return 64+16;
}

int CMsgRequestLoginPara::para2buf(char * buf)
{
	memcpy(buf, username, 64);
	memcpy(buf+64, password, 16);
	return 64+16;
}

void CMsgRequestLoginPara::print()
{
	printf("------CMsgRequestLoginPara begin print-------\n");
	printf("username: %s \n", username);
	printf("password: %s \n", password);
	printf("-----------------END------------------\n");
}





CMsgResponseLoginPara::~CMsgResponseLoginPara()
{
	
}
int CMsgResponseLoginPara::buf2para(char * buf)
{
	char * p = buf;
	memcpy(&m_unUin, p, sizeof(m_unUin));	p+= sizeof(m_unUin);
	memcpy(&m_unSessionID, p, sizeof(m_unSessionID));	p+=sizeof(m_unSessionID);
	memcpy(&m_bResultID, p, sizeof(m_bResultID));	p+=sizeof(m_bResultID);
	memcpy(&m_stPlayerInfo, p, sizeof(m_stPlayerInfo));	p+=sizeof(m_stPlayerInfo);
	return (p - buf);
}

int CMsgResponseLoginPara::para2buf(char * buf)
{
	char * p = buf;
	memcpy(p, &m_unUin, sizeof(m_unUin));	p+= sizeof(m_unUin);
	memcpy(p, &m_unSessionID, sizeof(m_unSessionID));	p+=sizeof(m_unSessionID);
	memcpy(p, &m_bResultID, sizeof(m_bResultID));	p+=sizeof(m_bResultID);
	memcpy(p, &m_stPlayerInfo, sizeof(m_stPlayerInfo));	p+=sizeof(m_stPlayerInfo);
	return (p - buf);
}

void CMsgResponseLoginPara::print()
{
	printf("------CMsgResponseLoginPara begin print-------\n");
	printf("m_unUin: %d \n", m_unUin);
	printf("m_unSessionID: %d \n", m_unSessionID);
	printf("m_bResultID: %d \n", m_bResultID);
	printf("m_stPlayerInfo:  \n");
	printf("  m_szUserName: %s \n", m_stPlayerInfo.m_szUserName);
	printf("  m_unUin: %d \n", m_stPlayerInfo.m_unUin);
	printf("  m_bySex: %d \n", m_stPlayerInfo.m_bySex);
	printf("  m_unLevel: %d \n", m_stPlayerInfo.m_unLevel);
	printf("  m_unWin: %d \n", m_stPlayerInfo.m_unWin);
	printf("  m_unLose: %d \n", m_stPlayerInfo.m_unLose);
	printf("  m_unRun: %d \n", m_stPlayerInfo.m_unRun);
	printf("-----------------END------------------\n");
}



void CRequestUserInfoPara::print()
{
	printf("------CRequestUserInfoPara begin print-------\n");
	printf("m_szUserName: %s \n", m_szUserName);
	printf("-----------------END------------------\n");
}

void CResponseUserInfoPara::print()
{
	printf("------CResponseUserInfoPara begin print-------\n");
	printf("m_stPlayerInfo: \n" );
	printf("  m_szUserName: %s \n", m_stPlayerInfo.m_szUserName);
	printf("  m_unUin: %d \n", m_stPlayerInfo.m_unUin);
	printf("  m_bySex: %d \n", m_stPlayerInfo.m_bySex);
	printf("  m_unLevel: %d \n", m_stPlayerInfo.m_unLevel);
	printf("  m_unWin: %d \n", m_stPlayerInfo.m_unWin);
	printf("  m_unLose: %d \n", m_stPlayerInfo.m_unLose);
	printf("  m_unRun: %d \n", m_stPlayerInfo.m_unRun);
	printf("m_szPwd: %s \n", m_szPwd);
	printf("m_bResultID: %d \n", m_bResultID);
	printf("-----------------END------------------\n");
}
