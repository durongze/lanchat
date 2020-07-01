// WinChat.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include <windows.h>
#include <winsock.h>
#include <map>
#include "WinChat.h"
#include "DisplayManager.h"
#include "DuplicationManager.h"
#include "OutputManager.h"
#include "ThreadManager.h"

#define MAX_LOADSTRING 100
#define CAMERA_WIDTH 320
#define CAMERA_HEIGHT 240

// 全局变量: 
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名
HWND g_hIpAddr;
HWND g_hPort;
HWND g_hSendMsg;
HWND g_hRecvMsg;
HWND g_hConnBtn;
HWND g_hSendBtn;
HWND g_hCamera;
HWND g_hMainWindow;

HRESULT SystemTransitionsExpectedErrors[] = {
	DXGI_ERROR_DEVICE_REMOVED,
	DXGI_ERROR_ACCESS_LOST,
	static_cast<HRESULT>(WAIT_ABANDONED),
	S_OK                                    // Terminate list with zero valued HRESULT
};

// These are the errors we expect from IDXGIOutput1::DuplicateOutput due to a transition
HRESULT CreateDuplicationExpectedErrors[] = {
	DXGI_ERROR_DEVICE_REMOVED,
	static_cast<HRESULT>(E_ACCESSDENIED),
	DXGI_ERROR_UNSUPPORTED,
	DXGI_ERROR_SESSION_DISCONNECTED,
	S_OK                                    // Terminate list with zero valued HRESULT
};

// These are the errors we expect from IDXGIOutputDuplication methods due to a transition
HRESULT FrameInfoExpectedErrors[] = {
	DXGI_ERROR_DEVICE_REMOVED,
	DXGI_ERROR_ACCESS_LOST,
	S_OK                                    // Terminate list with zero valued HRESULT
};

// These are the errors we expect from IDXGIAdapter::EnumOutputs methods due to outputs becoming stale during a transition
HRESULT EnumOutputsExpectedErrors[] = {
	DXGI_ERROR_NOT_FOUND,
	S_OK                                    // Terminate list with zero valued HRESULT
};

// 此代码模块中包含的函数的前向声明: 
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

#pragma comment(lib,"Ws2_32.lib ")
typedef struct  {
	long size;
	char buf[CAMERA_WIDTH * CAMERA_HEIGHT * 4 + sizeof(BITMAPINFOHEADER)];
}TcpPackage;

int DrawWindowRegon(const HBITMAP& bitmap)
{
	RECT regon = { 710, 70, 710 + CAMERA_WIDTH, 70 + CAMERA_HEIGHT };
	RedrawWindow(g_hMainWindow, &regon, NULL, RDW_INVALIDATE | RDW_UPDATENOW);// | RDW_ERASE);
	UpdateWindow(g_hMainWindow);
	return 0;
}

class TcpServer
{
	WSADATA wsaData;
	struct sockaddr_in servAddr;
	SOCKET serverSocket;
	std::map<SOCKET, struct sockaddr_in> m_cli;
public:
	int Init()
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
		servAddr.sin_port = htons(8888);
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

	int Accept()
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

