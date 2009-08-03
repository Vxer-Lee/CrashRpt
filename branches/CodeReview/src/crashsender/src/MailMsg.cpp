///////////////////////////////////////////////////////////////////////////////
//
//  Module: MailMsg.cpp
//
//    Desc: See MailMsg.h
//
// Copyright (c) 2003 Michael Carruth
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MailMsg.h"

CMailMsg::CMailMsg()
{
   m_lpCmcLogon      = NULL;
   m_lpCmcSend       = NULL;
   m_lpCmcLogoff     = NULL;
   m_lpMapiLogon     = NULL;
   m_lpMapiSendMail  = NULL;
   m_lpMapiLogoff    = NULL;
   m_bReady          = FALSE;
}

CMailMsg::~CMailMsg()
{
  if (m_bReady)
    MAPIFinalize();
}


void CMailMsg::SetFrom(CString sAddress)
{  
  USES_CONVERSION;
  LPSTR lpszAddress = T2A(sAddress.GetBuffer(0));
  m_from = lpszAddress;
}

void CMailMsg::SetTo(CString sAddress)
{
  USES_CONVERSION;
  LPSTR lpszAddress = T2A(sAddress.GetBuffer(0));
  m_to = lpszAddress;
}

void CMailMsg::SetSubject(CString sSubject)
{
  USES_CONVERSION;
  LPSTR lpszSubject = T2A(sSubject.GetBuffer(0));
  m_sSubject = lpszSubject;
}

void CMailMsg::SetMessage(CString sMessage) 
{
  USES_CONVERSION;
  LPSTR lpszMessage = T2A(sMessage.GetBuffer(0));
  m_sMessage = lpszMessage;
};

void CMailMsg::AddAttachment(CString sAttachment, CString sTitle)
{
  USES_CONVERSION;
  LPSTR lpszAttachment = T2A(sAttachment.GetBuffer(0));
  LPSTR lpszTitle = T2A(sTitle.GetBuffer(0));
  m_attachments[lpszAttachment] = lpszTitle;  
}

