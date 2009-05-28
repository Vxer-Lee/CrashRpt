// crashcon.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include <assert.h>
#include <process.h>
#include "CrashRpt.h"

LPVOID lpvState = NULL;

int filter(unsigned int code, struct _EXCEPTION_POINTERS* ep)
{
  code; // this is to avoid  C4100 unreferenced formal parameter warning
  GenerateErrorReport(lpvState, ep);
  return EXCEPTION_EXECUTE_HANDLER;
}

int main(int argc, char* argv[])
{
  argc; // this is to avoid C4100 unreferenced formal parameter warning
  argv; // this is to avoid C4100 unreferenced formal parameter warning
  
  // Install crash reporting

#ifdef TEST_DEPRECATED_FUNCS
  lpvState = Install(NULL, NULL, NULL);
#else
  CR_INSTALL_INFO info;
  memset(&info, 0, sizeof(CR_INSTALL_INFO));
  info.cb = sizeof(CR_INSTALL_INFO);
  info.pszAppName = _T("CrashRpt Console Test");
  info.pszAppVersion = _T("1.0.0");
  info.pszEmailSubject = _T("CrashRpt Console Test 1.0.0 Error Report");
  info.pszEmailTo = _T("zexspectrum_1980.mail.ru");

  int nInstResult = crInstall(&info);
  assert(nInstResult==0);
  nInstResult;
#endif //TEST_DEPRECATED_FUNCS
  
  printf("Press Enter to simulate a null pointer exception or any other key to exit...\n");
  int n = _getch();
  if(n==13)
  {

#ifdef _DEBUG
     __try
     {
        RaiseException(EXCEPTION_ACCESS_VIOLATION, 0, 0, NULL);
     } 
     __except(filter(GetExceptionCode(), GetExceptionInformation()))
     {
     }
#else
     int *p = 0;
     *p = 0;
#endif // _DEBUG
  
  }

#ifdef TEST_DEPRECATED_FUNCS
  Uninstall(lpvState);
#else
  int nUninstRes = crUninstall();
  assert(nUninstRes==0);
  nUninstRes;
#endif //TEST_DEPRECATED_FUNCS

  return 0;
}

