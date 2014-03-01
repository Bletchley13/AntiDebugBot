#pragma once

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdio.h>

#include "stdafx.h"
#pragma comment(lib,"Ws2_32.lib")

struct DataUnit
{
	size_t length;
	char* data;
};

extern BOOL AD_IsDebuggerPresent();
extern BOOL AntiDebug();

///
/// Send a data unit to the server.
/// 
/// \return TRUE on success, FALSE on failure
///
extern BOOL sendDataUnit(SOCKET* ConnectSocket,DataUnit* data);
extern BOOL sendEncryptedDataUnit(SOCKET* ConnectSocket,DataUnit* data,char* Key);

///
/// Receive a data unit from the server.
/// It will append an extra '\0' to the end of the data.
/// It would be useful if you want to use it as a string.
///
/// \return TRUE on success, FALSE on failure
///
extern BOOL recvDataUnit(SOCKET* ConnectSocket,DataUnit* data);
extern BOOL recvEncryptedDataUnit(SOCKET* ConnectSocket,DataUnit* data,char* Key);
extern BOOL Connect(SOCKET* ConnectSocket,char* host,char* port);
