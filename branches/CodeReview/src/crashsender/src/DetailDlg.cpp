#include "stdafx.h"
#include "DetailDlg.h"

LRESULT CDetailDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
  m_list = GetDlgItem(IDC_FILE_LIST);
  m_list.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);

  m_list.InsertColumn(0, _T("Name"), LVCFMT_LEFT, 120);
  m_list.InsertColumn(1, _T("Description"), LVCFMT_LEFT, 80);
  m_list.InsertColumn(2, _T("Type"), LVCFMT_LEFT, 80);
  m_list.InsertColumn(3, _T("Size"), LVCFMT_RIGHT, 80);

  m_iconList.Create(16, 16, ILC_COLOR32|ILC_MASK, 3, 1);
  m_list.SetImageList(m_iconList, LVSIL_SMALL);

  // Insert items
  WIN32_FIND_DATA   findFileData   = {0};
  HANDLE            hFind          = NULL;
  CString           sSize;
  LVITEM            lvi            = {0};
  TStrStrMap::iterator p;
  unsigned i;
  for (i = 0, p = m_pUDFiles.begin(); p != m_pUDFiles.end(); p++, i++)
  {     
    SHFILEINFO sfi;
    SHGetFileInfo(CString(p->first), 0, &sfi, sizeof(sfi),
      SHGFI_DISPLAYNAME | SHGFI_ICON | SHGFI_TYPENAME | SHGFI_SMALLICON);

    int iImage = -1;
    if(sfi.hIcon)
    {
      iImage = m_iconList.AddIcon(sfi.hIcon);
      DestroyIcon(sfi.hIcon);
    }

    int nItem = m_list.InsertItem(i, sfi.szDisplayName, iImage);
    m_list.SetItemText(nItem, 1, CString(p->second));
    m_list.SetItemText(nItem, 2, CString(sfi.szTypeName));

    hFind = FindFirstFile(CString(p->first), &findFileData);
    if (INVALID_HANDLE_VALUE != hFind)
    {
      FindClose(hFind);
      sSize.Format(TEXT("%d KB"), findFileData.nFileSizeLow);
      m_list.SetItemText(nItem, 3, sSize);
    }    
  }

  m_list.SetItemState(0, LVIS_SELECTED, LVIS_SELECTED);

  // center the dialog on the screen
	CenterWindow();
  FlashWindow(TRUE);

  return TRUE;
}

LRESULT CDetailDlg::OnItemChanged(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
{
  LPNMLISTVIEW lpItem           = (LPNMLISTVIEW)pnmh; 
  int iItem                     = lpItem->iItem;

  if (lpItem->uChanged & LVIF_STATE
     && lpItem->uNewState & LVIS_SELECTED)
  {
     SelectItem(iItem);
  }

  return 0;
}

LRESULT CDetailDlg::OnItemDblClicked(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
{
  LPNMLISTVIEW lpItem           = (LPNMLISTVIEW)pnmh; 
  int iItem                     = lpItem->iItem;
  DWORD_PTR dwRet               = 0;

  if (iItem < 0 || (int)m_pUDFiles.size() < iItem)
     return 0;

  TStrStrMap::iterator p = m_pUDFiles.begin();
  for (int i = 0; i < iItem; i++, p++);

  dwRet = (DWORD_PTR)::ShellExecute(0, _T("open"), CString(p->first),
    0, 0, SW_SHOWNORMAL);
  ATLASSERT(dwRet > 32);

  return 0;
}

void CDetailDlg::SelectItem(int iItem)
{
  const int MAX_FILE_SIZE          = 32768; // 32k file preview max
  DWORD dwBytesRead                = 0;
  BYTE buffer[MAX_FILE_SIZE + 1]  = "";

  // Sanity check
  if (iItem < 0 || (int)m_pUDFiles.size() < iItem)
      return;

  TStrStrMap::iterator p = m_pUDFiles.begin();
  for (int i = 0; i < iItem; i++, p++);

  //
  // Display file contents in preview window
  //
  HANDLE hFile = CreateFile(
     CString(p->first),
     GENERIC_READ,
     FILE_SHARE_READ | FILE_SHARE_WRITE,
     NULL,
     OPEN_EXISTING,
     FILE_ATTRIBUTE_NORMAL,
     0);

  if (hFile!=INVALID_HANDLE_VALUE)
  {
     // Read up to first 32 KB
     ReadFile(hFile, buffer, MAX_FILE_SIZE, &dwBytesRead, 0);
     buffer[dwBytesRead] = 0;
     CloseHandle(hFile);
  }

  // Update edit control with file contents
  ::SetWindowTextA(GetDlgItem(IDC_FILE_EDIT), (char*)buffer);
}

LRESULT CDetailDlg::OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  EndDialog(0);
  return 0;
}


