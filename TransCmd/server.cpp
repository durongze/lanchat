#include <iostream>
#include <string>
#include <map>
#include "TcpChatX.h"
#include "dump.h"

#ifdef _WIN32
#define PATH_SEP ";"
#define DIR_SEP "\\"
#define USER_HOME "D:"
#define NDK_VER "android-ndk-r19c" 
#define SYSTEM_DIR "windows-x86_64" 
#else
#define PATH_SEP ":"
#define DIR_SEP "/"
#define USER_HOME "/home/du"
#define NDK_VER "22.0.7026061"
#define SYSTEM_DIR "linux-x86_64"
#endif

#define ENV_NAME_PATH     "PATH"
#define ANDROID_USER_PATH USER_HOME DIR_SEP "Android" DIR_SEP

#define ADB_TOOL_HOME ANDROID_USER_PATH "Sdk" DIR_SEP "platform-tools" 
#define SDK_TOOL_HOME ANDROID_USER_PATH "Sdk" DIR_SEP "tools"            DIR_SEP "bin" DIR_SEP
#define NDK_TOOL_HOME ANDROID_USER_PATH "Sdk" DIR_SEP "ndk" DIR_SEP NDK_VER DIR_SEP "toolchains"  DIR_SEP

#define AARCH64_HOME  NDK_TOOL_HOME "aarch64-linux-android-4.9" DIR_SEP 
#define AARCH64_PATH  AARCH64_HOME  "prebuilt" DIR_SEP SYSTEM_DIR DIR_SEP "bin"

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
	pathVal += PATH_SEP;
	pathVal += AARCH64_PATH;
	pathVal += PATH_SEP;
	pathVal += ADB_TOOL_HOME;
	pathVal += PATH_SEP;
	pathVal += NDK_TOOL_HOME;
	pathVal += PATH_SEP;
	pathVal += SDK_TOOL_HOME;
	putenv(pathVal.c_str());
    std::cout << pathVal.c_str() << std::endl;
	return 0;
}
int main(int argc, char** argv)
{
	// SetEnvCfg();
	DWORD threadId;
	TcpChat *tc = TcpChat::GetInstance();
	tc->Init(DrawWindowRegon);
#ifdef _WIN32
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)tc->Accept, tc, 0, &threadId);
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)tc->RecvTextCtx, tc, 0, &threadId);
#else
    pthread_t pid;
    pthread_create(&pid, NULL, tc->Accept, tc);
    pthread_create(&pid, NULL, tc->RecvTextCtx, tc);
#endif
    getchar();

    struct stat statbuf = {0};
    stat(argv[1], &statbuf);
    FILE *fp = fopen(argv[1], "r");
    char *ctx = (char *)malloc(statbuf.st_size);
    fread(ctx, statbuf.st_size, 1, fp);
    DYZLogHexString(1, ctx, statbuf.st_size);
    free(ctx);
    fclose(fp);
	return 0;
}
