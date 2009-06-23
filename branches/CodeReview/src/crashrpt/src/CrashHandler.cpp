///////////////////////////////////////////////////////////////////////////////
//
//  Module: CrashHandler.cpp
//
//    Desc: See CrashHandler.h
//
// Copyright (c) 2003 Michael Carruth
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CrashHandler.h"
#include "zip.h"
#include "process.h"
#include "Utility.h"
#include "resource.h"
#include <sys/stat.h>
#include "tinyxml.h"

#if _MSC_VER>=1300
#include <rtcapi.h>
#endif

#if _MSC_VER<1400

// Taken from: http://msdn.microsoft.com/en-us/library/s975zw7k(VS.71).aspx
#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

// _ReturnAddress and _AddressOfReturnAddress should be prototyped before use 
EXTERNC void * _AddressOfReturnAddress(void);
EXTERNC void * _ReturnAddress(void);

#endif //_MSC_VER<1400


// This internal structure contains the list of processes 
// that had called crInstall().
struct _crash_handlers
{
  _crash_handlers(){m_bCrashHappened=FALSE;}

  ~_crash_handlers()
  {
    // On destroy, check that client process has called crUninstall().
    ATLASSERT( m_bCrashHappened || (!m_bCrashHappened && m_map.size()==0) );    
  }

  std::map<int, CCrashHandler*> m_map; // <PID, CrashHandler> pairs
  BOOL m_bCrashHappened; // This flag is set by exception handler when crash happens
}
g_CrashHandlers;


LONG WINAPI Win32UnhandledExceptionFilter(PEXCEPTION_POINTERS pExceptionPtrs)
{
  CCrashHandler* pCrashHandler = CCrashHandler::GetCurrentProcessCrashHandler();
  ATLASSERT(pCrashHandler!=NULL);

  if(pCrashHandler!=NULL)
  {
    CR_EXCEPTION_INFO ei;
    memset(&ei, 0, sizeof(CR_EXCEPTION_INFO));
    ei.cb = sizeof(CR_EXCEPTION_INFO);
    ei.exctype = CR_WIN32_UNHANDLED_EXCEPTION;
    ei.pexcptrs = pExceptionPtrs;

    pCrashHandler->GenerateErrorReport(&ei);
  }

  // Terminate program
  exit(1);

  return EXCEPTION_EXECUTE_HANDLER;
}


void __cdecl cpp_terminate_handler()
{
  // Abnormal program termination (terminate() function was called)

  CCrashHandler* pCrashHandler = CCrashHandler::GetCurrentProcessCrashHandler();
  ATLASSERT(pCrashHandler!=NULL);

  if(pCrashHandler!=NULL)
  {    
    // Get exception pointers
    EXCEPTION_POINTERS* excptrs = NULL;
    pCrashHandler->GetExceptionPointers(&excptrs);

    // Fill in the exception info
    CR_EXCEPTION_INFO ei;
    memset(&ei, 0, sizeof(CR_EXCEPTION_INFO));
    ei.cb = sizeof(CR_EXCEPTION_INFO);
    ei.exctype = CR_CPP_TERMINATE_CALL;
    ei.pexcptrs = excptrs;    

    pCrashHandler->GenerateErrorReport(&ei);
  }

  // Terminate program
  exit(1); 
}

void __cdecl cpp_unexp_handler()
{
  // Unexpected error (unexpected() function was called)
  
  CCrashHandler* pCrashHandler = CCrashHandler::GetCurrentProcessCrashHandler();
  ATLASSERT(pCrashHandler!=NULL);

  if(pCrashHandler!=NULL)
  { 
    // Get exception pointers
    EXCEPTION_POINTERS* excptrs = NULL;
    pCrashHandler->GetExceptionPointers(&excptrs);

    // Fill in the exception info
    CR_EXCEPTION_INFO ei;
    memset(&ei, 0, sizeof(CR_EXCEPTION_INFO));
    ei.cb = sizeof(CR_EXCEPTION_INFO);
    ei.exctype = CR_CPP_UNEXPECTED_CALL;
    ei.pexcptrs = excptrs;    

    pCrashHandler->GenerateErrorReport(&ei);
  }

  // Terminate program
  exit(1); 
}


void __cdecl cpp_purecall_handler()
{
  // Pure virtual function call
    
  CCrashHandler* pCrashHandler = CCrashHandler::GetCurrentProcessCrashHandler();
  ATLASSERT(pCrashHandler!=NULL);

  if(pCrashHandler!=NULL)
  {    
    // Get exception pointers
    EXCEPTION_POINTERS* excptrs = NULL;
    pCrashHandler->GetExceptionPointers(&excptrs);

    // Fill in the exception info
    CR_EXCEPTION_INFO ei;
    memset(&ei, 0, sizeof(CR_EXCEPTION_INFO));
    ei.cb = sizeof(CR_EXCEPTION_INFO);
    ei.exctype = CR_CPP_PURE_CALL;
    ei.pexcptrs = excptrs;    

    pCrashHandler->GenerateErrorReport(&ei);
  }

  // Terminate program
  exit(1); 
}

#if _MSCVER<1400
void __cdecl cpp_security_handler(int code, void *x)
{
  // Security error (buffer overrun).
  
  CCrashHandler* pCrashHandler = CCrashHandler::GetCurrentProcessCrashHandler();
  ATLASSERT(pCrashHandler!=NULL);

  if(pCrashHandler!=NULL)
  {    
    // Fill in the exception info
    CR_EXCEPTION_INFO ei;
    memset(&ei, 0, sizeof(CR_EXCEPTION_INFO));
    ei.cb = sizeof(CR_EXCEPTION_INFO);
    ei.exctype = CR_CPP_SECURITY_ERROR;
    ei.pexcptrs = NULL;    

    pCrashHandler->GenerateErrorReport(&ei);
  }

  exit(1); // Terminate program 
}
#endif //_MSCVER<1400

