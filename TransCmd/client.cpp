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
	int ret;
	DWORD threadId;
	TcpChat* tc = TcpChat::GetInstance();
	tc->Init(DrawWindowRegon);
	tc->SetRemoteSvr("127.0.0.1", atoi("3000"));
	tc->ConnectRemoteSvr((DWORD*)tc);
	TcpPackage msg = { sizeof(TcpPackage) - 4, {"ss 2>&1"} };
	ret = tc->SendTextCtx(&msg);
	std::cout << "BitMapSize:" << sizeof(BITMAPINFOHEADER) << std::endl;
	std::cout << "Send ret:" << ret << msg.buf << std::endl;
	do {
		ret = tc->RecvText(msg);
	} while(ret == 0);
	std::cout << "RecvText ret:" << ret << msg.buf << std::endl;

	return 0;
}