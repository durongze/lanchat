// Win32Service.cpp : 定义控制台应用程序的入口点。
//Author: Rlz Studio.

#include "stdafx.h"
#include "Win32Service.h"
#include "shlwapi.h"

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "Advapi32.lib")

#define  SERVICE_ACT_NAME _T("DrzTestAct")
#define  SERVICE_DSP_NAME _T("DrzTestDsp")
#define	 SERVICE_LOG_NAME _T("DrzLog.log")

#define DrzWriteLog(x, ...)  do {          \
	TCHAR tcInMsg[MAX_PATH];               \
	ZeroMemory(tcInMsg, sizeof(tcInMsg));  \
	_stprintf_s(tcInMsg, _countof(tcInMsg), SERVICE_DSP_NAME _T(x), __VA_ARGS__); \
	WriteLog(tcLogPath, tcInMsg);          \
} while(1)

CRITICAL_SECTION m_Critical;
TCHAR tcLogPath[MAX_PATH];
TCHAR tcSrvName[MAX_PATH];
TCHAR tcSrvPath[MAX_PATH];

SERVICE_STATUS_HANDLE   hServiceStatusHandle;
SERVICE_STATUS          ServiceStatus;
SERVICE_TABLE_ENTRY		lpServiceStartTable[] =
{
	{tcSrvName, ServiceMain},
	{NULL, NULL}
};

TCHAR* GetLogPath(_TCHAR *Program)
{
	if (Program != NULL) {
		ZeroMemory(tcLogPath, sizeof(tcLogPath));
		_tcscpy_s(tcLogPath, _countof(tcLogPath), Program);
		::PathRemoveFileSpec(tcLogPath);
		_tcscat_s(tcLogPath, _countof(tcLogPath), _T("\\"));
		_tcscat_s(tcLogPath, _countof(tcLogPath), SERVICE_LOG_NAME);
	}
	else {
		if (::GetModuleFileName(NULL, tcLogPath, _countof(tcLogPath)))
		{
			::PathRemoveFileSpec(tcLogPath);
			_tcscat_s(tcLogPath, _countof(tcLogPath), _T("\\"));
			_tcscat_s(tcLogPath, _countof(tcLogPath), SERVICE_LOG_NAME);
		}
	}
	return tcLogPath;
}

int _tmain(int argc, _TCHAR* argv[])
{
	TCHAR tcCmdLine[MAX_PATH];
	ZeroMemory(tcCmdLine, sizeof(tcCmdLine));
	if(argc >= 2)
		_tcscpy_s(tcCmdLine, _countof(tcCmdLine), argv[1]);

	InitializeCriticalSection(&m_Critical);

	GetLogPath(argv[0]);

	_tcscpy_s(tcSrvPath, _countof(tcSrvPath), argv[0]);
	if (0 == _tcsncmp(tcCmdLine, _T("-install"), sizeof(_T("-install"))))//Service Install Stuff
	{
		DrzWriteLog("Service Install Start!\n");
		InstallService(tcSrvPath, SERVICE_ACT_NAME);
	}
	else if (0 == _tcsncmp(tcCmdLine, _T("-kill"), sizeof(_T("-kill"))))//Service Kill Stuff
	{
		DrzWriteLog("Service kill Start!\n");
		KillService(SERVICE_ACT_NAME);
	}
	else if (0 == _tcsncmp(tcCmdLine, _T("-uninstall"), sizeof(_T("-uninstall"))))//Service Uninstall Stuff
	{
		DrzWriteLog("Service uninstall Start!\n");
		UnInstallService(SERVICE_ACT_NAME);
	}
	else if (0 == _tcsncmp(tcCmdLine, _T("-start"), sizeof(_T("-start"))))//Service Start Stuff
	{
		DrzWriteLog("Service start Start!\n");
		RunService(SERVICE_ACT_NAME);
	}
	else
		ExecuteSubProcess();

	return 0;
}