#if _MSC_VER>=1300
void __cdecl cpp_invalid_parameter_handler(
  const wchar_t* expression, 
  const wchar_t* function, 
  const wchar_t* file, 
  unsigned int line, 
  uintptr_t pReserved)
 {
   // Invalid parameter exception
   
   CCrashHandler* pCrashHandler = CCrashHandler::GetCurrentProcessCrashHandler();
   ATLASSERT(pCrashHandler!=NULL);

  if(pCrashHandler!=NULL)
  { 
    // Get exception pointers
    EXCEPTION_POINTERS* excptrs = NULL;
    pCrashHandler->GetExceptionPointers(&excptrs);

    // Fill in the exception info
    CR_EXCEPTION_INFO ei;
    memset(&ei, 0, sizeof(CR_EXCEPTION_INFO));
    ei.cb = sizeof(CR_EXCEPTION_INFO);
    ei.exctype = CR_CPP_INVALID_PARAMETER;
    ei.pexcptrs = excptrs;
    ei.expression = expression;
    ei.function = function;
    ei.file = file;
    ei.line = line;    

    pCrashHandler->GenerateErrorReport(&ei);
  }

   exit(1); // Terminate program
 }

int __cdecl cpp_new_handler(size_t)
{
  // 'new' operator memory allocation exception
   
  CCrashHandler* pCrashHandler = CCrashHandler::GetCurrentProcessCrashHandler();
  ATLASSERT(pCrashHandler!=NULL);

  if(pCrashHandler!=NULL)
  { 
    // Although it's not good to allocate memory here, we'll try...
    // Get exception pointers
    EXCEPTION_POINTERS* excptrs = NULL;
    pCrashHandler->GetExceptionPointers(&excptrs);

    // Fill in the exception info
    CR_EXCEPTION_INFO ei;
    memset(&ei, 0, sizeof(CR_EXCEPTION_INFO));
    ei.cb = sizeof(CR_EXCEPTION_INFO);
    ei.exctype = CR_CPP_NEW_OPERATOR_ERROR;
    ei.pexcptrs = NULL;    

    pCrashHandler->GenerateErrorReport(&ei);
  }

   exit(1); // Terminate program
}
#endif //_MSC_VER>=1300

void cpp_sigabrt_handler(int)
{
  // Caught SIGABRT C++ signal

  CCrashHandler* pCrashHandler = CCrashHandler::GetCurrentProcessCrashHandler();
  ATLASSERT(pCrashHandler!=NULL);

  if(pCrashHandler!=NULL)
  { 
    // Get exception pointers
    EXCEPTION_POINTERS* excptrs = NULL;
    pCrashHandler->GetExceptionPointers(&excptrs);

    // Fill in the exception info
    CR_EXCEPTION_INFO ei;
    memset(&ei, 0, sizeof(CR_EXCEPTION_INFO));
    ei.cb = sizeof(CR_EXCEPTION_INFO);
    ei.exctype = CR_CPP_SIGABRT;
    ei.pexcptrs = excptrs;    
    

    pCrashHandler->GenerateErrorReport(&ei);
  }
 
  // Terminate program
  exit(1);
}

void cpp_sigfpe_handler(int /*code*/, int subcode)
{
  // Floating point exception (SIGFPE)
 
  CCrashHandler* pCrashHandler = CCrashHandler::GetCurrentProcessCrashHandler();
  ATLASSERT(pCrashHandler!=NULL);

  if(pCrashHandler!=NULL)
  {     
    // Get exception pointers
    EXCEPTION_POINTERS* excptrs = NULL;
    if(_pxcptinfoptrs!=NULL)
      excptrs = (PEXCEPTION_POINTERS)_pxcptinfoptrs;
    else
      pCrashHandler->GetExceptionPointers(&excptrs);

    // Fill in the exception info
    CR_EXCEPTION_INFO ei;
    memset(&ei, 0, sizeof(CR_EXCEPTION_INFO));
    ei.cb = sizeof(CR_EXCEPTION_INFO);
    ei.exctype = CR_CPP_SIGFPE;
    ei.pexcptrs = excptrs;        
    ei.fpe_subcode = subcode;

    pCrashHandler->GenerateErrorReport(&ei);
  }

  // Terminate program
  exit(1);
}

void cpp_sigill_handler(int)
{
  // Illegal instruction (SIGILL)

  CCrashHandler* pCrashHandler = CCrashHandler::GetCurrentProcessCrashHandler();
  ATLASSERT(pCrashHandler!=NULL);

  if(pCrashHandler!=NULL)
  {    
    // Get exception pointers
    EXCEPTION_POINTERS* excptrs = NULL;
    pCrashHandler->GetExceptionPointers(&excptrs);

    // Fill in the exception info
    CR_EXCEPTION_INFO ei;
    memset(&ei, 0, sizeof(CR_EXCEPTION_INFO));
    ei.cb = sizeof(CR_EXCEPTION_INFO);
    ei.exctype = CR_CPP_SIGILL;
    ei.pexcptrs = excptrs;        

    pCrashHandler->GenerateErrorReport(&ei);
  }

  // Terminate program
  exit(1);
}

void cpp_sigint_handler(int)
{
  // Interruption (SIGINT)

  CCrashHandler* pCrashHandler = CCrashHandler::GetCurrentProcessCrashHandler();
  ATLASSERT(pCrashHandler!=NULL);

  if(pCrashHandler!=NULL)
  { 
    // Get exception pointers
    EXCEPTION_POINTERS* excptrs = NULL;
    pCrashHandler->GetExceptionPointers(&excptrs);

    // Fill in the exception info
    CR_EXCEPTION_INFO ei;
    memset(&ei, 0, sizeof(CR_EXCEPTION_INFO));
    ei.cb = sizeof(CR_EXCEPTION_INFO);
    ei.exctype = CR_CPP_SIGINT;
    ei.pexcptrs = excptrs;        

    pCrashHandler->GenerateErrorReport(&ei);
  }

  // Terminate program
  exit(1);
}

