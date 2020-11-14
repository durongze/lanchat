#ifndef __TCP_CHAT__
#define __TCP_CHAT__

#include <string>
#include <winsock.h>
#include <map>
// #include <semaphore.h>

#define CAMERA_WIDTH 320
#define CAMERA_HEIGHT 240

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
	HANDLE sem;
public:
	int Init(int p = 3000);

	int Accept();

	int Recv(TcpPackage& img);

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
	int Send(TcpPackage& img);
	int TransBitMap(const TcpPackage& img, HBITMAP& hbitmap);
	int Uninit();
};

class TcpChat {
	TcpChat();
public:
	~TcpChat();
	static TcpChat *GetInstance();
	static int SetRemoteSvr(std::string ip = "127.0.0.1", int port = 8888);
	static int ConnectRemoteSvr(DWORD *arg);
	static int DisConnectRemoteSvr(DWORD *arg);
	int Init(FuncDrawWin drawWin);
	static int Accept(DWORD *arg);
	HBITMAP ReadImage(LPCWSTR path);

	static int SendImage(DWORD *arg);
	static int RecvImage(DWORD *arg);
	static int TransBitMap(HBITMAP& hbitmap);
	static int SendText(TcpPackage& arg);
	static int RecvText(TcpPackage& arg);
	static int SendTextCtx(TcpPackage* arg);
	static int RecvTextCtx(DWORD* arg);
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