void InstallService(TCHAR* pSrvPath, TCHAR* pSrvName)
{  
	SC_HANDLE schSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_CREATE_SERVICE); 
	if (schSCManager == 0) 
	{
		DrzWriteLog("@FUNC--OpenSCManager failed, error code = %d.\n", GetLastError());
	}
	else
	{
		SC_HANDLE schService = CreateService
			( 
			schSCManager,						/*1 SCManager database      */ 
			pSrvName,							/*2 name of service         */ 
			SERVICE_DSP_NAME,					/*3 service name to display */ 
			SERVICE_ALL_ACCESS,					/*4 desired access          */ 
			SERVICE_WIN32_OWN_PROCESS,			/*5 service type            */ 
			SERVICE_AUTO_START,      			/*6 start type              */ 
			SERVICE_ERROR_NORMAL,      			/*7 error control type      */ 
			pSrvPath,							/*8 service's binary        */ 
			NULL,                      			/*9 no load ordering group  */ 
			NULL,                      			/*10 no tag identifier       */ 
			NULL,                      			/*11 no dependencies         */ 
			NULL,								/*12 LocalSystem account     */ 
			NULL								/*13 no password             */
			);
		if (schService==0) 
		{
			DrzWriteLog("@FUNC--CreateService failed, error code = %d.\n", GetLastError());
		}
		else
		{
			DrzWriteLog("Service Install Success!\n");
			CloseServiceHandle(schService); 
		}
		CloseServiceHandle(schSCManager);
	}	
}

BOOL KillService(TCHAR* pSrvName) 
{ 
	// kill service with given name
	SC_HANDLE schSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS); 
	if (schSCManager==0) 
	{
		DrzWriteLog("@FUNC--OpenSCManager failed, error code = %d.\n", GetLastError());
	}
	else
	{
		// open the service
		SC_HANDLE schService = OpenService( schSCManager, pSrvName, SERVICE_ALL_ACCESS);
		if (schService==0) 
		{
			DrzWriteLog("@FUNC--OpenService failed, error code = %d.\n", GetLastError());
		}
		else
		{
			// call ControlService to kill the given service
			SERVICE_STATUS status;
			if(ControlService(schService,SERVICE_CONTROL_STOP,&status))
			{
				CloseServiceHandle(schService); 
				CloseServiceHandle(schSCManager); 
				return TRUE;
			}
			else
			{
				DrzWriteLog("@FUNC--ControlService failed, error code = %d.\n", GetLastError());
			}
			CloseServiceHandle(schService); 
		}
		CloseServiceHandle(schSCManager); 
	}
	return FALSE;
}

void UnInstallService(TCHAR* pSrvName)
{
	SC_HANDLE schSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS); 
	if (schSCManager==0) 
	{
		DrzWriteLog("@FUNC--OpenSCManager failed, error code = %d.\n", GetLastError());
	}
	else
	{
		SC_HANDLE schService = OpenService( schSCManager, pSrvName, SERVICE_ALL_ACCESS);
		if (schService==0) 
		{
			DrzWriteLog("@FUNC--OpenService failed, error code = %d.\n", GetLastError());
		}
		else
		{
			if(!DeleteService(schService)) 
			{
				DrzWriteLog("DeleteService failed, error code = %d.\n", GetLastError());
			}
			else 
			{
				DrzWriteLog("DeleteService Success!\n");
			}
			CloseServiceHandle(schService); 
		}
		CloseServiceHandle(schSCManager);	
	}
	DeleteFile(tcLogPath);
}

BOOL RunService(TCHAR* pSrvName) 
{ 
	// run service with given name
	SC_HANDLE schSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS); 
	if (schSCManager==0) 
	{
		DrzWriteLog("@FUNC--OpenSCManager failed, error code = %d\n", GetLastError());
	}
	else
	{
		// open the service
		SC_HANDLE schService = OpenService( schSCManager, pSrvName, SERVICE_ALL_ACCESS);
		if (schService==0) 
		{
			DrzWriteLog("@FUNC--OpenService failed, error code = %d\n", GetLastError());
		}
		else
		{
			// call StartService to run the service
			if(StartService(schService, 0, (const TCHAR**)NULL))
			{
				DrzWriteLog("@FUNC--StartService success, error code = %d\n", GetLastError());
				CloseServiceHandle(schService); 
				CloseServiceHandle(schSCManager); 
				return TRUE;
			}
			else
			{
				DrzWriteLog("@FUNC--StartService failed, error code = %d\n", GetLastError());
			}
			CloseServiceHandle(schService); 
		}
		CloseServiceHandle(schSCManager); 
	}
	return FALSE;
}

