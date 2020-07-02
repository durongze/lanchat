#include "stdafx.h"
#include "TcpChatX.h"

#pragma comment(lib,"Ws2_32.lib ")

	int TcpServer::Init()
	{
		if (WSAStartup(MAKEWORD(2, 2), &wsaData))
		{
			return -1;
		}

		/* 创建套接服务字 */
		serverSocket = socket(AF_INET, SOCK_STREAM, 0);
		if (serverSocket == INVALID_SOCKET)
		{
			return -2;
		}

		/* 设置IP地址 */
		memset(&servAddr, 0, sizeof(servAddr));
		servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
		servAddr.sin_family = AF_INET;
		servAddr.sin_port = htons(3000);
		/* 绑定服务器套接字 */
		if (::bind(serverSocket, (sockaddr*)&servAddr, sizeof(sockaddr)) == SOCKET_ERROR)
		{
			return -3;
		}

		/* 监听端口 */
		if (listen(serverSocket, 20) == SOCKET_ERROR)
		{
			closesocket(serverSocket);
			WSACleanup();
			return -4;
		}
		return 0;
	}

	int TcpServer::Accept()
	{
		/* 阻塞方式等待accept */
		struct sockaddr_in cliAddr;
		int len = sizeof(cliAddr);
		SOCKET clientSocket;
		while (1) {
			clientSocket = accept(serverSocket, (sockaddr*)&cliAddr, &len);
			if (clientSocket == INVALID_SOCKET)
			{
				Sleep(111);
			}
			m_cli.insert(std::make_pair(clientSocket, cliAddr));
		}
		return 0;
	}

	int TcpServer::Send(const TcpPackage& img)
	{
		fd_set writeSet;
		struct timeval tm = { 0,30 };
		int ret;
		std::map<SOCKET, struct sockaddr_in>::iterator iter;
		for (iter = m_cli.begin(); iter != m_cli.end(); ++iter)
		{
			FD_ZERO(&writeSet);
			FD_SET(iter->first, &writeSet);
			ret = select(iter->first + 1, NULL, &writeSet, NULL, &tm);
			if (ret <= 0) {
				continue;
			}
			// inet_ntoa(iter->second.sin_addr);
			ret = send(iter->first, (char*)&img, sizeof(img), NULL);
			if (ret > 0) {
				return ret;
			}
		}
		return 0;
	}
	int TcpServer::TransBitMap(const BITMAP& bm, BITMAPINFOHEADER& bi)
	{
		bi.biSize = sizeof(bi);
		bi.biWidth = bm.bmWidth;
		bi.biHeight = bm.bmHeight;
		bi.biBitCount = bm.bmBitsPixel;
		bi.biPlanes = bm.bmPlanes;
		bi.biSizeImage = bm.bmWidthBytes * bm.bmHeight;

		LONG lineSize = bi.biWidth * bi.biBitCount / 8;
		BYTE* pLineData = new BYTE[lineSize];
		BYTE* pStart;
		BYTE* pEnd;
		BYTE* pData = (BYTE*)bm.bmBits;
		LONG lineStart = 0;
		LONG lineEnd = bi.biHeight - 1;
		while (lineStart < lineEnd)
		{
			pStart = pData + (lineStart * lineSize);
			pEnd = pData + (lineEnd * lineSize);
			// Swap the top with the bottom
			memcpy(pLineData, pStart, lineSize);
			memcpy(pStart, pEnd, lineSize);
			memcpy(pEnd, pLineData, lineSize);
			// Adjust the line index
			lineStart++;
			lineEnd--;
		}
		delete pLineData;
		return 0;
	}

	int TcpServer::TransTcpPackage(const BITMAP& bm, const BITMAPINFOHEADER& bi, TcpPackage& tp)
	{
		memset(&tp, 0, sizeof(TcpPackage));
		tp.size = sizeof(BITMAPINFOHEADER) + bm.bmWidthBytes * bm.bmHeight;
		if (sizeof(tp.buf) < sizeof(bi) + bm.bmWidthBytes * bm.bmHeight) {
			return -1;
		}
		memcpy(tp.buf, (char*)&bi, sizeof(BITMAPINFOHEADER));
		memcpy(tp.buf + sizeof(BITMAPINFOHEADER), (char*)bm.bmBits, bm.bmWidthBytes * bm.bmHeight);
		return 0;
	}

	int TcpServer::Send(const BITMAP& bm)
	{
		BITMAPINFOHEADER bi;
		TcpPackage tp;
		TransBitMap(bm, bi);
		TransTcpPackage(bm, bi, tp);
		int ret = Send(tp);
		return ret;
	}
	int TcpServer::Uninit()
	{
		std::map<SOCKET, struct sockaddr_in>::iterator iter;
		for (iter = m_cli.begin(); iter != m_cli.end(); ++iter)
		{
			closesocket(iter->first);
		}
		WSACleanup();
		return 0;
	}



	TcpClient::TcpClient(std::string ip, int p) :ipAddr(ip), port(p) {}
	int TcpClient::Init()
	{
		if (WSAStartup(sockVersion, &wsaData) != 0) {
			return 0;
		}
		return 0;
	}

	int TcpClient::SetRemoteSvr(const char* ip, int p)
	{
		ipAddr = ip;
		port = p;
		return 0;
	}
	int TcpClient::Connect(const char* ip, int p)
	{
		SetRemoteSvr(ip, p);
		return Connect();
	}
	int TcpClient::Connect()
	{
		if (ipAddr.length() < 7 || ipAddr.length() > 15 || port == 0) {
			return -1;
		}
		sockaddr_in sockAddr;
		memset(&sockAddr, 0, sizeof(sockAddr));
		sockAddr.sin_family = PF_INET;
		sockAddr.sin_addr.s_addr = inet_addr(ipAddr.c_str());
		sockAddr.sin_port = htons(port);
		sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
		return connect(sock, (SOCKADDR*)& sockAddr, sizeof(SOCKADDR));
	}
	int TcpClient::DisConnect()
	{
		return closesocket(sock);
	}
	int TcpClient::Recv(TcpPackage& img)
	{
		fd_set readSet;
		struct timeval tm = { 0,30 };
		int ret;
		FD_ZERO(&readSet);
		FD_SET(sock, &readSet);
		ret = select(sock + 1, NULL, &readSet, NULL, &tm);
		if (ret <= 0) {
			return 0;
		}
		int msgSize = 0;
		do {
			ret = recv(sock, (char*)&img + msgSize, sizeof(img) - msgSize, 0);
			if (ret < 0) {
				return 0;
			}
			msgSize += ret;
		} while (msgSize < sizeof(img));
		return msgSize;
	}
	int TcpClient::TransBitMap(const TcpPackage& img, HBITMAP& hbitmap)
	{
		BITMAPINFOHEADER* bi = (BITMAPINFOHEADER*)img.buf;
		BYTE* bits = (BYTE*)img.buf + sizeof(BITMAPINFOHEADER);
		hbitmap = CreateBitmap(bi->biWidth, bi->biHeight, bi->biPlanes, bi->biBitCount, bits);
		if (hbitmap == NULL) {
			return -1;
		}
		return 0;
	}

	int TcpClient::Uninit()
	{
		closesocket(sock);
		WSACleanup();
		return 0;
	}

	TcpChat *TcpChat::tc = NULL;

	TcpChat::TcpChat() :svr(), cli(), img(NULL) {	}
	TcpChat::~TcpChat() { delete img; }
	TcpChat *TcpChat::GetInstance() 
	{
		if (tc == NULL) { tc = new TcpChat; tc->img = new TcpPackage; }
		return tc;
	}
	int TcpChat::SetRemoteSvr(std::string ip, int port)
	{
		return tc->cli.SetRemoteSvr(ip.c_str(), port);
	}
	int TcpChat::ConnectRemoteSvr(DWORD *arg)
	{
		TcpChat* tc = (TcpChat*)arg;
		return tc->cli.Connect();
	}
	int TcpChat::DisConnectRemoteSvr(DWORD *arg)
	{
		TcpChat* tc = (TcpChat*)arg;
		return tc->cli.DisConnect();
	}
	int TcpChat::Init(FuncDrawWin drawWin)
	{
		svr.Init();
		cli.Init();
		SetFuncDrawWin(drawWin);
		return 0;
	}
	int TcpChat::Accept(DWORD *arg)
	{
		TcpChat* tc = (TcpChat*)arg;
		return tc->svr.Accept();
	}
	HBITMAP TcpChat::ReadImage(wchar_t *path)
	{
		return (HBITMAP)LoadImage(NULL, path, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	}

	int TcpChat::SendImage(DWORD *arg)
	{
		int ret;
		TcpChat* tc = (TcpChat*)arg;
		int nWidth = CAMERA_WIDTH ; // GetSystemMetrics(SM_CXSCREEN); // 得到屏幕的分辨率的x    
		int nHeight = CAMERA_HEIGHT; // GetSystemMetrics(SM_CYSCREEN); // 得到屏幕分辨率的y    
		POINT mouse;
		BITMAP bm;
		while (1) {
			GetCursorPos(&mouse);
			HDC hdcDesk = GetDC(GetDesktopWindow()); // 得到屏幕的dc    
			HDC hdcCopy = CreateCompatibleDC(hdcDesk); //  GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)
			HBITMAP hBitmap = CreateCompatibleBitmap(hdcDesk, nWidth, nHeight); // 得到位图, 这3个参数必须这样传好像。  
			HGDIOBJ hObj = SelectObject(hdcCopy, hBitmap); // 好像总得这么写。   
			bool bRet = BitBlt(hdcCopy, 0, 0, nWidth, nHeight, hdcDesk, mouse.x, mouse.y, SRCCOPY);
			auto hcopy = (HBITMAP)CopyImage(hBitmap, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION); //以上代码是正确的，因为hBitmap对了
			GetObject(hcopy, sizeof(bm), &bm);
			ret = tc->svr.Send(bm);
			if (ret > 0) {
				Sleep(1);
			}
			DeleteObject(hObj);
			DeleteDC(hdcDesk);
			Sleep(111);
		}
	}
	int TcpChat::RecvImage(DWORD *arg)
	{
		int ret;
		TcpPackage tp;
		HBITMAP bitmap;
		TcpChat* tc = (TcpChat*)arg;
		while (1) {
			ret = tc->cli.Recv(tp);
			if (ret > 0) {
				*(tc->img) = tp;
				tc->cli.TransBitMap(tp, bitmap);
				if (tc->funcDrawWin) {
					OpenClipboard(NULL);
					EmptyClipboard();
					SetClipboardData(CF_BITMAP, bitmap);
					CloseClipboard();
					tc->funcDrawWin(NULL);
				}
			}
			Sleep(111);
		}
		return ret;
	}
	int TcpChat::SetFuncDrawWin(FuncDrawWin pDrawWin)
	{
		funcDrawWin = pDrawWin;
		return 0;
	}
	int TcpChat::TransBitMap(HBITMAP& hbitmap)
	{
		return tc->cli.TransBitMap(*tc->img, hbitmap);
	}
	int TcpChat::SendText(TcpPackage& tp)
	{
		return TcpChat::GetInstance()->svr.Send(tp);
	}
	int TcpChat::RecvText(TcpPackage& tp)
	{
		int ret = TcpChat::GetInstance()->cli.Recv(tp);
		tc->funcDrawWin(&tp);
		return ret;
	}