	int Send(TcpPackage& img)
	{
		fd_set writeSet;
		struct timeval tm = {0,30};
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
	int TransBitMap(const BITMAP& bm, BITMAPINFOHEADER& bi)
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

	int TransTcpPackage(const BITMAP& bm, const BITMAPINFOHEADER& bi, TcpPackage& tp)
	{
		memset(&tp, 0, sizeof(TcpPackage));
		tp.size = sizeof(BITMAPINFOHEADER) + bm.bmWidthBytes * bm.bmHeight;
		memcpy(tp.buf, (char*)&bi, sizeof(BITMAPINFOHEADER));
		memcpy(tp.buf + sizeof(BITMAPINFOHEADER), (char*)&bm.bmBits, sizeof(bm.bmWidthBytes * bm.bmHeight));
		return 0;
	}

	int Send(HBITMAP& bitmap)
	{
		BITMAP bm;
		BITMAPINFOHEADER bi;
		TcpPackage tp;
		auto hcopy = (HBITMAP)CopyImage(bitmap, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
		GetObject(hcopy, sizeof(bm), &bm);
		TransBitMap(bm, bi);
		TransTcpPackage(bm, bi, tp);
		int ret = Send(tp);
		if (ret > 0) {
			Sleep(1);
		}
		DeleteObject(hcopy);
		return ret;
	}
	int Uninit()
	{
		std::map<SOCKET, struct sockaddr_in>::iterator iter;
		for (iter = m_cli.begin(); iter != m_cli.end(); ++iter)
		{
			closesocket(iter->first);
		}
		WSACleanup();
		return 0;
	}
};

class TcpClient
{
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	SOCKET sock;
	std::string ipAddr;
	int port;
public:
	TcpClient(std::string ip = "127.0.0.1", int p = 8888):ipAddr(ip), port(p) {}
	int Init()
	{
		if (WSAStartup(sockVersion, &wsaData) != 0) {
			return 0;
		}
		return 0;
	}

	int SetRemoteSvr(const char* ip, int p)
	{
		ipAddr = ip;
		port = p;
		return 0;
	}
	int Connect(const char* ip, int p)
	{
		SetRemoteSvr(ip, p);
		return Connect();
	}
	int Connect()
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

	int Recv(TcpPackage& img)
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
	int TransBitMap(const TcpPackage& img, HBITMAP& hbitmap)
	{
		BITMAPINFOHEADER* bi = (BITMAPINFOHEADER*)img.buf;
		BYTE* bits = (BYTE*)img.buf + sizeof(BITMAPINFOHEADER);
		hbitmap = CreateBitmap(bi->biWidth, bi->biHeight, bi->biPlanes, bi->biBitCount, bits);
#if 1
		OpenClipboard(NULL);
		EmptyClipboard();
		SetClipboardData(CF_BITMAP, hbitmap);
		CloseClipboard();
#endif
		return 0;
	}

	int Uninit()
	{
		closesocket(sock);
		WSACleanup();
		return 0;
	}
};

class TcpChat {
	TcpChat():svr(), cli() {	}
public:
	static TcpChat *GetInstance() {
		if (tc == NULL) { tc = new TcpChat; }
		return tc;
	}	
	static int SetRemoteSvr(std::string ip = "127.0.0.1", int port = 8888) 
	{
		return tc->cli.SetRemoteSvr(ip.c_str(), port);
	}
	static int ConnectRemoteSvr(DWORD *arg) 
	{
		TcpChat* tc = (TcpChat*)arg;
		return tc->cli.Connect();
	}
    int Init()
	{
		svr.Init();
		cli.Init();
		return 0;
	}
	static int Accept(DWORD *arg)
	{
		TcpChat* tc = (TcpChat*)arg;
		return tc->svr.Accept();
	}
	HBITMAP ReadImage(wchar_t *path)
	{
		return (HBITMAP)LoadImage(NULL, path, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	}

	static int SendImage(DWORD *arg)
	{
		int ret;
		HBITMAP bitmap;
		TcpChat* tc = (TcpChat*)arg;
		// bitmap = tc->ReadImage(TEXT(""));
		RECT rctA; // 定义一个RECT结构体，存储窗口的长宽高
		HWND cam = g_hCamera;
		::GetWindowRect(cam, &rctA); // 通过窗口句柄获得窗口的大小存储在rctA结构中
		int nWidth = rctA.right - rctA.left; // GetSystemMetrics(SM_CXSCREEN); // 得到屏幕的分辨率的x    
		int nHeight = rctA.bottom - rctA.top; // GetSystemMetrics(SM_CYSCREEN); // 得到屏幕分辨率的y    
		POINT mouse;
		while (1) {
			GetCursorPos(&mouse);
			HDC hdcDesk = GetDC(GetDesktopWindow()); // 得到屏幕的dc    
			HDC hdcCopy = CreateCompatibleDC(hdcDesk); //  
			HBITMAP hBitmap = CreateCompatibleBitmap(hdcDesk, nWidth, nHeight); // 得到位图    
			HGDIOBJ hObj = SelectObject(hdcCopy, hBitmap); // 好像总得这么写。             
			BitBlt(hdcCopy, 0, 0, nWidth, nHeight, hdcDesk,  mouse.x, mouse.y, SRCCOPY);
			ret = tc->svr.Send(hBitmap);
			if (ret > 0) {
				Sleep(1);
#if 0
				OpenClipboard(NULL);
				EmptyClipboard();
				SetClipboardData(CF_BITMAP, hBitmap);
				CloseClipboard();
#endif
			}
			DeleteObject(hObj);
			DeleteDC(hdcDesk);
			Sleep(111);
		}
	}
	static int RecvImage(DWORD *arg)
	{
		int ret;
		TcpPackage tp;
		HBITMAP bitmap;
		TcpChat* tc = (TcpChat*)arg;
		while (1) {
			ret = tc->cli.Recv(tp);
			if (ret > 0) {
				tc->cli.TransBitMap(tp, bitmap);
				DrawWindowRegon(bitmap);
			}
			Sleep(111);
		}
		return ret;
	}

	static int SendIext(DWORD *arg)
	{
		TcpPackage *tp = (TcpPackage *)arg;
		return TcpChat::GetInstance()->svr.Send(*tp);
	}
	static int RecvText(DWORD *arg)
	{
		TcpPackage *tp = (TcpPackage *)arg;
		return TcpChat::GetInstance()->cli.Recv(*tp);
	}
private:
	TcpServer svr;
	TcpClient cli;
public:
	static TcpChat *tc;
};
TcpChat *TcpChat::tc = NULL;
//
// Entry point for new duplication threads
//
DWORD WINAPI DDProc(_In_ void* Param)
{
	// Classes
	DISPLAYMANAGER DispMgr;
	DUPLICATIONMANAGER DuplMgr;

	// D3D objects
	ID3D11Texture2D* SharedSurf = nullptr;
	IDXGIKeyedMutex* KeyMutex = nullptr;

	// Data passed in from thread creation
	THREAD_DATA* TData = reinterpret_cast<THREAD_DATA*>(Param);

	// Get desktop
	DUPL_RETURN Ret;
	HDESK CurrentDesktop = nullptr;
	CurrentDesktop = OpenInputDesktop(0, FALSE, GENERIC_ALL);
	if (!CurrentDesktop)
	{
		// We do not have access to the desktop so request a retry
		SetEvent(TData->ExpectedErrorEvent);
		Ret = DUPL_RETURN_ERROR_EXPECTED;
		goto Exit;
	}

	// Attach desktop to this thread
	bool DesktopAttached = SetThreadDesktop(CurrentDesktop) != 0;
	CloseDesktop(CurrentDesktop);
	CurrentDesktop = nullptr;
	if (!DesktopAttached)
	{
		// We do not have access to the desktop so request a retry
		Ret = DUPL_RETURN_ERROR_EXPECTED;
		goto Exit;
	}

	// New display manager
	DispMgr.InitD3D(&TData->DxRes);

	// Obtain handle to sync shared Surface
	HRESULT hr = TData->DxRes.Device->OpenSharedResource(TData->TexSharedHandle, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&SharedSurf));
	if (FAILED(hr))
	{
		Ret = ProcessFailure(TData->DxRes.Device, L"Opening shared texture failed", L"Error", hr, SystemTransitionsExpectedErrors);
		goto Exit;
	}

	hr = SharedSurf->QueryInterface(__uuidof(IDXGIKeyedMutex), reinterpret_cast<void**>(&KeyMutex));
	if (FAILED(hr))
	{
		Ret = ProcessFailure(nullptr, L"Failed to get keyed mutex interface in spawned thread", L"Error", hr);
		goto Exit;
	}

	// Make duplication manager
	Ret = DuplMgr.InitDupl(TData->DxRes.Device, TData->Output);
	if (Ret != DUPL_RETURN_SUCCESS)
	{
		goto Exit;
	}

	// Get output description
	DXGI_OUTPUT_DESC DesktopDesc;
	RtlZeroMemory(&DesktopDesc, sizeof(DXGI_OUTPUT_DESC));
	DuplMgr.GetOutputDesc(&DesktopDesc);

	// Main duplication loop
	bool WaitToProcessCurrentFrame = false;
	FRAME_DATA CurrentData;

	while ((WaitForSingleObjectEx(TData->TerminateThreadsEvent, 0, FALSE) == WAIT_TIMEOUT))
	{
		if (!WaitToProcessCurrentFrame)
		{
			// Get new frame from desktop duplication
			bool TimeOut;
			Ret = DuplMgr.GetFrame(&CurrentData, &TimeOut);
			if (Ret != DUPL_RETURN_SUCCESS)
			{
				// An error occurred getting the next frame drop out of loop which
				// will check if it was expected or not
				break;
			}

			// Check for timeout
			if (TimeOut)
			{
				// No new frame at the moment
				continue;
			}
		}

		// We have a new frame so try and process it
		// Try to acquire keyed mutex in order to access shared surface
		hr = KeyMutex->AcquireSync(0, 1000);
		if (hr == static_cast<HRESULT>(WAIT_TIMEOUT))
		{
			// Can't use shared surface right now, try again later
			WaitToProcessCurrentFrame = true;
			continue;
		}
		else if (FAILED(hr))
		{
			// Generic unknown failure
			Ret = ProcessFailure(TData->DxRes.Device, L"Unexpected error acquiring KeyMutex", L"Error", hr, SystemTransitionsExpectedErrors);
			DuplMgr.DoneWithFrame();
			break;
		}

		// We can now process the current frame
		WaitToProcessCurrentFrame = false;

		// Get mouse info
		Ret = DuplMgr.GetMouse(TData->PtrInfo, &(CurrentData.FrameInfo), TData->OffsetX, TData->OffsetY);
		if (Ret != DUPL_RETURN_SUCCESS)
		{
			DuplMgr.DoneWithFrame();
			KeyMutex->ReleaseSync(1);
			break;
		}

		// Process new frame
		Ret = DispMgr.ProcessFrame(&CurrentData, SharedSurf, TData->OffsetX, TData->OffsetY, &DesktopDesc);
		if (Ret != DUPL_RETURN_SUCCESS)
		{
			DuplMgr.DoneWithFrame();
			KeyMutex->ReleaseSync(1);
			break;
		}

		// Release acquired keyed mutex
		hr = KeyMutex->ReleaseSync(1);
		if (FAILED(hr))
		{
			Ret = ProcessFailure(TData->DxRes.Device, L"Unexpected error releasing the keyed mutex", L"Error", hr, SystemTransitionsExpectedErrors);
			DuplMgr.DoneWithFrame();
			break;
		}

		// Release frame back to desktop duplication
		Ret = DuplMgr.DoneWithFrame();
		if (Ret != DUPL_RETURN_SUCCESS)
		{
			break;
		}
	}

Exit:
	if (Ret != DUPL_RETURN_SUCCESS)
	{
		if (Ret == DUPL_RETURN_ERROR_EXPECTED)
		{
			// The system is in a transition state so request the duplication be restarted
			SetEvent(TData->ExpectedErrorEvent);
		}
		else
		{
			// Unexpected error so exit the application
			SetEvent(TData->UnexpectedErrorEvent);
		}
	}

	if (SharedSurf)
	{
		SharedSurf->Release();
		SharedSurf = nullptr;
	}

	if (KeyMutex)
	{
		KeyMutex->Release();
		KeyMutex = nullptr;
	}

	return 0;
}

_Post_satisfies_(return != DUPL_RETURN_SUCCESS)
DUPL_RETURN ProcessFailure(_In_opt_ ID3D11Device* Device, _In_ LPCWSTR Str, _In_ LPCWSTR Title, HRESULT hr, _In_opt_z_ HRESULT* ExpectedErrors)
{
	HRESULT TranslatedHr;

	// On an error check if the DX device is lost
	if (Device)
	{
		HRESULT DeviceRemovedReason = Device->GetDeviceRemovedReason();

		switch (DeviceRemovedReason)
		{
		case DXGI_ERROR_DEVICE_REMOVED:
		case DXGI_ERROR_DEVICE_RESET:
		case static_cast<HRESULT>(E_OUTOFMEMORY) :
		{
			// Our device has been stopped due to an external event on the GPU so map them all to
			// device removed and continue processing the condition
			TranslatedHr = DXGI_ERROR_DEVICE_REMOVED;
			break;
		}

		case S_OK:
		{
			// Device is not removed so use original error
			TranslatedHr = hr;
			break;
		}

		default:
		{
			// Device is removed but not a error we want to remap
			TranslatedHr = DeviceRemovedReason;
		}
		}
	}
	else
	{
		TranslatedHr = hr;
	}

	// Check if this error was expected or not
	if (ExpectedErrors)
	{
		HRESULT* CurrentResult = ExpectedErrors;

		while (*CurrentResult != S_OK)
		{
			if (*(CurrentResult++) == TranslatedHr)
			{
				return DUPL_RETURN_ERROR_EXPECTED;
			}
		}
	}

	// Error was not expected so display the message box
	DisplayMsg(Str, Title, TranslatedHr);

	return DUPL_RETURN_ERROR_UNEXPECTED;
}

//
// Displays a message
//
void DisplayMsg(_In_ LPCWSTR Str, _In_ LPCWSTR Title, HRESULT hr)
{
	if (SUCCEEDED(hr))
	{
		MessageBoxW(nullptr, Str, Title, MB_OK);
		return;
	}

	const UINT StringLen = (UINT)(wcslen(Str) + sizeof(" with HRESULT 0x########."));
	wchar_t* OutStr = new wchar_t[StringLen];
	if (!OutStr)
	{
		return;
	}

	INT LenWritten = swprintf_s(OutStr, StringLen, L"%s with 0x%X.", Str, hr);
	if (LenWritten != -1)
	{
		MessageBoxW(nullptr, OutStr, Title, MB_OK);
	}

	delete[] OutStr;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 在此放置代码。

    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WINCHAT, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化: 
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINCHAT));
	DWORD  threadId;
	TcpChat *tc = TcpChat::GetInstance();
	tc->Init();
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)tc->Accept, tc, 0, &threadId);
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)tc->SendImage, tc, 0, &threadId);
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)tc->RecvImage, tc, 0, &threadId);
    MSG msg;

    // 主消息循环: 
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  函数: MyRegisterClass()
//
//  目的: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINCHAT));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_WINCHAT);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目的: 保存实例句柄并创建主窗口
//
//   注释: 
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 将实例句柄存储在全局变量中

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);
   g_hMainWindow = hWnd;

   return TRUE;
}