void ExecuteSubProcess()
{
	if(!StartServiceCtrlDispatcher(lpServiceStartTable))
	{
		DrzWriteLog("@FUNC--StartServiceCtrlDispatcher failed, error code = %d\n", GetLastError());
	}
}

void WINAPI ServiceMain(DWORD dwArgc, LPTSTR *lpszArgv)
{
	DWORD   status = 0; 
	DWORD   specificError = 0xfffffff; 

	ServiceStatus.dwServiceType        = SERVICE_WIN32; 
	ServiceStatus.dwCurrentState       = SERVICE_START_PENDING; 
	ServiceStatus.dwControlsAccepted   = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_PAUSE_CONTINUE; 
	ServiceStatus.dwWin32ExitCode      = 0; 
	ServiceStatus.dwServiceSpecificExitCode = 0; 
	ServiceStatus.dwCheckPoint         = 0; 
	ServiceStatus.dwWaitHint           = 0; 

	hServiceStatusHandle = RegisterServiceCtrlHandler(tcSrvName, ServiceHandler); 
	if (hServiceStatusHandle==0) 
	{
		DrzWriteLog("@FUNC--RegisterServiceCtrlHandler failed, error code = %d\n", GetLastError());
		return; 
	} 

	// Initialization complete - report running status 
	ServiceStatus.dwCurrentState       = SERVICE_RUNNING; 
	ServiceStatus.dwCheckPoint         = 0; 
	ServiceStatus.dwWaitHint           = 0;  
	if(!SetServiceStatus(hServiceStatusHandle, &ServiceStatus)) 
	{ 
		DrzWriteLog("@FUNC--SetServiceStatus failed, error code = %d\n", GetLastError());
	} 
	//
	StartServiceProcess();
}


void WINAPI ServiceHandler(DWORD fdwControl)
{
	switch(fdwControl) 
	{
	case SERVICE_CONTROL_STOP:
		WriteLog(tcLogPath, _T("Service Control Stop!\n"));
	case SERVICE_CONTROL_SHUTDOWN:
		ServiceStatus.dwWin32ExitCode = 0; 
		ServiceStatus.dwCurrentState  = SERVICE_STOPPED; 
		ServiceStatus.dwCheckPoint    = 0; 
		ServiceStatus.dwWaitHint      = 0;
		break; 
	case SERVICE_CONTROL_PAUSE:
		ServiceStatus.dwCurrentState = SERVICE_PAUSED; 
		break;
	case SERVICE_CONTROL_CONTINUE:
		ServiceStatus.dwCurrentState = SERVICE_RUNNING; 
		break;
	case SERVICE_CONTROL_INTERROGATE:
		break;
	default:
		;
	}
	if (!SetServiceStatus(hServiceStatusHandle,  &ServiceStatus)) 
	{ 
		DrzWriteLog("@FUNC--SetServiceStatus failed, error code = %d\n", GetLastError());
	} 
}

void StartServiceProcess()
{
	while (1)
	{
		::WinExec(("cmd.exe"), SW_HIDE);
		Sleep(10*1000);
	}
}

void WriteLog(TCHAR *pLogPath, TCHAR *pWriteMsg)
{
	::EnterCriticalSection(&m_Critical);
	if (0 == _tcscmp(pLogPath, _T("")))
	{
		pLogPath = GetLogPath(NULL);
	}
	try
	{
		SYSTEMTIME CrtTime;
		::GetLocalTime(&CrtTime);
		FILE *pLogFile = _tfopen(pLogPath, _T("a"));
		if (pLogFile != NULL)
		{
			_ftprintf(pLogFile, _T("-----------------------------------------------\n%02d/%02d/%04d, %02d:%02d:%02d\n    %s\n"), CrtTime.wMonth, CrtTime.wDay, CrtTime.wYear, CrtTime.wHour, CrtTime.wMinute, CrtTime.wSecond, pWriteMsg);
			fclose(pLogFile);
		}
	}
	catch (...)
	{
		
	}
	::LeaveCriticalSection(&m_Critical);
}