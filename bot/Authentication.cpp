#include "Authentication.h"
#include "util.h"

AuthenticationProtocol AuthTable[ AuthWayNum ];

static BOOL getLocalIP(char* buf,int buf_size)
{
	if(gethostname(buf, buf_size) == SOCKET_ERROR)
	{
		printf("Fail to get the local host name: %d\n", WSAGetLastError());
		return FALSE;
	}

	struct hostent *host = gethostbyname(buf);
	if(host == NULL)
	{
		printf("gethostbyname error: %d\n", WSAGetLastError());
		return FALSE;
	}
	memset(buf,0,buf_size);
	_snprintf_s(buf,buf_size,_TRUNCATE,"%d.%d.%d.%d",((struct in_addr *)(host->h_addr))->S_un.S_un_b.s_b1,
					((struct in_addr *)(host->h_addr))->S_un.S_un_b.s_b2,
					((struct in_addr *)(host->h_addr))->S_un.S_un_b.s_b3,
					((struct in_addr *)(host->h_addr))->S_un.S_un_b.s_b4);
	
	return TRUE;
}


static BOOL do_authHello(SOCKET *ConnectSocket,char* key)
{
	DataUnit du;
	du.data = AuthTable[AuthOpHello].AuthMessege;
	du.length = strlen(du.data);
	return sendDataUnit(ConnectSocket,&du);
}

static BOOL do_authHelloACK(SOCKET *ConnectSocket,char* key)
{
	DataUnit du;

	BOOL success = recvDataUnit(ConnectSocket,&du);
	if( !success ||
		strncmp(du.data,AuthTable[AuthOpHelloAck].AuthMessege,
				strlen(AuthTable[AuthOpHelloAck].AuthMessege)) != 0)
	{
		return FALSE;
	}
	return TRUE;
}

static BOOL do_authSendIP(SOCKET *ConnectSocket,char* key)
{
	DataUnit du;
	char addr[80];
	char data[80];
	if(!getLocalIP(addr,sizeof(addr)))
		return FALSE;
	_snprintf_s(data,sizeof(data),_TRUNCATE,"%s %s",
		AuthTable[AuthOpSendIP].AuthMessege,addr);
	du.data= data;
	du.length = strlen(data);
	return sendDataUnit(ConnectSocket,&du);
}

static BOOL do_authFin(SOCKET *ConnectSocket,char* key)
{
	DataUnit du;
	char *dataTmp = NULL;
	BOOL success = FALSE;
	if(!recvDataUnit(ConnectSocket,&du))
		goto end;
	char* cmd = strtok(du.data," ");
	if(strcmp(cmd,AuthTable[AuthOpFin].AuthMessege) != 0)
	{
		printf("Invalid response %s message: %s\n", AuthTable[AuthOpFin].AuthMessege, cmd);
		goto end;
	}
	char* sessionKey = cmd + strlen(cmd) + 1;
	if(strlen(sessionKey) > MAX_KEY_LEN)
	{
		printf("Session key too long: %s (%u)\n", sessionKey, (unsigned)strlen(sessionKey));
		goto end;
	}
	strncpy(key, sessionKey, MAX_KEY_LEN + 1);
	key[MAX_KEY_LEN ] = '\0';
	printf("Receive session key: %s (len: %u)\n", key, (unsigned)strlen(key));
	success = TRUE;
end:
	free(dataTmp);
	return success;
}

BOOL initializeAuthentication()
{
	printf("[initialAuthentication] Start Initial\n");
	AuthTable[AuthOpHello].AuthMessege = AuthMsgHello;
	AuthTable[AuthOpHello].AuthenFunction = do_authHello;

	AuthTable[AuthOpHelloAck].AuthMessege = AuthMsgHelloAck;
	AuthTable[AuthOpHelloAck].AuthenFunction = do_authHelloACK;

	AuthTable[AuthOpSendIP].AuthMessege = AuthMsgSendIP;
	AuthTable[AuthOpSendIP].AuthenFunction = do_authSendIP;

	AuthTable[AuthOpFin].AuthMessege = AuthMsgFin;
	AuthTable[AuthOpFin].AuthenFunction = do_authFin;

	return TRUE;
}

BOOL Authentication(SOCKET *ConnectSocket, char* key)
{
	if(!initializeAuthentication())
		return FALSE;
	for(int i = 0; i < AuthWayNum; i++)
	{
		printf("[Authentication] Authentication Step %d\n",i);
		BOOL success = (*(AuthTable[i].AuthenFunction))(ConnectSocket,key);
		if(!success)
		{	
			printf("Authentication Fails\n");
			return FALSE;
		}
	}
	printf("[Authentication] Authentication OK\n");
	return TRUE;
}