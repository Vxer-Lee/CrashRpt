/************************************************************************************* 
  This file is a part of CrashRpt library.

  CrashRpt is Copyright (c) 2003, Michael Carruth
  All rights reserved.
 
  Redistribution and use in source and binary forms, with or without modification, 
  are permitted provided that the following conditions are met:
 
   * Redistributions of source code must retain the above copyright notice, this 
     list of conditions and the following disclaimer.
 
   * Redistributions in binary form must reproduce the above copyright notice, 
     this list of conditions and the following disclaimer in the documentation 
     and/or other materials provided with the distribution.
 
   * Neither the name of the author nor the names of its contributors 
     may be used to endorse or promote products derived from this software without 
     specific prior written permission.
 

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY 
  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES 
  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
  SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED 
  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR 
  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT 
  OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***************************************************************************************/

// File: CrashInfoReader.cpp
// Description: Retrieves crash information passed from CrashRpt.dll in form of XML files.
// Authors: zexspectrum
// Date: 2010

#include "stdafx.h"
#include "CrashRpt.h"
#include "CrashInfoReader.h"
#include "strconv.h"
#include "tinyxml.h"

// Define global CCrashInfoReader object
CCrashInfoReader g_CrashInfo;

int CCrashInfoReader::Init(CString sCrashInfoFileName)
{ 
  strconv_t strconv;

  TiXmlDocument doc;
  bool bOpen = doc.LoadFile(strconv.t2a(sCrashInfoFileName));
  if(!bOpen)
    return 1;

  TiXmlHandle hRoot = doc.FirstChild("CrashRptInternal");
  if(hRoot.ToElement()==NULL)
    return 1;

  {
    TiXmlHandle hReportFolder = hRoot.FirstChild("ReportFolder");
    if(hReportFolder.FirstChild().ToText()!=NULL)
    {
      const char* szReportFolder = hReportFolder.FirstChild().ToText()->Value();
      if(szReportFolder!=NULL)
        m_sErrorReportDirName = strconv.utf82t(szReportFolder);
    }
  }
  
  {
    TiXmlHandle hCrashGUID = hRoot.FirstChild("CrashGUID");
    if(hCrashGUID.FirstChild().ToText()!=NULL)
    {
      const char* szCrashGUID = hCrashGUID.FirstChild().ToText()->Value();
      if(szCrashGUID!=NULL)
        m_sCrashGUID = strconv.utf82t(szCrashGUID);
    }
  }

  {
    TiXmlHandle hDbgHelpPath = hRoot.FirstChild("DbgHelpPath");
    if(hDbgHelpPath.FirstChild().ToText()!=NULL)
    {
      const char* szDbgHelpPath = hDbgHelpPath.FirstChild().ToText()->Value();
      if(szDbgHelpPath!=NULL)
        m_sDbgHelpPath = strconv.utf82t(szDbgHelpPath);
    }
  }

  {
    TiXmlHandle hMinidumpType = hRoot.FirstChild("MinidumpType");
    if(hMinidumpType.FirstChild().ToText()!=NULL)
    {
      const char* szMinidumpType = hMinidumpType.FirstChild().ToText()->Value();
      if(szMinidumpType!=NULL)
        m_MinidumpType = (MINIDUMP_TYPE)atol(szMinidumpType);
      else
        m_MinidumpType = MiniDumpNormal;
    }
    else
      m_MinidumpType = MiniDumpNormal;
  }

  {
    TiXmlHandle hUrl = hRoot.FirstChild("Url");
    if(hUrl.FirstChild().ToText()!=NULL)
    {
      const char* szUrl = hUrl.FirstChild().ToText()->Value();
      if(szUrl!=NULL)
        m_sUrl = strconv.utf82t(szUrl);
    }
  }

  {
    TiXmlHandle hEmailTo = hRoot.FirstChild("EmailTo");
    if(hEmailTo.FirstChild().ToText()!=NULL)
    {
      const char* szEmailTo = hEmailTo.FirstChild().ToText()->Value();
      if(szEmailTo!=NULL)
        m_sEmailTo = strconv.utf82t(szEmailTo);
    }
  }

  {
    m_nSmtpPort = 25;
    TiXmlHandle hSmtpPort = hRoot.FirstChild("SmtpPort");
    if(hSmtpPort.FirstChild().ToText()!=NULL)
    {
      const char* szSmtpPort = hSmtpPort.FirstChild().ToText()->Value();
      if(szSmtpPort!=NULL)
        m_nSmtpPort = atoi(szSmtpPort);
    }
  }

  {
    TiXmlHandle hEmailSubject = hRoot.FirstChild("EmailSubject");
    if(hEmailSubject.FirstChild().ToText()!=NULL)
    {
      const char* szEmailSubject = hEmailSubject.FirstChild().ToText()->Value();
      if(szEmailSubject!=NULL)
        m_sEmailSubject = strconv.utf82t(szEmailSubject);
    }
  }

  {
    TiXmlHandle hPrivacyPolicyUrl = hRoot.FirstChild("PrivacyPolicyUrl");    
    if(hPrivacyPolicyUrl.FirstChild().ToText()!=NULL)
    {
      const char* szPrivacyPolicyUrl = hPrivacyPolicyUrl.FirstChild().ToText()->Value();
      if(szPrivacyPolicyUrl!=NULL)
        m_sPrivacyPolicyURL = strconv.utf82t(szPrivacyPolicyUrl);
    }
  }

  {
    TiXmlHandle hHttpPriority = hRoot.FirstChild("HttpPriority");
    if(hHttpPriority.FirstChild().ToText()!=NULL)
    {
      const char* szHttpPriority = hHttpPriority.FirstChild().ToText()->Value();
      if(szHttpPriority!=NULL)
        m_uPriorities[CR_HTTP] = atoi(szHttpPriority);
      else
        m_uPriorities[CR_HTTP] = 0;
    }
  }

  {
    TiXmlHandle hSmtpPriority = hRoot.FirstChild("SmtpPriority");
    if(hSmtpPriority.FirstChild().ToText()!=NULL)
    {
      const char* szSmtpPriority = hSmtpPriority.FirstChild().ToText()->Value();
      if(szSmtpPriority!=NULL)
        m_uPriorities[CR_SMTP] = atoi(szSmtpPriority);
      else
        m_uPriorities[CR_SMTP] = 0;
    }
  }

  {
    TiXmlHandle hMapiPriority = hRoot.FirstChild("MapiPriority");
    if(hMapiPriority.FirstChild().ToText()!=NULL)
    {
      const char* szMapiPriority = hMapiPriority.FirstChild().ToText()->Value();
      if(szMapiPriority!=NULL)
        m_uPriorities[CR_SMAPI] = atoi(szMapiPriority);
      else
        m_uPriorities[CR_SMAPI] = 0;
    }
  }

  {
    TiXmlHandle hProcessId = hRoot.FirstChild("ProcessId");
    if(hProcessId.FirstChild().ToText()!=NULL)
    {
      const char* szProcessId = hProcessId.FirstChild().ToText()->Value();
      if(szProcessId!=NULL)
        m_dwProcessId = strtoul(szProcessId, NULL, 10);
      else
        m_dwProcessId = 0;
    }
  }

  {
    TiXmlHandle hThreadId = hRoot.FirstChild("ThreadId");
    if(hThreadId.FirstChild().ToText()!=NULL)
    {
      const char* szThreadId = hThreadId.FirstChild().ToText()->Value();
      if(szThreadId!=NULL)
        m_dwThreadId = strtoul(szThreadId, NULL, 10);
      else 
        m_dwThreadId = 0;
    }
  }

  {
    m_pExInfo = NULL;
    TiXmlHandle hExceptionPointersAddress = hRoot.FirstChild("ExceptionPointersAddress");
    if(hExceptionPointersAddress.FirstChild().ToText()!=NULL)
    {
      const char* szExceptionPointersAddress = hExceptionPointersAddress.FirstChild().ToText()->Value();
      if(szExceptionPointersAddress!=NULL)
        m_pExInfo = (PEXCEPTION_POINTERS)_strtoui64(szExceptionPointersAddress, NULL, 16);     
    }    
  }

  {
    TiXmlHandle hAddScreenshot = hRoot.FirstChild("AddScreenshot");
    if(hAddScreenshot.FirstChild().ToText()!=NULL)
    {
      const char* szAddScreenshot = hAddScreenshot.FirstChild().ToText()->Value();
      if(szAddScreenshot!=NULL)
        m_bAddScreenshot = strtol(szAddScreenshot, NULL, 10);
      else
        m_bAddScreenshot = FALSE;
    }
  }

  {
    m_dwScreenshotFlags = 0;
    TiXmlHandle hScreenshotFlags = hRoot.FirstChild("ScreenshotFlags");
    if(hScreenshotFlags.FirstChild().ToText()!=NULL)
    {
      const char* szScreenshotFlags = hScreenshotFlags.FirstChild().ToText()->Value();
      if(szScreenshotFlags!=NULL)
        m_dwScreenshotFlags = strtoul(szScreenshotFlags, NULL, 10);        
    }
  }

  {
    m_ptCursorPos.SetPoint(0, 0);
    TiXmlHandle hCursorPos = hRoot.FirstChild("CursorPos");
    if(hCursorPos.ToElement()!=NULL)
    {
      const char* szX = hCursorPos.ToElement()->Attribute("x");
      const char* szY = hCursorPos.ToElement()->Attribute("y");
      if(szX && szY)
      {
        m_ptCursorPos.x = atoi(szX);
        m_ptCursorPos.y = atoi(szY);
      }
    }
  }

  {
    m_rcAppWnd.SetRectEmpty();
    TiXmlHandle hAppWndRect = hRoot.FirstChild("AppWindowRect");
    if(hAppWndRect.ToElement()!=NULL)
    {
      const char* szLeft = hAppWndRect.ToElement()->Attribute("left");
      const char* szTop = hAppWndRect.ToElement()->Attribute("top");
      const char* szRight = hAppWndRect.ToElement()->Attribute("right");
      const char* szBottom = hAppWndRect.ToElement()->Attribute("bottom");

      if(szLeft && szTop && szRight && szBottom)
      {
        m_rcAppWnd.left = atoi(szLeft);
        m_rcAppWnd.top = atoi(szTop);
        m_rcAppWnd.right = atoi(szRight);
        m_rcAppWnd.bottom = atoi(szBottom);
      }
    }
  }

  {
    m_bHttpBinaryEncoding = FALSE;    
    TiXmlHandle hHttpBinaryEncoding = hRoot.FirstChild("HttpBinaryEncoding");
    if(hHttpBinaryEncoding.FirstChild().ToText()!=NULL)
    {
      const char* szHttpBinaryEncoding = hHttpBinaryEncoding.FirstChild().ToText()->Value();
      if(szHttpBinaryEncoding!=NULL)
        m_bHttpBinaryEncoding = atoi(szHttpBinaryEncoding);
    }      
  }

  {
    m_bSilentMode = FALSE;    
    TiXmlHandle hSilentMode = hRoot.FirstChild("SilentMode");
    if(hSilentMode.FirstChild().ToText()!=NULL)
    {
      const char* szSilentMode = hSilentMode.FirstChild().ToText()->Value();
      if(szSilentMode!=NULL)
        m_bSilentMode = atoi(szSilentMode);
    }      
  }

  {
    m_bSendErrorReport = FALSE;    
    TiXmlHandle hSendErrorReport = hRoot.FirstChild("SendErrorReport");
    if(hSendErrorReport.FirstChild().ToText()!=NULL)
    {
      const char* szSendErrorReport = hSendErrorReport.FirstChild().ToText()->Value();
      if(szSendErrorReport!=NULL)
        m_bSendErrorReport = atoi(szSendErrorReport);     
    }      
  }

  {
    m_bAppRestart = FALSE;    
    TiXmlHandle hAppRestart = hRoot.FirstChild("AppRestart");
    if(hAppRestart.FirstChild().ToText()!=NULL)
    {
      const char* szAppRestart = hAppRestart.FirstChild().ToText()->Value();
      if(szAppRestart!=NULL)
        m_bAppRestart = atoi(szAppRestart);     
    }      
  }

  {    
    TiXmlHandle hRestartCmdLine = hRoot.FirstChild("RestartCmdLine");
    if(hRestartCmdLine.FirstChild().ToText()!=NULL)
    {
      const char* szRestartCmdLine = hRestartCmdLine.FirstChild().ToText()->Value();
      if(szRestartCmdLine!=NULL)
        m_sRestartCmdLine = strconv.utf82t(szRestartCmdLine);     
    }      
  }

  ParseCrashDescription(m_sErrorReportDirName + _T("\\crashrpt.xml"));

  return ParseFileList(hRoot);
}

