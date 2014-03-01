#pragma once
#include "util.h"

#define MAX_KEY_LEN 100
struct AuthenticationProtocol
{
	//int AuthenOp;
	char* AuthMessege;
	//char* AuthenResponse; 
	BOOL (*AuthenFunction)( SOCKET *,char* ); 
};
typedef struct AuthenticationProtocol AuthenticationProtocol;

///
/// Number of authentication messages
///
#define AuthWayNum 4

#define AuthOpHello 0
#define AuthOpHelloAck 1
#define AuthOpSendIP 2
#define AuthOpFin 3

//***** YOUR CODE HERE ******/
// Please correct the following #define statement
#define AuthMsgHello "/hello"
#define AuthMsgHelloAck "/hello Ok "
#define AuthMsgSendIP "/sendIP"
#define AuthMsgFin "/fin"

///
/// Do authentication with the server.
///
/// \return TRUE on success, FALSE on failure
///
extern BOOL Authentication(SOCKET *ConnectSocket ,char* key );