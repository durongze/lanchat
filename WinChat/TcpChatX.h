#ifndef __TCP_CHAT__
#define __TCP_CHAT__

#include <string>
#include <winsock.h>
#include <map>

#define CAMERA_WIDTH 128
#define CAMERA_HEIGHT 128

typedef struct {
	long size;
	char buf[CAMERA_WIDTH * CAMERA_HEIGHT * 4 + sizeof(BITMAPINFOHEADER)];
}TcpPackage;

typedef int (*FuncDrawWin)(TcpPackage* tp);

class TcpServer
{
	WSADATA wsaData;
	struct sockaddr_in servAddr;
	SOCKET serverSocket;
	std::map<SOCKET, struct sockaddr_in> m_cli;
public:
	int Init();

	int Accept();

	int Send(const TcpPackage& img);

	int TransBitMap(const BITMAP& bm, BITMAPINFOHEADER& bi);

	int TransTcpPackage(const BITMAP& bm, const BITMAPINFOHEADER& bi, TcpPackage& tp);

	int Send(const BITMAP& bitmap);

	int Uninit();
};

class TcpClient
{
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	SOCKET sock;
	std::string ipAddr;
	int port;
public:
	TcpClient(std::string ip = "127.0.0.1", int p = 8888);
	int Init();
	int SetRemoteSvr(const char* ip, int p);
	int Connect(const char* ip, int p);
	int Connect();
	int DisConnect();
	int Recv(TcpPackage& img);
	int TransBitMap(const TcpPackage& img, HBITMAP& hbitmap);
	int Uninit();
};

class TcpChat {
	TcpChat();
public:
	TcpChat::~TcpChat();
	static TcpChat *GetInstance();
	static int SetRemoteSvr(std::string ip = "127.0.0.1", int port = 8888);
	static int ConnectRemoteSvr(DWORD *arg);
	static int DisConnectRemoteSvr(DWORD *arg);
	int Init(FuncDrawWin drawWin);
	static int Accept(DWORD *arg);
	HBITMAP ReadImage(wchar_t *path);

	static int SendImage(DWORD *arg);
	static int RecvImage(DWORD *arg);
	static int TransBitMap(HBITMAP& hbitmap);
	static int SendText(TcpPackage& arg);
	static int RecvText(TcpPackage& arg);
	int SetFuncDrawWin(FuncDrawWin drawWin);
private:
	TcpServer svr;
	TcpClient cli;
	FuncDrawWin funcDrawWin;
	TcpPackage *img;
public:
	static TcpChat *tc;

};

#endif
