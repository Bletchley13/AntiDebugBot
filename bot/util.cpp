#include "util.h" 
static void encrypt(unsigned char* data,unsigned char* key,size_t datalen,size_t keylen)
{
	//********** YOUR CODE ***************//
	unsigned int i ; 
	for (i = 0; i < datalen ;i++ , data++ , key++)
		*data= *data ^ *key; 
}


BOOL sendDataUnit(SOCKET* ConnectSocket,DataUnit* data)
{
	char* buffer = (char*)malloc(data->length+sizeof(u_long));
	if(buffer == NULL)
		return FALSE;
	u_long length = htonl(data->length);
	int ret;
	memcpy(buffer,&length,sizeof(u_long));
	*((u_long*)buffer) = length;
	memcpy(buffer+sizeof(u_long),data->data,data->length);
	ret = send(*ConnectSocket, buffer , data->length+sizeof(u_long) ,0);
	//printf("[] %s\n",data->data);
	if(ret == SOCKET_ERROR)
	{
		return FALSE;
	}
	
	return TRUE;
}

BOOL sendEncryptedDataUnit(SOCKET* ConnectSocket,DataUnit* data,char* key)
{
	const size_t keylen = strlen(key);
	encrypt((unsigned char*)data->data,(unsigned char*)key,data->length,keylen);
	return sendDataUnit(ConnectSocket,data);	
}

BOOL recvDataUnit(SOCKET* ConnectSocket,DataUnit* data)
{
	u_long length;
	int ret;
	ret = recv(*ConnectSocket,(char*)&length,sizeof(length),0);
	if (ret <= 0)
	{
		return FALSE;
	}
	data->length = ntohl(length);

	// Reserve a space for extra '\0' character
	data->data = (char*)malloc(sizeof(char)*(data->length + 1));
	
	u_long index =0;
	while(index < data->length)
	{
		ret = recv(*ConnectSocket,data->data+index,1,0);
		if (ret <= 0)
			return FALSE;
		index += ret;
	}

	// Add extra '\0' character for ease of parsing
	data->data[index] = '\0';
	//printf("[recv] %s\n",data->data);
	return TRUE;
}

BOOL recvEncryptedDataUnit(SOCKET* ConnectSocket,DataUnit* data,char* key)
{
	BOOL success = recvDataUnit(ConnectSocket,data);
	if(!success)
	{
		return FALSE;
	}
	size_t keylen = strlen(key);
	encrypt((unsigned char*)data->data,(unsigned char*)key,data->length,keylen);
	return TRUE;
}


BOOL Connect(SOCKET* connectSocket,char* host,char* port)
{
    struct addrinfo *addresult = NULL,
                    hints;
	*connectSocket = INVALID_SOCKET;
	BOOL success = FALSE;
    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_UNSPEC; //IPv6 or IPv4 unspecified
    hints.ai_socktype = SOCK_STREAM; //socket type
    hints.ai_protocol = IPPROTO_TCP; //protocol

    //resolve the server address and port
    int result = getaddrinfo(host, port, &hints, &addresult);
    if ( result != 0 ) 
	{
        printf("getaddrinfo failed with error: %d\n", result);
        goto end;
    }

    //attempt to connect to an address until one succeeds
    for(addrinfo* ptr = addresult; ptr != NULL; ptr = ptr->ai_next)
	{

        //create a socket for connecting to server
        SOCKET sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (sock == INVALID_SOCKET)
		{
            printf("Socket creation fails with error: %ld\n", WSAGetLastError());
            goto end;
        }

        //connect to server
        result = connect( sock, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (result == SOCKET_ERROR)
		{
            closesocket(sock);
            continue;
        }
		*connectSocket = sock;
        break;
    }
end:
	if(addresult != NULL)
		freeaddrinfo(addresult);
    if (*connectSocket == INVALID_SOCKET)
	{
        printf("Unable to connect to server!\n");
        return FALSE;
    }
	else
		return TRUE;
}


