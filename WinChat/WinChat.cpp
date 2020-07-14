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
#include "TcpChatX.h"
#include "WaveApi.h"
#include "FreeTypeBmp.h"
#include "GifEditor.h"
#include <commdlg.h>

#define MAX_LOADSTRING 100

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
HWND g_hLoadBtn;
HWND g_hCamera;
HWND g_hAvatar;
HWND g_hMainWindow;
wchar_t g_MsgBrowser[2048] = {0};
GifBitMap GrbBuffer = { 0, 0,{ 0 } };

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

char * wchar2char(const wchar_t* wchar)
{
	char * m_char;
	int len = WideCharToMultiByte(CP_ACP, 0, wchar, wcslen(wchar), NULL, 0, NULL, NULL);
	m_char = new char[len + 1];
	WideCharToMultiByte(CP_ACP, 0, wchar, wcslen(wchar), m_char, len, NULL, NULL);
	m_char[len] = '\0';
	return m_char;
}

wchar_t * char2wchar(const char* cchar)
{
	wchar_t *m_wchar;
	int len = MultiByteToWideChar(CP_ACP, 0, cchar, strlen(cchar), NULL, 0);
	m_wchar = new wchar_t[len + 1];
	MultiByteToWideChar(CP_ACP, 0, cchar, strlen(cchar), m_wchar, len);
	m_wchar[len] = '\0';
	return m_wchar;
}

int AppendToMsgBrowser(const TcpPackage& tp)
{

	GetWindowText(g_hRecvMsg, g_MsgBrowser, sizeof(g_MsgBrowser));
	int msgLen = lstrlen(g_MsgBrowser);
	if (msgLen > sizeof(g_MsgBrowser) / 3) {
		memmove(g_MsgBrowser, g_MsgBrowser + msgLen / 2, msgLen / 2);
	}
	msgLen = lstrlen(g_MsgBrowser);
	memcpy(g_MsgBrowser + msgLen, tp.buf, lstrlen((wchar_t*)tp.buf) * 2);
	msgLen = lstrlen(g_MsgBrowser);
	memcpy(g_MsgBrowser + msgLen , TEXT(" "), lstrlen(TEXT(" ")) * 2);
	SetWindowText(g_hRecvMsg, g_MsgBrowser);
	return 0;
}

int DrawWindowRegon(TcpPackage *tp)
{
	if (tp->size > 2048) {
		BITMAPINFOHEADER* bi = (BITMAPINFOHEADER*)tp->buf;
		IMAGEDATA* bits = (IMAGEDATA*)tp->buf + sizeof(BITMAPINFOHEADER);
		HBITMAP hbitmap = CreateBitmap(bi->biWidth, bi->biHeight, bi->biPlanes, bi->biBitCount, bits);
		if (hbitmap == NULL) {
			return -1;
		}
		WriteWordToBmp(*bi, bits);
		DeleteObject(hbitmap);
		RECT regonCam = { 710, 70, 710 + CAMERA_WIDTH, 70 + CAMERA_HEIGHT };
		RedrawWindow(g_hMainWindow, &regonCam, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
		UpdateWindow(g_hMainWindow);
		RECT regonAvatar = { 710 + CAMERA_WIDTH - 112, 70 + CAMERA_HEIGHT + 10,  710 + CAMERA_WIDTH, 70 + CAMERA_HEIGHT + 10 + 112 };
		RedrawWindow(g_hMainWindow, &regonAvatar, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
		UpdateWindow(g_hMainWindow);
	} else {
		AppendToMsgBrowser(*tp);
	}
	return 0;
}

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

int OpenConsole()
{
#ifdef WIN32
	_tsetlocale(LC_ALL, _T(" "));
	::AllocConsole();
	::freopen("CONIN$", "r", stdin);
	::freopen("CONOUT$", "w", stdout);
	::freopen("CONOUT$", "w", stderr);
#endif // WIN32
	return 0;
}

int CloseConsole()
{
	fclose(stdin);
	fclose(stdout);
	fclose(stderr);
	return 0;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
	OpenConsole();
	GifWrite(NULL);
	CloseConsole();
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
	DWORD threadId;
	TcpChat *tc = TcpChat::GetInstance();
	tc->Init(DrawWindowRegon);
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

int HandleCmdConnect()
{
	DWORD threadId;
	TcpChat *tc = TcpChat::GetInstance();
	static bool IsConnected = false;
	if (IsConnected) {
		SetWindowText(g_hConnBtn, TEXT("连接"));
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)tc->DisConnectRemoteSvr, tc, 0, &threadId);
		IsConnected = false;
	} else {
		SetWindowText(g_hConnBtn, TEXT("断开"));
		wchar_t ipAddr[32] = { 0 };
		wchar_t port[32] = { 0 };
		char *pIp = NULL;
		char *pPort = NULL;
		GetWindowText(g_hIpAddr, ipAddr, sizeof(ipAddr));
		GetWindowText(g_hPort, port, sizeof(port));
		pIp = wchar2char(ipAddr);
		pPort = wchar2char(port);
		tc->SetRemoteSvr(pIp, atoi(pPort));
		delete[] pIp;
		delete[] pPort;
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)tc->ConnectRemoteSvr, tc, 0, &threadId);
		IsConnected = true;
	}
	return 0;
}

int HandleCmdSend()
{
	TcpPackage msg = { 0 };
	GetWindowText(g_hSendMsg, (wchar_t*)msg.buf, sizeof(msg.buf));
	int ret = TcpChat::GetInstance()->SendText(msg);
	if (ret > 0) AppendToMsgBrowser(msg);
	return 0;
}

int HandleCmdLoad(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	OPENFILENAME ofn;
	WCHAR* szFile = new WCHAR[512];
	WCHAR* szFileTitle = new WCHAR[512];
	memset(&ofn, 0, sizeof(ofn));
	memset(szFile, 0, sizeof(WCHAR) * 512);
	memset(szFileTitle, 0, sizeof(WCHAR) * 512);

	ofn.lStructSize = sizeof(ofn);
	// ofn.hwndOwner = hWnd;
	// ofn.hInstance = yMain->m_hInst;
	ofn.lpstrFilter = L"All File\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(WCHAR) * 512;
	ofn.lpstrFileTitle = szFileTitle;
	ofn.nMaxFileTitle = sizeof(WCHAR) * 512;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_EXPLORER;

	// 按下确定按钮
	BOOL ok = GetOpenFileName(&ofn);
	if (ok) {
		HWND hAvatar = GetDlgItem(g_hAvatar, IDC_AVATAR);
		// SendMessage(hAvatar, WM_SETTEXT, NULL, (LPARAM)szFile);
		DWORD threadId;
		char *fileDir = wchar2char(szFile);
		snprintf(GrbBuffer.gitdir, sizeof(GrbBuffer.gitdir), "%s", fileDir);
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)GifRead, &GrbBuffer, 0, &threadId);
		delete[]fileDir;
	}

	delete[]szFile;
	delete[]szFileTitle;
	return 0;
}