int HandleCmdMsg(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId = LOWORD(wParam);
	DWORD threadId;
	TcpChat *tc = TcpChat::GetInstance();
	// 分析菜单选择: 
	switch (wmId)
	{
	case IDC_CONNECT:
		tc->SetRemoteSvr("127.0.0.1", 8888);
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)tc->ConnectRemoteSvr, tc, 0, &threadId);
		break;
	case IDM_ABOUT:
		DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
		break;
	case IDM_EXIT:
		DestroyWindow(hWnd);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

int HandlePaintMsg(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	RECT rctA; // 定义一个RECT结构体，存储窗口的长宽高
	HWND cam = g_hCamera;
	::GetWindowRect(cam, &rctA); // 通过窗口句柄获得窗口的大小存储在rctA结构中
	PAINTSTRUCT ps, psCamera;
	HDC hdc = BeginPaint(hWnd, &ps);
	HDC hdcCamera = BeginPaint(g_hCamera, &psCamera);
	// TODO: 在此处添加使用 hdc 的任何绘图代码...
	int nWidth = GetSystemMetrics(SM_CXSCREEN); // 得到屏幕的分辨率的x    
	int nHeight = GetSystemMetrics(SM_CYSCREEN); // 得到屏幕分辨率的y    
	HDC hdcDesk = GetDC(GetDesktopWindow()); // 得到屏幕的dc    
	HDC hdcCopy = CreateCompatibleDC(hdcDesk); //  
	HBITMAP hBitmap = CreateCompatibleBitmap(hdcDesk, rctA.right - rctA.left, rctA.bottom - rctA.top); // 得到位图    
	HGDIOBJ hObj = SelectObject(hdcCopy, hBitmap); // 好像总得这么写。             
	BitBlt(hdcCopy, 0, 0, nWidth, nHeight, hdcDesk, 0, 0, SRCCOPY);
	BitBlt(hdcCamera, 0, 0, nWidth, nHeight, hdcCopy, 0, 0, SRCCOPY);
	DeleteObject(hObj);
	DeleteDC(hdcCopy);
	DeleteDC(hdcDesk);
	EndPaint(g_hCamera, &psCamera);
	EndPaint(hWnd, &ps);
	return 0;
}