BOOL CMailMsg::DetectMailClient()
{
  CRegKey regKey;
  TCHAR buf[1024];
  ULONG buf_size = 0;
  LONG lResult;

  /*lResult = regKey.Open(HKEY_CURRENT_USER, _T("SOFTWARE\\Clients\\Mail"), KEY_READ);
  if(lResult==ERROR_SUCCESS)
  {    
    buf_size = 1024;
    LONG result = regKey.QueryStringValue(_T(""), buf, &buf_size);
    if(result==ERROR_SUCCESS)
    {
      m_sEmailClientName = CString(buf, buf_size);      
      return TRUE;  
    }

    regKey.Close();
  }*/  

  lResult = regKey.Open(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Clients\\Mail"), KEY_READ);
  if(lResult==ERROR_SUCCESS)
  {    
    buf_size = 1024;
#pragma warning(suppress:4996)
    LONG result = regKey.QueryValue(_T(""), buf, &buf_size);
    if(result==ERROR_SUCCESS)
    {
      m_sEmailClientName = CString(buf, buf_size);
      return TRUE;  
    }

    regKey.Close();
  }

  return FALSE;
}

BOOL CMailMsg::MAPIInitialize()
{   
   // Determine if there is default email program

   if(!DetectMailClient())
   {
     m_sErrorMsg = _T("Error detecting E-mail client");
     return FALSE;
   }
   else
   {
     m_sErrorMsg = _T("Detected E-mail client ") + m_sEmailClientName;
   }
   
   
   // Load MAPI.dll

   m_hMapi = ::LoadLibrary(_T("mapi32.dll"));
   if (!m_hMapi)
   {
     m_sErrorMsg = _T("Error loading mapi32.dll");
     return FALSE;
   }

   m_lpCmcQueryConfiguration = (LPCMCQUERY)::GetProcAddress(m_hMapi, "cmc_query_configuration");
   m_lpCmcLogon = (LPCMCLOGON)::GetProcAddress(m_hMapi, "cmc_logon");
   m_lpCmcSend = (LPCMCSEND)::GetProcAddress(m_hMapi, "cmc_send");
   m_lpCmcLogoff = (LPCMCLOGOFF)::GetProcAddress(m_hMapi, "cmc_logoff");
   
   m_lpMapiLogon = (LPMAPILOGON)::GetProcAddress(m_hMapi, "MAPILogon");
   m_lpMapiSendMail = (LPMAPISENDMAIL)::GetProcAddress(m_hMapi, "MAPISendMail");
   m_lpMapiLogoff = (LPMAPILOGOFF)::GetProcAddress(m_hMapi, "MAPILogoff");

   m_bReady = (m_lpCmcLogon && m_lpCmcSend && m_lpCmcLogoff) ||
              (m_lpMapiLogon && m_lpMapiSendMail && m_lpMapiLogoff);

   if(!m_bReady)
   {
     m_sErrorMsg = _T("Not found required function entries in mapi32.dll");
   }

   return m_bReady;
}

void CMailMsg::MAPIFinalize()
{
   ::FreeLibrary(m_hMapi);
}

CString CMailMsg::GetEmailClientName()
{
  return m_sEmailClientName;
}

BOOL CMailMsg::Send()
{
  if(MAPISend())
    return TRUE;

  if(CMCSend())
    return TRUE;

  return FALSE;
}

BOOL CMailMsg::MAPISend()
{
  if(m_lpMapiSendMail==NULL)
      return FALSE;

   TStrStrMap::iterator p;
   int                  nIndex = 0;   
   MapiRecipDesc*       pRecipients = NULL;
   int                  nAttachments = 0;
   MapiFileDesc*        pAttachments = NULL;
   ULONG                status = 0;
   MapiMessage          message;

   if(!m_bReady && !MAPIInitialize())
     return FALSE;
   
   LHANDLE hMapiSession = 0;
   status = m_lpMapiLogon(NULL, NULL, NULL, MAPI_LOGON_UI|MAPI_PASSWORD_UI, NULL, &hMapiSession);
   if(status!=SUCCESS_SUCCESS)
   {
     m_sErrorMsg.Format(_T("MAPILogon has failed with code %X."), status);
     return FALSE;
   }


   pRecipients = new MapiRecipDesc[2];
   if(!pRecipients)
   {
     m_sErrorMsg = _T("Error allocating memory");
     return FALSE;
   }

   nAttachments = (int)m_attachments.size();
   if (nAttachments)
     pAttachments = new MapiFileDesc[nAttachments];
   if(!pAttachments)
   {
     m_sErrorMsg = _T("Error allocating memory");
     return FALSE;
   }

   // set from
   pRecipients[0].ulReserved = 0;
   pRecipients[0].ulRecipClass = MAPI_ORIG;
   pRecipients[0].lpszAddress = (LPSTR)m_from.c_str();
   pRecipients[0].lpszName = "";
   pRecipients[0].ulEIDSize = 0;
   pRecipients[0].lpEntryID = NULL;
      
   // set to
   pRecipients[1].ulReserved = 0;
   pRecipients[1].ulRecipClass = MAPI_TO;
   pRecipients[1].lpszAddress = (LPSTR)m_to.c_str();
   pRecipients[1].lpszName = (LPSTR)m_to.c_str();
   pRecipients[1].ulEIDSize = 0;
   pRecipients[1].lpEntryID = NULL;
      
   
   // add attachments
   nIndex=0;   
    for (p = m_attachments.begin(), nIndex = 0;
      p != m_attachments.end(); p++, nIndex++)
    {
      pAttachments[nIndex].ulReserved        = 0;
      pAttachments[nIndex].flFlags           = 0;
      pAttachments[nIndex].nPosition         = 0xFFFFFFFF;
	    pAttachments[nIndex].lpszPathName      = (LPSTR)p->first.c_str();
	    pAttachments[nIndex].lpszFileName      = (LPSTR)p->second.c_str();
      pAttachments[nIndex].lpFileType        = NULL;
    }
    
    message.ulReserved                        = 0;
	  message.lpszSubject                       = (LPSTR)m_sSubject.c_str();
    message.lpszNoteText                      = (LPSTR)m_sMessage.c_str();
    message.lpszMessageType                   = NULL;
    message.lpszDateReceived                  = NULL;
    message.lpszConversationID                = NULL;
    message.flFlags                           = 0;
    message.lpOriginator                      = pRecipients;
    message.nRecipCount                       = 1;
    message.lpRecips                          = &pRecipients[1];
    message.nFileCount                        = nAttachments;
    message.lpFiles                           = nAttachments ? pAttachments : NULL;
        
    status = m_lpMapiSendMail(hMapiSession, 0, &message, 0/*MAPI_DIALOG*/, 0);    

    if(status!=SUCCESS_SUCCESS)
    {
      m_sErrorMsg.Format(_T("MAPISendMail has failed with code %X."), status);      
    }

    m_lpMapiLogoff(hMapiSession, NULL, 0, 0);

    if (pRecipients)
       delete [] pRecipients;

    if (nAttachments)
         delete [] pAttachments;
   
   return (SUCCESS_SUCCESS == status);
}

BOOL CMailMsg::CMCSend()
{
  TStrStrMap::iterator p;
  int                  nIndex = 0;
  CMC_recipient*       pRecipients;
  CMC_attachment*      pAttachments;
  CMC_session_id       session;
  CMC_return_code      status = 0;
  CMC_message          message;
  CMC_boolean          bAvailable = FALSE;
  CMC_time             t_now = {0};

  if (!m_bReady && !MAPIInitialize())
    return FALSE;

  pRecipients = new CMC_recipient[2];
  pAttachments = new CMC_attachment[m_attachments.size()];

  // set to
  pRecipients[nIndex].name = (LPSTR)m_to.c_str();
  pRecipients[nIndex].name_type = CMC_TYPE_INDIVIDUAL;
  pRecipients[nIndex].address = (CMC_string)(LPCSTR)m_to.c_str();
  pRecipients[nIndex].role = CMC_ROLE_TO;
  pRecipients[nIndex].recip_flags = 0;
  pRecipients[nIndex].recip_extensions = NULL;
   
  // set from
  pRecipients[nIndex+1].name = (LPSTR)m_from.c_str();
  pRecipients[nIndex+1].name_type = CMC_TYPE_INDIVIDUAL;
  pRecipients[nIndex+1].address = (CMC_string)(LPCSTR)m_from.c_str();
  pRecipients[nIndex+1].role = CMC_ROLE_ORIGINATOR;
  pRecipients[nIndex+1].recip_flags = CMC_RECIP_LAST_ELEMENT;
  pRecipients[nIndex+1].recip_extensions = NULL;
   
  // add attachments
  for (p = m_attachments.begin(), nIndex = 0;
    p != m_attachments.end(); p++, nIndex++)
  {
    pAttachments[nIndex].attach_title       = (LPSTR)p->second.c_str();
    pAttachments[nIndex].attach_type        = NULL;
	pAttachments[nIndex].attach_filename    = (CMC_string)(LPCSTR)p->first.c_str();
    pAttachments[nIndex].attach_flags       = 0;
    pAttachments[nIndex].attach_extensions  = NULL;
  }
  
  pAttachments[nIndex-1].attach_flags        = CMC_ATT_LAST_ELEMENT;

  message.message_reference                 = NULL;
  message.message_type                      = NULL;
  message.subject                           = (LPSTR)m_sSubject.c_str();
  message.time_sent                         = t_now;
  message.text_note                         = (LPSTR)m_sMessage.c_str();
  message.recipients                        = pRecipients;
  message.attachments                       = pAttachments;
  message.message_flags                     = 0;
  message.message_extensions                = NULL;

  status = m_lpCmcQueryConfiguration(
    0, 
    CMC_CONFIG_UI_AVAIL, 
    (void*)&bAvailable, 
    NULL
    );

  if (CMC_SUCCESS == status && bAvailable)
  {
    status = m_lpCmcLogon(
      NULL,
      NULL,
      NULL,
      NULL,
      0,
      CMC_VERSION,
      CMC_LOGON_UI_ALLOWED |
      CMC_ERROR_UI_ALLOWED,
      &session,
      NULL
      );

    if (CMC_SUCCESS == status)
    {
      status = m_lpCmcSend(session, &message, 0, 0, NULL);
      m_lpCmcLogoff(session, NULL, CMC_LOGON_UI_ALLOWED, NULL);
    }
  }

  delete [] pRecipients;
  delete [] pAttachments;

  return ((CMC_SUCCESS == status) && bAvailable);
}