int CCrashInfoReader::ParseFileList(TiXmlHandle& hRoot)
{
  strconv_t strconv;
   
  TiXmlHandle fl = hRoot.FirstChild("FileList");
  if(fl.ToElement()==0)
  {    
    return 1;
  }

  TiXmlHandle fi = fl.FirstChild("FileItem");
  while(fi.ToElement()!=0)
  {
    const char* pszDestFile = fi.ToElement()->Attribute("destfile");
    const char* pszSrcFile = fi.ToElement()->Attribute("srcfile");
    const char* pszDesc = fi.ToElement()->Attribute("description");
    const char* pszMakeCopy = fi.ToElement()->Attribute("makecopy");

    if(pszDestFile!=NULL)
    {
	    CString sDestFile = strconv.utf82t(pszDestFile);      
      FileItem item;
      item.m_sDestFile = sDestFile;
      if(pszSrcFile)
        item.m_sSrcFile = strconv.utf82t(pszSrcFile);
      if(pszDesc)
        item.m_sDesc = strconv.utf82t(pszDesc);

      if(pszMakeCopy)
      {
        if(strcmp(pszMakeCopy, "1")==0)
          item.m_bMakeCopy = TRUE;
        else
          item.m_bMakeCopy = FALSE;
      }
      else
        item.m_bMakeCopy = FALSE;
      
      m_FileItems[sDestFile] = item;
    }

    fi = fi.ToElement()->NextSibling("FileItem");
  }

  return 0;
}