void cpp_sigsegv_handler(int)
{
  // Invalid storage access (SIGSEGV)

  CCrashHandler* pCrashHandler = CCrashHandler::GetCurrentProcessCrashHandler();
  ATLASSERT(pCrashHandler!=NULL);
  
  if(pCrashHandler!=NULL)
  {     
    // Get exception pointers
    EXCEPTION_POINTERS* excptrs = NULL;
    if(_pxcptinfoptrs!=NULL)
      excptrs = (PEXCEPTION_POINTERS)_pxcptinfoptrs;
    else
      pCrashHandler->GetExceptionPointers(&excptrs);

    // Fill in exception info
    CR_EXCEPTION_INFO ei;
    memset(&ei, 0, sizeof(CR_EXCEPTION_INFO));
    ei.cb = sizeof(CR_EXCEPTION_INFO);    
    ei.exctype = CR_CPP_SIGSEGV;
    ei.pexcptrs = excptrs;
        
    pCrashHandler->GenerateErrorReport(&ei);
  }

  // Terminate program
  exit(1);
}

void cpp_sigterm_handler(int)
{
  // Termination request (SIGTERM)

  CCrashHandler* pCrashHandler = CCrashHandler::GetCurrentProcessCrashHandler();
  ATLASSERT(pCrashHandler!=NULL);

  if(pCrashHandler!=NULL)
  {    
    // Get exception pointers
    EXCEPTION_POINTERS* excptrs = NULL;
    pCrashHandler->GetExceptionPointers(&excptrs);

    // Fill in the exception info
    CR_EXCEPTION_INFO ei;
    memset(&ei, 0, sizeof(CR_EXCEPTION_INFO));
    ei.cb = sizeof(CR_EXCEPTION_INFO);
    ei.exctype = CR_CPP_SIGTERM;
    ei.pexcptrs = excptrs;        

    pCrashHandler->GenerateErrorReport(&ei);
  }

  // Terminate program
  exit(1);
}

CCrashHandler::CCrashHandler()
{
}

CCrashHandler::~CCrashHandler()
{
  Destroy();
}


int CCrashHandler::Init(
  LPCTSTR lpcszAppName,
  LPCTSTR lpcszAppVersion,
  LPCTSTR lpcszCrashSenderPath,
  LPGETLOGFILE lpfnCallback, 
  LPCTSTR lpcszTo, 
  LPCTSTR lpcszSubject,
  LPCTSTR lpcszUrl,
  UINT (*puPriorities)[3])
{ 
  crSetErrorMsg(_T("Unspecified error."));

  m_sAppName = lpcszAppName;

  if(m_sAppName.IsEmpty())
    m_sAppName = CUtility::getAppName();

  m_sAppVersion = lpcszAppVersion;
    
  // Get handle to the EXE module used to create this process
  HMODULE hExeModule = GetModuleHandle(NULL);
  if(hExeModule==NULL)
  {
    ATLASSERT(hExeModule!=NULL);
    crSetErrorMsg(_T("Couldn't get module handle for the executable."));
    return 1;
  }

  TCHAR szExeName[_MAX_PATH];
  DWORD dwLength = GetModuleFileName(hExeModule, szExeName, _MAX_PATH);  
  if(dwLength==0)
  {
    // Couldn't get the name of EXE that was used to create current process
    ATLASSERT(0);
    crSetErrorMsg(_T("Couldn't get the name of EXE that was used to create current process."));
    return 2;
  }  

  m_sImageName = CString(szExeName, dwLength);

  CString sCrashRptName;

#ifdef _DEBUG
  sCrashRptName = _T("CrashRptd.dll");
#else
  sCrashRptName = _T("CrashRpt.dll");
#endif //_DEBUG

  // Get handle to the CrashRpt module that is loaded by current process
  HMODULE hCrashRptModule = GetModuleHandle(sCrashRptName);
  if(hCrashRptModule==NULL)
  {
    ATLASSERT(hCrashRptModule!=NULL);
    crSetErrorMsg(_T("Couldn't get handle to CrashRpt.dll."));
    return 3;
  }  
  
  if(lpcszCrashSenderPath==NULL)
  {
    // By default assume that CrashSender.exe is located in the same dir as CrashRpt.dll
    m_sPathToCrashSender = CUtility::GetModulePath(hCrashRptModule);   
  }
  else
    m_sPathToCrashSender = CString(lpcszCrashSenderPath);    

  CString sCrashSenderName;

#ifdef _DEBUG
  sCrashSenderName = _T("CrashSenderd.exe");
#else
  sCrashSenderName = _T("CrashSender.exe");
#endif //_DEBUG

  if(m_sPathToCrashSender.Right(1)!='\\')
      m_sPathToCrashSender+="\\";
  m_sPathToCrashSender+=sCrashSenderName;   

  HANDLE hFile = CreateFile(m_sPathToCrashSender, FILE_GENERIC_READ, 
    FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
  
  if(hFile==INVALID_HANDLE_VALUE)
  {
    ATLASSERT(hFile!=INVALID_HANDLE_VALUE);
    crSetErrorMsg(_T("Couldn't locate CrashSender.exe in specified directory."));
    return 3; // CrashSender not found!
  }

  CloseHandle(hFile);

  // Generate unique GUID for this crash report.
  if(0!=CUtility::GenerateGUID(m_sCrashGUID))
  {
    ATLASSERT(0);
    crSetErrorMsg(_T("Couldn't generate crash GUID."));
    return 4; 
  }

  // Get operating system friendly name.
  if(0!=CUtility::GetOSFriendlyName(m_sOSName))
  {
    ATLASSERT(0);
    crSetErrorMsg(_T("Couldn't get operating system's friendly name."));
    return 5; 
  }

  // Create %LOCAL_APPDATA%\CrashRpt\UnsavedCrashReports folder.
  CString sLocalAppDataFolder;

  DWORD dwCSIDL;
#if _MSC_VER<1300
  dwCSIDL = CSIDL_APPDATA;
#else
  dwCSIDL = CSIDL_LOCAL_APPDATA;
#endif 

  CUtility::GetSpecialFolder(dwCSIDL, sLocalAppDataFolder);
  if(sLocalAppDataFolder.Right(1)!='\\')
    sLocalAppDataFolder += _T("\\");
  
  CString sCrashRptFolder = sLocalAppDataFolder+_T("CrashRpt");
  if(FALSE==CreateDirectory(sCrashRptFolder, NULL) && GetLastError()!=ERROR_ALREADY_EXISTS)
  {
    ATLASSERT(0);
    crSetErrorMsg(_T("Couldn't create CrashRpt directory."));
    return 6; 
  }

  CString sUnsentCrashReportsFolder = sCrashRptFolder+_T("\\UnsentCrashReports");
  if(FALSE==CreateDirectory(sUnsentCrashReportsFolder, NULL) && GetLastError()!=ERROR_ALREADY_EXISTS)
  {
    ATLASSERT(0);
    crSetErrorMsg(_T("Couldn't create UnsentCrashReports directory."));
    return 7; 
  }

  m_sUnsentCrashReportsFolder = sUnsentCrashReportsFolder;

  if(lpcszUrl!=NULL)
    m_sUrl = CString(lpcszUrl);

  if(puPriorities!=NULL)
    memcpy(&m_uPriorities, puPriorities, 3*sizeof(UINT));
  else
    memset(&m_uPriorities, 0, 3*sizeof(UINT));
  
  m_lpfnCallback = NULL;
  m_oldFilter    = NULL;

  // save user supplied callback
  if (lpfnCallback)
    m_lpfnCallback = lpfnCallback;

  // add this filter in the exception callback chain
  m_oldFilter = SetUnhandledExceptionFilter(Win32UnhandledExceptionFilter);

  // Set C++ exception handlers
  InitPrevCPPExceptionHandlerPointers();
   
  int nSetProcessHandlers = SetProcessCPPExceptionHandlers();   
  if(nSetProcessHandlers!=0)
  {
    ATLASSERT(nSetProcessHandlers==0);
    crSetErrorMsg(_T("Couldn't set C++ exception handlers for current process."));
    return 4;
  }

  int nSetThreadHandlers = SetThreadCPPExceptionHandlers();
  if(nSetThreadHandlers!=0)
  {
    ATLASSERT(nSetThreadHandlers==0);
    crSetErrorMsg(_T("Couldn't set C++ exception handlers for main execution thread."));
    return 5;
  }

  // attach this handler with this process
  m_pid = _getpid();
  g_CrashHandlers.m_map[m_pid] =  this;
   
  // save optional email info
  m_sTo = lpcszTo;
  m_sSubject = lpcszSubject;

  // OK.
  crSetErrorMsg(_T("Success."));
  return 0;
}

int CCrashHandler::Destroy()
{
  crSetErrorMsg(_T("Unspecified error."));

  // Reset exception callback
  if (m_oldFilter)
    SetUnhandledExceptionFilter(m_oldFilter);

  m_oldFilter = NULL;

  // All installed per-thread C++ exception handlers should be uninstalled 
  // using crUninstallFromCurrentThread() before calling Destroy()
  
  ATLASSERT(m_ThreadExceptionHandlers.size()==0);      
  m_ThreadExceptionHandlers.clear();

  std::map<int, CCrashHandler*>::iterator it = g_CrashHandlers.m_map.find(m_pid);
  if(it==g_CrashHandlers.m_map.end())
  {
    ATLASSERT(0); // No such crash handler list entry?
    return 1;
  }

  g_CrashHandlers.m_map.erase(it);

  // OK.
  crSetErrorMsg(_T("Success."));
  return 0;
}


// Sets internal pointers to previously used exception handlers to NULL
void CCrashHandler::InitPrevCPPExceptionHandlerPointers()
{

#if _MSC_VER>=1300
  m_prevPurec = NULL;
  m_prevNewHandler = NULL;
#endif

#if _MSC_VER>=1300 && _MSC_VER<1400    
   m_prevSec = NULL;
#endif

#if _MSC_VER>=1400
  m_prevInvpar = NULL;
#endif  

  m_prevSigABRT = NULL;  
  m_prevSigINT = NULL;  
  m_prevSigTERM = NULL;  
}

CCrashHandler* CCrashHandler::GetCurrentProcessCrashHandler()
{
  int pid = _getpid();
  std::map<int, CCrashHandler*>::iterator it = g_CrashHandlers.m_map.find(pid);
  if(it==g_CrashHandlers.m_map.end())
    return NULL; // No handler for calling process.

  return it->second;
}

int CCrashHandler::SetProcessCPPExceptionHandlers()
{
  crSetErrorMsg(_T("Unspecified error."));

  // Set CRT error mode
  // Write exception info to file
  HANDLE hLogFile = NULL;
  hLogFile = CreateFile(_T("crterror.log"), GENERIC_WRITE, FILE_SHARE_WRITE, 
      NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  if(hLogFile==NULL)
  {
    ATLASSERT(hLogFile!=NULL);
    crSetErrorMsg(_T("Couldn't create CRT log file."));
    return 1;
  }

  _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
  _CrtSetReportFile(_CRT_ERROR, hLogFile);


#if _MSC_VER>=1300
  // Catch pure virtual function calls.
  // Because there is one _purecall_handler for the whole process, 
  // calling this function immediately impacts all threads. The last 
  // caller on any thread sets the handler. 
  // http://msdn.microsoft.com/en-us/library/t296ys27.aspx
  m_prevPurec = _set_purecall_handler(cpp_purecall_handler);    

  // Catch new operator memory allocation exceptions
  m_prevNewHandler = _set_new_handler(cpp_new_handler);
#endif

#if _MSC_VER>=1400
  // Catch invalid parameter exceptions.
  m_prevInvpar = _set_invalid_parameter_handler(cpp_invalid_parameter_handler); 
#endif

  
  // Catch buffer overrun exceptions
  // The _set_security_error_handler is deprecated in VC8 C++ run time library
#if _MSC_VER>=1300 && _MSC_VER<1400    
   m_prevSec = _set_security_error_handler(cpp_security_handler);
#endif

   // Set up C++ signal handlers
  
   // Catch an abnormal program termination
   m_prevSigABRT = signal(SIGABRT, cpp_sigabrt_handler);  
   
   // Catch illegal instruction handler
   m_prevSigINT = signal(SIGINT, cpp_sigint_handler);     
   
   // Catch a termination request
   m_prevSigTERM = signal(SIGTERM, cpp_sigterm_handler);      

   crSetErrorMsg(_T("Success."));
   return 0;
}

int CCrashHandler::UnSetProcessCPPExceptionHandlers()
{
  crSetErrorMsg(_T("Unspecified error."));

  // Unset all previously set handlers

#if _MSC_VER>=1300
  if(m_prevPurec!=NULL)
    _set_purecall_handler(m_prevPurec);

  if(m_prevNewHandler!=NULL)
    _set_new_handler(m_prevNewHandler);
#endif

#if _MSC_VER>=1400
  if(m_prevInvpar!=NULL)
    _set_invalid_parameter_handler(m_prevInvpar);
#endif //_MSC_VER>=1400  

#if _MSC_VER>=1300 && _MSC_VER<1400    
  if(m_prevSec!=NULL)
    _set_security_error_handler(m_prevSec);
#endif //_MSC_VER<1400
     
  if(m_prevSigABRT!=NULL)
    signal(SIGABRT, m_prevSigABRT);  
  
  if(m_prevSigINT!=NULL)
    signal(SIGINT, m_prevSigINT);     

  if(m_prevSigTERM!=NULL)
   signal(SIGTERM, m_prevSigTERM);    

  crSetErrorMsg(_T("Success."));
  return 0;
}

// Installs C++ exception handlers that function on per-thread basis
int CCrashHandler::SetThreadCPPExceptionHandlers()
{
  crSetErrorMsg(_T("Unspecified error."));

  DWORD dwThreadId = GetCurrentThreadId();

  std::map<DWORD, _cpp_thread_exception_handlers>::iterator it = 
    m_ThreadExceptionHandlers.find(dwThreadId);

  if(it!=m_ThreadExceptionHandlers.end())
  {
    // handlers are already set for the thread
    ATLASSERT(0);
    crSetErrorMsg(_T("Can't install handlers for current thread twice."));
    return 1; // failed
  }
  
  _cpp_thread_exception_handlers handlers;

  // Catch terminate() calls. 
  // In a multithreaded environment, terminate functions are maintained 
  // separately for each thread. Each new thread needs to install its own 
  // terminate function. Thus, each thread is in charge of its own termination handling.
  // http://msdn.microsoft.com/en-us/library/t6fk7h29.aspx
  handlers.m_prevTerm = set_terminate(cpp_terminate_handler);       

  // Catch unexpected() calls.
  // In a multithreaded environment, unexpected functions are maintained 
  // separately for each thread. Each new thread needs to install its own 
  // unexpected function. Thus, each thread is in charge of its own unexpected handling.
  // http://msdn.microsoft.com/en-us/library/h46t5b69.aspx  
  handlers.m_prevUnexp = set_unexpected(cpp_unexp_handler);    

  // Catch a floating point error
  typedef void (*sigh)(int);
  handlers.m_prevSigFPE = signal(SIGFPE, (sigh)cpp_sigfpe_handler);     

  // Catch an illegal instruction
  handlers.m_prevSigILL = signal(SIGILL, cpp_sigill_handler);     

  // Catch illegal storage access errors
  handlers.m_prevSigSEGV = signal(SIGSEGV, cpp_sigsegv_handler);   

  // Insert the structure to the list of handlers
  std::pair<DWORD, _cpp_thread_exception_handlers> _pair(dwThreadId, handlers);
  m_ThreadExceptionHandlers.insert(_pair);

  // OK.
  crSetErrorMsg(_T("Success."));
  return 0;
}

int CCrashHandler::UnSetThreadCPPExceptionHandlers()
{
  crSetErrorMsg(_T("Unspecified error."));

  DWORD dwThreadId = GetCurrentThreadId();

  std::map<DWORD, _cpp_thread_exception_handlers>::iterator it = 
    m_ThreadExceptionHandlers.find(dwThreadId);

  if(it==m_ThreadExceptionHandlers.end())
  {
    // no such handlers?
    ATLASSERT(0);
    crSetErrorMsg(_T("Crash handler wasn't previously installed for current thread."));
    return 1;
  }
  
  _cpp_thread_exception_handlers* handlers = &(it->second);

  if(handlers->m_prevTerm!=NULL)
    set_terminate(handlers->m_prevTerm);

  if(handlers->m_prevUnexp!=NULL)
    set_unexpected(handlers->m_prevUnexp);

  if(handlers->m_prevSigFPE!=NULL)
    signal(SIGFPE, handlers->m_prevSigFPE);     

  if(handlers->m_prevSigILL!=NULL)
    signal(SIGINT, handlers->m_prevSigILL);     

  if(handlers->m_prevSigSEGV!=NULL)
    signal(SIGSEGV, handlers->m_prevSigSEGV); 

  // Remove from the list
  m_ThreadExceptionHandlers.erase(it);

  // OK.
  crSetErrorMsg(_T("Success."));
  return 0;
}


int CCrashHandler::AddFile(LPCTSTR pszFile, LPCTSTR pszDesc)
{
  crSetErrorMsg(_T("Unspecified error."));

  // make sure the file exist
  struct _stat st;
  int result = _tstat(pszFile, &st);

  if (result!=0)
  {
   ATLASSERT(0);
   crSetErrorMsg(_T("Couldn't stat file. File may not exist."));
   return 1;
  }

  // Add file to file list.
  m_files[pszFile] = pszDesc;

  // OK.
  crSetErrorMsg(_T("Success."));
  return 0;
}

int CCrashHandler::GenerateErrorReport(
  PCR_EXCEPTION_INFO pExceptionInfo)
{  
  crSetErrorMsg(_T("Unspecified error."));

  /* Let client add application-specific files to report via crash callback. */

  if (m_lpfnCallback!=NULL && m_lpfnCallback(NULL)==FALSE)
  {
    crSetErrorMsg(_T("The operation was cancelled by client application."));
    return 1;
  }

  // Notify g_CrashHandlers about crash, to avoid assertion in destructor
  g_CrashHandlers.m_bCrashHappened = TRUE;

  /* Create crash minidump and crash log. */

  CString sTempDir = CUtility::getTempFileName();
  DeleteFile(sTempDir);

  BOOL bCreateDir = CreateDirectory(sTempDir, NULL);  
  if(bCreateDir)
  {
    /* Create crash minidump file */

    CString sFileName;
    sFileName.Format(_T("%s\\crashdump.dmp"), sTempDir);
    int result = CreateMinidump(sFileName, pExceptionInfo->pexcptrs);
    ATLASSERT(result==0);

    if(result==0)
    {
      m_files[sFileName] = CString((LPCTSTR)IDS_CRASH_DUMP);
    }

    /* Create crash report descriptor file in XML format. */
  
    sFileName.Format(_T("%s\\crashrpt.xml"), sTempDir, CUtility::getAppName());
    m_files[sFileName] = CString((LPCTSTR)IDS_CRASH_LOG);
    result = GenerateCrashDescriptorXML(sFileName.GetBuffer(0), pExceptionInfo);
    ATLASSERT(result==0);
    result;
  }
  
  // Save error report as ZIP archive.
  CString sZipName = m_sUnsentCrashReportsFolder+_T("\\")+m_sCrashGUID+_T(".zip");
  int zipped = ZipErrorReport(sZipName);
  if(zipped!=0)
  {
    ATLASSERT(zipped==0);
    
    // Failed to create zip.
    // Try notify user about crash using message box.
    CString szCaption;
    szCaption.Format(_T("%s has stopped working"), CUtility::getAppName());
    CString szMessage;
    szMessage.Format(_T("The program has stopped working due to unexpected error, but CrashRpt wasn't able to create error report.\nPlease report about this issue at http://code.google.com/p/crashrpt/issues/list"));
    MessageBox(NULL, szMessage, szCaption, MB_OK|MB_ICONERROR);    
    return 2;
  }

  // Launch the CrashSender process that would notify user about crash
  // and send the error report by E-mail.
  
  int result = LaunchCrashSender(sZipName);
  if(result!=0)
  {
    ATLASSERT(result==0);
    crSetErrorMsg(_T("Error launching CrashSender.exe"));
    
    // Failed to launch crash sender process.
    // Try notify user about crash using message box.
    CString szCaption;
    szCaption.Format(_T("%s has stopped working"), CUtility::getAppName());
    CString szMessage;
    szMessage.Format(_T("The program has stopped working due to unexpected error, but CrashRpt wasn't able to run CrashSender.exe and send error report.\nPlease report about this issue at http://code.google.com/p/crashrpt/issues/list"));
    MessageBox(NULL, szMessage, szCaption, MB_OK|MB_ICONERROR);    
    return 3;
  }

  crSetErrorMsg(_T("Success."));
  return 0; 
}

int CCrashHandler::GenerateCrashDescriptorXML(LPTSTR pszFileName,          
     PCR_EXCEPTION_INFO pExceptionInfo)
{
  crSetErrorMsg(_T("Unspecified error."));

  USES_CONVERSION;

  TiXmlDocument doc;
  
  // Add root element
  TiXmlElement* root = new TiXmlElement("CrashRpt");
  doc.LinkEndChild(root);

  // Write CrashRpt version
  CString sCrashRptVer;
  sCrashRptVer.Format("%d", CRASHRPT_VER);
  LPSTR lpszCrashRptVer = T2A(sCrashRptVer.GetBuffer(0));
  root->SetAttribute("version", lpszCrashRptVer);

  // Write crash GUID
  TiXmlElement* crash_guid = new TiXmlElement("CrashGUID");
  root->LinkEndChild(crash_guid);
  LPSTR lpszCrashGUID = T2A(m_sCrashGUID.GetBuffer(0));
  TiXmlText* crash_guid_text = new TiXmlText(lpszCrashGUID);
  crash_guid->LinkEndChild(crash_guid_text);

  // Write application name 
  TiXmlElement* app_name = new TiXmlElement("AppName");
  root->LinkEndChild(app_name);
  LPSTR lpszAppName = T2A(m_sAppName.GetBuffer(0));
  TiXmlText* app_name_text = new TiXmlText(lpszAppName);
  app_name->LinkEndChild(app_name_text);

  // Write application version 
  TiXmlElement* app_ver = new TiXmlElement("AppVersion");
  root->LinkEndChild(app_ver);
  LPSTR lpszAppVersion = T2A(m_sAppVersion.GetBuffer(0));
  TiXmlText* app_ver_text = new TiXmlText(lpszAppVersion);
  app_ver->LinkEndChild(app_ver_text);

  // Write EXE image name
  TiXmlElement* image_name = new TiXmlElement("ImageName");
  root->LinkEndChild(image_name);
  LPSTR lpszImageName = T2A(m_sImageName.GetBuffer(0));  
  TiXmlText* image_name_text = new TiXmlText(lpszImageName);
  image_name->LinkEndChild(image_name_text);

  // Write operating system friendly name  
  TiXmlElement* os_name = new TiXmlElement("OperatingSystem");
  root->LinkEndChild(os_name);
  LPSTR lpszOSName = T2A(m_sOSName.GetBuffer(0));
  TiXmlText* os_name_text = new TiXmlText(lpszOSName);
  os_name->LinkEndChild(os_name_text);
  
  // Write system time in UTC format
  CString sSystemTime;
  CUtility::GetSystemTimeUTC(sSystemTime);
  TiXmlElement* sys_time = new TiXmlElement("SystemTimeUTC");
  root->LinkEndChild(sys_time);
  LPSTR lpszSystemTime = T2A(sSystemTime.GetBuffer(0));
  TiXmlText* sys_time_text = new TiXmlText(lpszSystemTime);
  sys_time->LinkEndChild(sys_time_text);
  
  // Write exception type
  CString sExcType;
  sExcType.Format("%d", pExceptionInfo->exctype);  
  TiXmlElement* exc_type = new TiXmlElement("ExceptionType");
  root->LinkEndChild(exc_type);  
  LPSTR lpszExcType = T2A(sExcType.GetBuffer(0));
  TiXmlText* exc_type_text = new TiXmlText(lpszExcType);
  exc_type->LinkEndChild(exc_type_text);

  if(pExceptionInfo->exctype==CR_CPP_SIGFPE)
  {
    // Write FPE exception subcode
    CString sFpeSubcode;
    sFpeSubcode.Format("%d", pExceptionInfo->fpe_subcode);    
    TiXmlElement* fpe_subcode = new TiXmlElement("FPESubcode");
    root->LinkEndChild(fpe_subcode);  
	LPSTR lpszFpeSubcode = T2A(sFpeSubcode.GetBuffer(0));
    TiXmlText* fpe_subcode_text = new TiXmlText(lpszFpeSubcode);
    fpe_subcode->LinkEndChild(fpe_subcode_text);
  }

  if(pExceptionInfo->exctype==CR_CPP_INVALID_PARAMETER)
  {
    if(pExceptionInfo->expression!=NULL)
    {
      // Write expression      
      TiXmlElement* expr = new TiXmlElement("InvParamExpression");
      root->LinkEndChild(expr);  
	  LPSTR lpszExpr = W2A(pExceptionInfo->expression);
      TiXmlText* expr_text = new TiXmlText(lpszExpr);
      expr->LinkEndChild(expr_text);
    }

    if(pExceptionInfo->function!=NULL)
    {
      // Write function      
      TiXmlElement* func = new TiXmlElement("InvParamFunction");
      root->LinkEndChild(func);  
	  LPSTR lpszFunc = W2A(pExceptionInfo->function);
      TiXmlText* func_text = new TiXmlText(lpszFunc);
      func->LinkEndChild(func_text);
    }

    if(pExceptionInfo->file!=NULL)
    {
      // Write file
      TiXmlElement* file = new TiXmlElement("InvParamFile");
      root->LinkEndChild(file);  
	  LPSTR lpszFile = W2A(pExceptionInfo->file);
      TiXmlText* file_text = new TiXmlText(lpszFile);
      file->LinkEndChild(file_text);
    }
    
    // Write line
    CString sLine;
    sLine.Format("%d", pExceptionInfo->line);    
    TiXmlElement* line = new TiXmlElement("InvParamLine");
    root->LinkEndChild(line);  
	LPSTR lpszLine = T2A(sLine.GetBuffer(0));
    TiXmlText* line_text = new TiXmlText(lpszLine);
    line->LinkEndChild(line_text);
  }

  // Write list of files that present in this crash report

  TiXmlElement* file_list = new TiXmlElement("FileList");
  root->LinkEndChild(file_list);      

  TStrStrMap::iterator cur = m_files.begin();
  unsigned i;
  for (i = 0; i < m_files.size(); i++, cur++)
  {    
    CString sFilePath = (*cur).first;

    int pos = -1;
    sFilePath.Replace('/', '\\');
    pos = sFilePath.ReverseFind('\\');
    if(pos!=-1)
      sFilePath = sFilePath.Mid(pos+1);

    CString sFileDesc = (*cur).second;

    TiXmlElement* file_item = new TiXmlElement("FileItem");
    file_list->LinkEndChild(file_item);      

	LPSTR lpszFilePath = T2A(sFilePath.GetBuffer(0));
	LPSTR lpszFileDesc = T2A(sFileDesc.GetBuffer(0));

    file_item->SetAttribute("name", lpszFilePath);    
    file_item->SetAttribute("description", lpszFileDesc);    
  }

  // Save document to file

  LPSTR lpszFileName = T2A(pszFileName);
  bool bSave = doc.SaveFile(lpszFileName);
  if(!bSave)
  {
    ATLASSERT(bSave);
    crSetErrorMsg(_T("Can't save crash log to XML."));
    return 2;
  }

  crSetErrorMsg(_T("Success."));
  return 0;
}

int CCrashHandler::CreateMinidump(LPCTSTR pszFileName, EXCEPTION_POINTERS* pExInfo)
{   
  crSetErrorMsg(_T("Success."));

  // Create the file
  HANDLE hFile = CreateFile(
    pszFileName,
    GENERIC_WRITE,
    0,
    NULL,
    CREATE_ALWAYS,
    FILE_ATTRIBUTE_NORMAL,
    NULL);

  if(hFile==INVALID_HANDLE_VALUE)
  {
    ATLASSERT(hFile!=INVALID_HANDLE_VALUE);
    crSetErrorMsg(_T("Couldn't create file."));
    return 1;
  }

  // Write the minidump to the file
  
  MINIDUMP_EXCEPTION_INFORMATION eInfo;
  eInfo.ThreadId = GetCurrentThreadId();
  eInfo.ExceptionPointers = pExInfo;
  eInfo.ClientPointers = FALSE;

  MINIDUMP_CALLBACK_INFORMATION cbMiniDump;
  cbMiniDump.CallbackRoutine = NULL;
  cbMiniDump.CallbackParam = 0;

  BOOL bWriteDump = MiniDumpWriteDump(
    GetCurrentProcess(),
    GetCurrentProcessId(),
    hFile,
    MiniDumpNormal,
    pExInfo ? &eInfo : NULL,
    NULL,
    &cbMiniDump);
 
  if(!bWriteDump)
  {
    ATLASSERT(bWriteDump);
    crSetErrorMsg(_T("Couldn't write minidump to file."));
    return 2;
  }

  // Close file
  CloseHandle(hFile);

  return 0;
}

int CCrashHandler::ZipErrorReport(CString sFileName)
{
  CString sTempFileName = CUtility::getTempFileName();

  HZIP hz = CreateZip(sFileName, NULL);
  if (hz==NULL)
  {
    crSetErrorMsg(_T("Couldn't create zip archive for the error report."));
    return 1;
  }

  // add report files to zip
  TStrStrMap::iterator cur = m_files.begin();
  unsigned i;
  for (i = 0; i < m_files.size(); i++, cur++)
  {    
    CString szFilePath = (*cur).first;
    int pos = szFilePath.ReverseFind('\\');
    CString szFileName; 
    if(pos>=0) 
      szFileName = szFilePath.Mid(pos+1);
    else
      szFileName = szFilePath;
        
    ZRESULT zr = ZipAdd(hz, szFileName, szFilePath);
    if(zr!=ZR_OK)
    {
      crSetErrorMsg(_T("Couldn't add file to zip archive."));
      CloseZip(hz);
      return 1;
    }
  }

  CloseZip(hz);    
  return 0;
}

int CCrashHandler::LaunchCrashSender(CString sZipName)
{
  crSetErrorMsg(_T("Success."));

  /* Create CrashSender process */

  STARTUPINFO si;
  memset(&si, 0, sizeof(STARTUPINFO));
  si.cb = sizeof(STARTUPINFO);

  PROCESS_INFORMATION pi;
  memset(&pi, 0, sizeof(PROCESS_INFORMATION));  

  BOOL bCreateProcess = CreateProcess(m_sPathToCrashSender, 
    NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
  if(!bCreateProcess)
  {
    ATLASSERT(bCreateProcess);
    crSetErrorMsg(_T("Error crating CrashSender process."));
    return 1;
  }

  /* Connect to the pipe that CrashSender creates. */

  CString sPipeName;
  sPipeName.Format(_T("\\\\.\\pipe\\CrashRpt_%lu"), pi.dwProcessId);

  HANDLE hPipe = INVALID_HANDLE_VALUE;

  int MAX_ATTEMPTS = 120;
  int i;
  for(i=0; i<MAX_ATTEMPTS; i++)
  {
    hPipe = CreateFile(sPipeName, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if(hPipe!=INVALID_HANDLE_VALUE)
      break;
    Sleep(1000);
  }

  if(hPipe==INVALID_HANDLE_VALUE)
  {
    ATLASSERT(hPipe!=INVALID_HANDLE_VALUE);
    crSetErrorMsg(_T("Error connecting to pipe."));
    return 2;
  }

  /* Transfer crash information in XML format */

  CString sCrashInfo;
  sCrashInfo.Format(
    _T("<crashrpt subject=\"%s\" mailto=\"%s\" url=\"%s\" appname=\"%s\" imagename=\"%s\" zipname=\"%s\" http_priority=\"%d\" smtp_priority=\"%d\" mapi_priority=\"%d\" />"), 
    _ReplaceRestrictedXMLCharacters(m_sSubject), 
    _ReplaceRestrictedXMLCharacters(m_sTo),
    _ReplaceRestrictedXMLCharacters(m_sUrl),
    _ReplaceRestrictedXMLCharacters(m_sAppName),
    _ReplaceRestrictedXMLCharacters(m_sImageName),
    _ReplaceRestrictedXMLCharacters(sZipName),
    m_uPriorities[CR_HTTP],
    m_uPriorities[CR_SMTP],
    m_uPriorities[CR_SMAPI]);

  LPSTR lpszCrashInfo =  T2A(sCrashInfo.GetBuffer(0));
  
  DWORD dwBytesWritten = 0;
  BOOL bWrite = WriteFile(hPipe, lpszCrashInfo, strlen(lpszCrashInfo), &dwBytesWritten, NULL);
  
  if(!bWrite || (int)dwBytesWritten == strlen(lpszCrashInfo))
  {
    ATLASSERT(bWrite);
    ATLASSERT((int)dwBytesWritten == strlen(lpszCrashInfo));
    crSetErrorMsg(_T("Error transferring the crash information through the pipe."));
  }

  /* Clean up */

  CloseHandle(hPipe);

  return 0;
}

CString CCrashHandler::_ReplaceRestrictedXMLCharacters(CString sText)
{
  CString sResult;

  sResult = sText;
  sResult.Replace(_T("\""), _T("&quot"));
  sResult.Replace(_T("'"), _T("&apos"));

  return sResult;
}

void CCrashHandler::GetExceptionPointers(EXCEPTION_POINTERS** ppExceptionPointers)
{
  // The following code was taken from VC++ 8.0 CRT (invarg.c: line 104)
  
  EXCEPTION_RECORD ExceptionRecord;
  CONTEXT ContextRecord;
  memset(&ContextRecord, 0, sizeof(CONTEXT));
  
#ifdef _X86_

  __asm {
      mov dword ptr [ContextRecord.Eax], eax
      mov dword ptr [ContextRecord.Ecx], ecx
      mov dword ptr [ContextRecord.Edx], edx
      mov dword ptr [ContextRecord.Ebx], ebx
      mov dword ptr [ContextRecord.Esi], esi
      mov dword ptr [ContextRecord.Edi], edi
      mov word ptr [ContextRecord.SegSs], ss
      mov word ptr [ContextRecord.SegCs], cs
      mov word ptr [ContextRecord.SegDs], ds
      mov word ptr [ContextRecord.SegEs], es
      mov word ptr [ContextRecord.SegFs], fs
      mov word ptr [ContextRecord.SegGs], gs
      pushfd
      pop [ContextRecord.EFlags]
  }

  ContextRecord.ContextFlags = CONTEXT_CONTROL;
#pragma warning(push)
#pragma warning(disable:4311)
  ContextRecord.Eip = (ULONG)_ReturnAddress();
//#if _MSC_VER>=1400
  ContextRecord.Esp = (ULONG)_AddressOfReturnAddress();
//#endif // TODO: make this work in VS.NET
#pragma warning(pop)
  ContextRecord.Ebp = *((ULONG *)_AddressOfReturnAddress()-1);


#elif defined (_IA64_) || defined (_AMD64_)

  /* Need to fill up the Context in IA64 and AMD64. */
  RtlCaptureContext(&ContextRecord);

#else  /* defined (_IA64_) || defined (_AMD64_) */

  ZeroMemory(&ContextRecord, sizeof(ContextRecord));

#endif  /* defined (_IA64_) || defined (_AMD64_) */

  ZeroMemory(&ExceptionRecord, sizeof(EXCEPTION_RECORD));

  ExceptionRecord.ExceptionCode = 0;
  ExceptionRecord.ExceptionAddress = _ReturnAddress();

  ///
  
  EXCEPTION_RECORD* pExceptionRecord = new EXCEPTION_RECORD;
  memcpy(pExceptionRecord, &ExceptionRecord, sizeof(EXCEPTION_RECORD));
  CONTEXT* pContextRecord = new CONTEXT;
  memcpy(pContextRecord, &ContextRecord, sizeof(CONTEXT));

  *ppExceptionPointers = new EXCEPTION_POINTERS;
  (*ppExceptionPointers)->ExceptionRecord = pExceptionRecord;
  (*ppExceptionPointers)->ContextRecord = pContextRecord;  
}