int HandleCmdMsg(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId = LOWORD(wParam);

	// 分析菜单选择: 
	switch (wmId)
	{
	case IDC_CONNECT:
		HandleCmdConnect();
		break;
	case IDC_SEND:
		HandleCmdSend();
		break;
	case IDC_LOAD:
		HandleCmdLoad(hWnd, message, wParam, lParam);
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

int HandlePaintMsgCamera()
{
	RECT rctA; // 定义一个RECT结构体，存储窗口的长宽高
	HWND cam = g_hCamera;
	PAINTSTRUCT psCamera;
	::GetWindowRect(cam, &rctA);

	HDC hdcCamera = BeginPaint(cam, &psCamera);
	HBITMAP hBitmap;
	TcpChat::GetInstance()->TransBitMap(hBitmap);
	HDC hdcDesk = GetDC(GetDesktopWindow()); // 得到屏幕的dc    
	HDC hdcCopy = CreateCompatibleDC(hdcDesk); // 
	HGDIOBJ hObj = SelectObject(hdcCopy, hBitmap); // 好像总得这么写。
	BitBlt(hdcCamera, 0, 0, rctA.right - rctA.left, rctA.bottom - rctA.top, hdcCopy, 0, 0, SRCCOPY);
	DeleteObject(hObj);
	DeleteObject(hBitmap);
	DeleteDC(hdcCopy);
	DeleteDC(hdcDesk);
	EndPaint(cam, &psCamera);
	return 0;
}

int HandlePaintMsgAvatar()
{
	static int frameIdx = 0;
	RECT rctA; // 定义一个RECT结构体，存储窗口的长宽高
	HWND avatar = g_hAvatar;
	PAINTSTRUCT psAvatar;
	::GetWindowRect(avatar, &rctA);

	HDC hdcAvatar = BeginPaint(avatar, &psAvatar);
	HBITMAP hBitmap;
	// TcpChat::GetInstance()->TransBitMap(hBitmap);
	hBitmap = CreateBitmap(GrbBuffer.width, GrbBuffer.height, 1, 32, GrbBuffer.frame[frameIdx]);
	frameIdx = (++frameIdx) % (sizeof(GrbBuffer.frame) / sizeof(unsigned char*));
	frameIdx = GrbBuffer.frame[frameIdx] == NULL ? 0 : frameIdx;
	/* OpenClipboard(NULL);
	EmptyClipboard();
	SetClipboardData(CF_BITMAP, hBitmap);
	CloseClipboard(); */
	HDC hdcDesk = GetDC(GetDesktopWindow()); // 得到屏幕的dc    
	HDC hdcCopy = CreateCompatibleDC(hdcDesk); // 
	HGDIOBJ hObj = SelectObject(hdcCopy, hBitmap); // 好像总得这么写。
	BitBlt(hdcAvatar, 0, 0, rctA.right - rctA.left, rctA.bottom - rctA.top, hdcCopy, 0, 0, SRCCOPY);
	DeleteObject(hObj);
	DeleteObject(hBitmap);
	DeleteDC(hdcCopy);
	DeleteDC(hdcDesk);
	EndPaint(avatar, &psAvatar);
	return 0;
}

int HandlePaintMsg(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hWnd, &ps);

#if 1
	HandlePaintMsgCamera();
	HandlePaintMsgAvatar();
#else
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
#endif

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
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | WS_VSCROLL | WS_HSCROLL,
		30, 70, 600, 200,
		hWnd, (HMENU)IDC_RECV_MSG, hInst, NULL);
	g_hSendMsg = CreateWindow(TEXT("edit"), NULL, 
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | WS_VSCROLL | WS_HSCROLL,
		30, 300, 600, 120,
		hWnd, (HMENU)IDC_SEND_MSG, hInst, NULL);

	g_hConnBtn = CreateWindow(TEXT("button"), TEXT("连接"),
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_CENTER,
		650, 10, 80, 40,
		hWnd, (HMENU)IDC_CONNECT, hInst, NULL);
	g_hSendBtn = CreateWindow(TEXT("button"), TEXT("发送"),
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_CENTER,
		650, 330, 80, 40,
		hWnd, (HMENU)IDC_SEND, hInst, NULL);
	g_hLoadBtn = CreateWindow(TEXT("button"), TEXT("浏览"),
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_CENTER,
		650, 330 + 40 + 10, 80, 40,
		hWnd, (HMENU)IDC_LOAD, hInst, NULL);
	g_hCamera = CreateWindow(TEXT("static"), TEXT("视频:"),
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_CENTER,
		710, 70, CAMERA_WIDTH, CAMERA_HEIGHT,
		hWnd, (HMENU)IDC_CAMERA, hInst, NULL);
	g_hAvatar = CreateWindow(TEXT("static"), TEXT("头像:"),
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_CENTER,
		710 + CAMERA_WIDTH - 112, 70 + CAMERA_HEIGHT + 10, 112, 112,
		hWnd, (HMENU)IDC_AVATAR, hInst, NULL);

	SetWindowText(g_hIpAddr, TEXT("127.0.0.1"));
	SetWindowText(g_hPort, TEXT("3000"));

	wchar_t user[MAX_PATH] = {0};
	DWORD userLen = sizeof(user);
	GetUserName(user, &userLen);
	SetWindowText(g_hSendMsg, user);

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
	case WM_ERASEBKGND:
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
