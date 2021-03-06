// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#pragma once

// Change these values to use different versions

#if _MSC_VER<1300
#define WINVER		0x0400
#define _WIN32_WINNT	0x0400
#define _WIN32_IE	0x0400
#else
#define WINVER		0x0500
//#define _WIN32_WINNT	0x0400
#define _WIN32_IE	0x0500
#endif

#define _RICHEDIT_VER	0x0100

#if _MSC_VER>=1300
#include <atlstr.h>
#include <atltypes.h>
#endif

#include <atlbase.h>
#include <atlapp.h>

extern CAppModule _Module;

#include <atlwin.h>

#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>

#if _MSC_VER<1300
#define _WTL_USE_CSTRING
#include <atlmisc.h>
#endif


#if _MSC_VER<1400
#define _TCSCPY_S(strDestination, numberOfElements, strSource) _tcscpy(strDestination, strSource)
#define _TCSNCPY_S(strDest, sizeInBytes, strSource, count) _tcsncpy(strDest, strSource, count)
#define STRCPY_S(strDestination, numberOfElements, strSource) strcpy(strDestination, strSource)
#else
#define _TCSCPY_S(strDestination, numberOfElements, strSource) _tcscpy_s(strDestination, numberOfElements, strSource)
#define _TCSNCPY_S(strDest, sizeInBytes, strSource, count) _tcsncpy_s(strDest, sizeInBytes, strSource, count)
#define STRCPY_S(strDestination, numberOfElements, strSource) strcpy_s(strDestination, numberOfElements, strSource)
#endif

#if _MSC_VER>=1400
#if defined _M_IX86
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif 