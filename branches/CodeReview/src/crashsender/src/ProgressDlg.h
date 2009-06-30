#pragma once

#include "resource.h"
#include "SenderThread.h"
#include "httpsend.h"
#include "MailMsg.h"
#include "smtpclient.h"

class CProgressDlg : public CDialogImpl<CProgressDlg>,
  public CDialogResize<CProgressDlg>
{
public:
	enum { IDD = IDD_PROGRESSDLG };
  
  CProgressBarCtrl m_prgProgress;
  CListBox m_listBox;
  SenderThreadContext* m_pctx;

  BEGIN_DLGRESIZE_MAP(CProgressDlg)
    DLGRESIZE_CONTROL(IDC_PROGRESS, DLSZ_SIZE_X)
    DLGRESIZE_CONTROL(IDC_LIST, DLSZ_SIZE_X|DLSZ_SIZE_Y)
    DLGRESIZE_CONTROL(IDCANCEL, DLSZ_MOVE_Y)    
  END_DLGRESIZE_MAP()

	BEGIN_MSG_MAP(CProgressDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
    MESSAGE_HANDLER(WM_CLOSE, OnClose)
    MESSAGE_HANDLER(WM_TIMER, OnTimer)    
    COMMAND_ID_HANDLER(IDCANCEL, OnCancel)

    CHAIN_MSG_MAP(CDialogResize<CProgressDlg>)
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
  LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
  LRESULT OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);  

  void Start();
  void CloseDialog(int nVal);

  BOOL m_bFinished;
  
};


