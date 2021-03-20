#ifndef __MY_SYS__
#define __MY_SYS__

#ifdef _WIN32

#include <winsock2.h>
#include <ws2tcpip.h>

#ifdef _MSC_VER	/* MSVC */
//#define snprintf _snprintf
#define strcasecmp stricmp
#define strncasecmp strnicmp
//#define vsnprintf _vsnprintf
#endif

#define GetSockError()	WSAGetLastError()
#define SetSockError(e)	WSASetLastError(e)
#define setsockopt(a,b,c,d,e)	(setsockopt)(a,b,c,(const char *)d,(int)e)
#define EWOULDBLOCK	WSAETIMEDOUT	/* we don't use nonblocking, but we do use timeouts */
#define sleep(n)	Sleep(n*1000)
#define msleep(n)	Sleep(n)
#define SET_RCVTIMEO(tv,s)	int tv = s*1000
#else /* !_WIN32 */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/times.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#define GetSockError()	errno
#define SetSockError(e)	errno = e
#undef closesocket
#define closesocket(s)	close(s)
#define msleep(n)	usleep(n*1000)
#define SET_RCVTIMEO(tv,s)	struct timeval tv = {s,0}

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define BYTE unsigned char
#define DWORD unsigned long
#define WORD unsigned short
#define LONG long
#define LPCSTR char*
#define LPVOID void*
#define SOCKET int
#define SOCKADDR sockaddr
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR -1
#define HBITMAP unsigned long


typedef struct {
    DWORD biSize;   
    LONG biWidth;   
    LONG biHeight;   
    WORD biPlanes;   
    WORD biBitCount; 
    DWORD biCompression;   
    DWORD biSizeImage;   
    LONG biXPelsPerMeter;   
    LONG biYPelsPerMeter;   
    DWORD biClrUsed;   
    DWORD biClrImportant; 
}BITMAPINFOHEADER;

typedef struct {
    int     bmType;//必须是BM
    int     bmWidth;//指定位图的宽度（以象素为单位）
    int     bmHeight;//指定位图的高度（以象素为单位）。
    int     bmWidthBytes;//一行几个字节，4位对齐
    BYTE    bmPlanes;//指定目标设备的位面数
    BYTE    bmBitsPixel;//指定每个象素的位数
    LPVOID  bmBits;//图象数据
}BITMAP;

typedef struct {
    char c;
}WSADATA;

typedef struct {
    int x;
    int y;
}POINT;
#define MAKEWORD(x, y)  2
#define WSAStartup(x, y) 0 
#define WSACleanup()

#endif

#endif
