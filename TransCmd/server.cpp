#include <iostream>
#include <string>
#include <winsock.h>
#include <map>
#include "TcpChatX.h"

int DrawWindowRegon(TcpPackage *tp)
{
	std::cout << "msg:" << tp->buf << std::endl;
	return 0;
}
int main()
{
	DWORD threadId;
	TcpChat *tc = TcpChat::GetInstance();
	tc->Init(DrawWindowRegon);
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)tc->Accept, tc, 0, &threadId);
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)tc->RecvTextCtx, tc, 0, &threadId);
	getchar();
	return 0;
}