int CCrashInfoReader::ParseCrashDescription(CString sFileName)
{
  strconv_t strconv;

  TiXmlDocument doc;
  bool bOpen = doc.LoadFile(strconv.t2a(sFileName));
  if(!bOpen)
    return 1;

  TiXmlHandle hRoot = doc.FirstChild("CrashRpt");
  if(hRoot.ToElement()==NULL)
    return 1;

  {
    TiXmlHandle hAppName = hRoot.FirstChild("AppName");
    const char* szAppName = hAppName.FirstChild().ToText()->Value();
    if(szAppName!=NULL)
      m_sAppName = strconv.utf82t(szAppName);
  }

  {
    TiXmlHandle hAppVersion = hRoot.FirstChild("AppVersion");
    const char* szAppVersion = hAppVersion.FirstChild().ToText()->Value();
    if(szAppVersion!=NULL)
      m_sAppVersion = strconv.utf82t(szAppVersion);
  }

  {
    TiXmlHandle hImageName = hRoot.FirstChild("ImageName");
    const char* szImageName = hImageName.FirstChild().ToText()->Value();
    if(szImageName!=NULL)
      m_sImageName = strconv.utf82t(szImageName);
  }

  return 0;
}

BOOL CCrashInfoReader::AddUserInfoToCrashDescriptionXML(CString sEmail, CString sDesc)
{ 
  strconv_t strconv;

  TiXmlDocument doc;
  
  CString sFileName = g_CrashInfo.m_sErrorReportDirName + _T("\\crashrpt.xml");
  bool bLoad = doc.LoadFile(strconv.t2a(sFileName.GetBuffer(0)));
  if(!bLoad)
    return FALSE;

  TiXmlNode* root = doc.FirstChild("CrashRpt");
  if(!root)
    return FALSE;

  // Write user e-mail

  TiXmlElement* email = new TiXmlElement("UserEmail");
  root->LinkEndChild(email);
  
  TiXmlText* email_text = new TiXmlText(strconv.t2utf8(sEmail));
  email->LinkEndChild(email_text);              

  // Write problem description

  TiXmlElement* desc = new TiXmlElement("ProblemDescription");
  root->LinkEndChild(desc);

  TiXmlText* desc_text = new TiXmlText(strconv.t2utf8(sDesc));
  desc->LinkEndChild(desc_text);              

  bool bSave = doc.SaveFile(); 
  if(!bSave)
    return FALSE;
  return TRUE;
}

BOOL CCrashInfoReader::AddFilesToCrashDescriptionXML(std::vector<FileItem> FilesToAdd)
{
  strconv_t strconv;

  TiXmlDocument doc;
  
  CString sFileName = g_CrashInfo.m_sErrorReportDirName + _T("\\crashrpt.xml");
  bool bLoad = doc.LoadFile(strconv.t2a(sFileName.GetBuffer(0)));
  if(!bLoad)
    return FALSE;

  TiXmlNode* root = doc.FirstChild("CrashRpt");
  if(!root)
    return FALSE;
  
  TiXmlHandle hFileItems = root->FirstChild("FileList");
  if(hFileItems.ToElement()==NULL)
  {
    hFileItems = new TiXmlElement("FileList");
    root->LinkEndChild(hFileItems.ToNode());
  }
  
  unsigned i;
  for(i=0; i<FilesToAdd.size(); i++)
  {    
    TiXmlHandle hFileItem = new TiXmlElement("FileItem");
    hFileItem.ToElement()->SetAttribute("name", strconv.t2utf8(FilesToAdd[i].m_sDestFile));
    hFileItem.ToElement()->SetAttribute("descrition", strconv.t2utf8(FilesToAdd[i].m_sDesc));
    hFileItems.ToElement()->LinkEndChild(hFileItem.ToNode());              

    m_FileItems[FilesToAdd[i].m_sDestFile] = FilesToAdd[i];
  }
  
  bool bSave = doc.SaveFile(); 
  if(!bSave)
    return FALSE;
  return TRUE;
}


