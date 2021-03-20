#ifndef __WIN32_SERVICE_H__
#define __WIN32_SERVICE_H__

void WINAPI ServiceMain(DWORD dwArgc, LPTSTR *lpszArgv);
void WriteLog(TCHAR *pLogPath = NULL, TCHAR *pWriteMsg = NULL);
void InstallService(TCHAR* pSrvPath, TCHAR* pSrvName);
BOOL KillService(TCHAR* pSrvName);
void UnInstallService(TCHAR* pSrvName);
BOOL RunService(TCHAR* pSrvName);
void ExecuteSubProcess();
void WINAPI ServiceHandler(DWORD fdwControl);
void StartServiceProcess();

#endif