int HandleCreateMsg(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	CreateWindow(TEXT("static"), TEXT("IP地址:"),
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_CENTER,
		30, 10, 60, 40,
		hWnd, (HMENU)IDC_IP, hInst, NULL);
	CreateWindow(TEXT("static"), TEXT("端口:"),
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_CENTER,
		430, 10, 60, 40,
		hWnd, (HMENU)IDC_PORT, hInst, NULL);
	g_hIpAddr = CreateWindow(TEXT("edit"), NULL,
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
		100, 10, 300, 40,
		hWnd, (HMENU)IDC_IP, hInst, NULL);
	g_hPort = CreateWindow(TEXT("edit"), NULL,
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
		520, 10, 110, 40,
		hWnd, (HMENU)IDC_PORT, hInst, NULL);
	g_hRecvMsg = CreateWindow(TEXT("edit"), NULL,
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
		30, 70, 600, 200,
		hWnd, (HMENU)IDC_RECV_MSG, hInst, NULL);
	g_hSendMsg = CreateWindow(TEXT("edit"), NULL, 
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
		30, 300, 600, 80,
		hWnd, (HMENU)IDC_SEND_MSG, hInst, NULL);

	g_hConnBtn = CreateWindow(TEXT("button"), TEXT("连接"),
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_CENTER,
		650, 10, 80, 40,
		hWnd, (HMENU)IDC_CONNECT, hInst, NULL);
	g_hSendBtn = CreateWindow(TEXT("button"), TEXT("发送"),
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_CENTER,
		650, 330, 80, 40,
		hWnd, (HMENU)IDC_SEND, hInst, NULL);
	g_hCamera = CreateWindow(TEXT("static"), TEXT("视频:"),
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_CENTER,
		710, 70, CAMERA_WIDTH, CAMERA_HEIGHT,
		hWnd, (HMENU)IDC_CAMERA, hInst, NULL);
	return 0;
}
//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的:    处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
		HandleCmdMsg(hWnd, message, wParam, lParam);
        break;
    case WM_PAINT:
		HandlePaintMsg(hWnd, message, wParam, lParam);
        break;
	case WM_CREATE:
		HandleCreateMsg(hWnd, message, wParam, lParam);
		break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
