
// RegistryTimeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "RegistryTime.h"
#include "RegistryTimeDlg.h"
#include "afxdialogex.h"
#include <malloc.h>
#include <strsafe.h>

#include "NtRegistry.h"
#include "Registry.h"
#include "DateTimeDlg.h"
#include "Splash.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CRegistryTimeDlg dialog

CRegistryTimeDlg::CRegistryTimeDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CRegistryTimeDlg::IDD, pParent)
{
	timerKicked = false;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CRegistryTimeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CRegistryTimeDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_NOTIFY(TVN_ITEMEXPANDED, TC_KEYS, &CRegistryTimeDlg::OnItemExpanded)
	ON_NOTIFY(TVN_ITEMEXPANDING, TC_KEYS, &CRegistryTimeDlg::OnItemExpanding)
	ON_NOTIFY(NM_CLICK, TC_KEYS, &CRegistryTimeDlg::OnClick)
	ON_NOTIFY(NM_RCLICK, TC_KEYS, &CRegistryTimeDlg::OnRightClick)
	ON_NOTIFY(TVN_SELCHANGED, TC_KEYS, &CRegistryTimeDlg::OnSelChanged)
	ON_BN_CLICKED(IDC_BAIL, &CRegistryTimeDlg::OnBnClickedBail)
	ON_WM_TIMER()
END_MESSAGE_MAP()

// CRegistryTimeDlg message handlers
CSplash *pSplash;

BOOL CRegistryTimeDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	pSplash = new CSplash(NULL);
	pSplash->Create(CSplash::IDD);
	pSplash->ShowWindow(SW_SHOW);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

CBitmap bm2;
bool loaded = false;

void CRegistryTimeDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CPaintDC dc(this); // device context for painting

		RECT clientRect;
		GetClientRect(&clientRect);
		int w = clientRect.right - clientRect.left;
		int h = clientRect.bottom - clientRect.top;

		if (!loaded)
		{
			BOOL ret = bm2.LoadBitmap(IDB_BITMAP4);
			loaded = true;
		}

		CDC memDC;
		memDC.CreateCompatibleDC(&dc);
		memDC.SelectObject(&bm2);

		dc.StretchBlt(0, 0, w + 160, h + 60, &memDC, 0, 0, 754, 400, SRCCOPY);

		CWnd * pW = this->GetDlgItem(IDC_STATIC);
		RECT rc;
		pW->GetClientRect(&rc);
		pW->ClientToScreen(&rc);
		this->ScreenToClient(&rc);

		dc.FillSolidRect(&rc, RGB(255, 255, 255));

		CDialogEx::OnPaint();
	}

	if (!timerKicked)
	{
		SetTimer(1, 200, NULL);
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CRegistryTimeDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void populateTreeNode(CTreeCtrl *tc, HTREEITEM item, HKEY hKey, char path[], int depth = 0, bool recurse = false)
{

	if (depth >= 3) return;

	char *fullPath = (char *)calloc(1, 1500);
	WCHAR *achClass = (WCHAR *)calloc(sizeof(WCHAR), 1500);
	WCHAR *achKey = (WCHAR *)calloc(sizeof(WCHAR), 1500);
	WCHAR *s = (WCHAR *)calloc(sizeof(WCHAR), 1500);

	for (int i = 0; i < (int)strlen(path); i++)
	{
		s[i] = (WCHAR)path[i];
		s[i + 1] = 0;
	}

	HKEY _hKey;
	int ret = RegOpenKeyEx(hKey, s, 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, &_hKey);
	if (_hKey == NULL)
	{
		goto FreeMem;
	}

	DWORD cSubKeys = 0;
	DWORD cchClassName = 255;
	DWORD cbMaxSubKey, cchMaxClass, cValues, cchMaxValue, cbMaxValueData, cbSecurityDescriptor;
	FILETIME ftLastWriteTime;

	int result = RegQueryInfoKey(
		_hKey,                  // key handle 
		achClass,                // buffer for class name 
		&cchClassName,           // size of class string 
		NULL,                    // reserved 
		&cSubKeys,               // number of subkeys 
		&cbMaxSubKey,            // longest subkey size 
		&cchMaxClass,            // longest class string 
		&cValues,                // number of values for this key 
		&cchMaxValue,            // longest value name 
		&cbMaxValueData,         // longest value data 
		&cbSecurityDescriptor,   // security descriptor 
		&ftLastWriteTime);       // last write time

	for (int i = 0; i<(int)cSubKeys; i++)
	{
		FILETIME ftLastWriteTime;
		DWORD cbName;
		cbName = 500;

		RegEnumKeyEx(_hKey, i,
			achKey,
			&cbName,
			NULL,
			NULL,
			NULL,
			&ftLastWriteTime );

		strcpy(fullPath, path);
		strcat(fullPath, "\\");
		int j = 0, k = strlen(fullPath);
		while (achKey[j] != 0)
		{
			fullPath[k] = (char)achKey[j++];
			fullPath[++k] = 0;
		}

		if (fullPath[0] == '\\' )
		{ 
			memmove(fullPath, &fullPath[1], strlen(fullPath));
		}

		memset(s, 1500*sizeof(WCHAR),0);

		for (int i = 0; i < (int)strlen(fullPath); i++)
		{
			s[i] = (WCHAR)fullPath[i];
			s[i + 1] = 0;
		}

		HTREEITEM t = tc->InsertItem(achKey, 2, 2, item);

		// Do recursion...
		if (t != NULL)
		{
			if (recurse)
			{
				populateTreeNode(tc, t, hKey, fullPath, depth + 1, true);
			}
		}

	}

	RegCloseKey(_hKey);

FreeMem:
	free(fullPath);
	free(achClass);
	free(achKey);
	free(s);
}

void CRegistryTimeDlg::InitTree()
{
	CTreeCtrl& tc = *GetTreeCtrl();

	// Create the image list that is attached to the tree control
	// Loadup the image list
	m_pImageTree.Create(IDB_ICONS, 16, 1, RGB(255, 0, 255));
	//Hook it up to the tree control
	tc.SetImageList(&m_pImageTree, TVSIL_NORMAL);

	tc.SetRedraw(FALSE);
	tc.DeleteAllItems();

	DWORD cnt;
	WCHAR nme[100];
	memset(nme, 100 * sizeof(WCHAR), 0);
	char nme2[100];
	GetComputerNameA(nme2, &cnt);

	for (int i = 0; i < (int)strlen(nme2); i++)
	{
		nme[i] = (WCHAR)nme2[i];
		nme[i + 1] = 0;
	}

	m_hRoot = tc.InsertItem(nme, 0, 0);
	m_hHKCR = tc.InsertItem(_T("HKEY_CLASSES_ROOT"), 2, 2, m_hRoot);
	populateTreeNode(&tc, m_hHKCR, HKEY_CLASSES_ROOT, "", 0, true);

	m_hHKCU = tc.InsertItem(_T("HKEY_CURRENT_USER"), 2, 2, m_hRoot);
	populateTreeNode(&tc, m_hHKCU, HKEY_CURRENT_USER, "", 0, true);

	m_hHKLM = tc.InsertItem(_T("HKEY_LOCAL_MACHINE"), 2, 2, m_hRoot);
	populateTreeNode(&tc, m_hHKLM, HKEY_LOCAL_MACHINE, "", 0, true);

	m_hHKU = tc.InsertItem(_T("HKEY_USERS"), 2, 2, m_hRoot);
	populateTreeNode(&tc, m_hHKU, HKEY_USERS, "", 0, true);

	m_hHKCC = tc.InsertItem(_T("HKEY_CURRENT_CONFIG"), 2, 2, m_hRoot);
	populateTreeNode(&tc, m_hHKCC, HKEY_CURRENT_CONFIG, "", 0, true);

	m_ntReg.LookupSID(m_csSID);
	tc.Expand(m_hRoot, TVE_EXPAND);
	tc.SetRedraw(TRUE);

	pSplash->ShowWindow(SW_HIDE);
	delete pSplash;
}

CTreeCtrl *CRegistryTimeDlg::GetTreeCtrl()
{
	CTreeCtrl *tree = (CTreeCtrl *)GetDlgItem(TC_KEYS);
	return tree;
}


void CRegistryTimeDlg::OnItemExpanded(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
}

int CRegistryTimeDlg::getChildCount(CTreeCtrl *tc, HTREEITEM item)
{
	int count = 0;
	HTREEITEM hChildItem = tc->GetChildItem(item);
	HTREEITEM hNextItem;
	while (hChildItem != NULL)
	{
		hNextItem = tc->GetNextItem(hChildItem, TVGN_NEXT);
		hChildItem = hNextItem;
		count++;
	}
	return count;
}

HKEY CRegistryTimeDlg::backUpToHKEY(CTreeCtrl *tc, HTREEITEM hChildItem)
{
	CString strItemText = tc->GetItemText(hChildItem);
	while (tc->GetParentItem(hChildItem) != NULL)
	{
		hChildItem = tc->GetParentItem(hChildItem);
		strItemText = tc->GetItemText(hChildItem);
		if (strItemText[0] == 'H' && strItemText[1] == 'K' && strItemText[2] == 'E' && strItemText[3] == 'Y' && strItemText[4] == '_')
		{
			if (strItemText == "HKEY_LOCAL_MACHINE")
			{
				return HKEY_LOCAL_MACHINE;
			}
			if (strItemText == "HKEY_CLASSES_ROOT")
			{
				return HKEY_CLASSES_ROOT;
			}
			if (strItemText == "HKEY_CURRENT_USER")
			{
				return HKEY_CURRENT_USER;
			}
			if (strItemText == "HKEY_USERS")
			{
				return HKEY_USERS;
			}
			if (strItemText == "HKEY_CURRENT_CONFIG")
			{
				return HKEY_CURRENT_CONFIG;
			}
		}
	}

	return NULL;
}

void CRegistryTimeDlg::buildFullPath(CTreeCtrl *tc, HTREEITEM hChildItem, char *fullPath)
{
	bool done = false;
	CString retValue = tc->GetItemText(hChildItem);
	while (tc->GetParentItem(hChildItem) != NULL && !done )
	{
		hChildItem = tc->GetParentItem(hChildItem);
		if (hChildItem != NULL)
		{
			CString strItemText = tc->GetItemText(hChildItem);
			if (!(strItemText[0] == 'H' && strItemText[1] == 'K' && strItemText[2] == 'E' && strItemText[3] == 'Y' && strItemText[4] == '_'))
			{
				retValue = strItemText + "\\" + retValue;
			}
			else
			{
				done = true;
			}
		}
	}

	for (int i = 0; i < retValue.GetLength(); i++)
	{
		fullPath[i] = (char)retValue[i];
		fullPath[i + 1] = 0;
	}

}

void CRegistryTimeDlg::OnItemExpanding(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

	UINT uiAction = pNMTreeView->action;
	HTREEITEM hItem = pNMTreeView->itemNew.hItem;

	if (hItem == m_hRoot) 
	{
		*pResult = 0;
		return;
	}

	CTreeCtrl &tc = *GetTreeCtrl();

	if (uiAction == TVE_EXPAND) // is being expanded
	{
		tc.SetItemImage(hItem, 6, 6);

		CString thisOne = tc.GetItemText(hItem);

		int count  = 0;
		HTREEITEM hChildItem = tc.GetChildItem(hItem);
		HTREEITEM hNextItem;
		while (hChildItem != NULL)
		{

			CString thatOne = tc.GetItemText(hChildItem);

			if (getChildCount(&tc, hChildItem) == 0)
			{
				HKEY hKey = backUpToHKEY(&tc, hChildItem);

				int iii;
				iii = 0;

				char *fullPath = (char *)calloc(1, 1500);
				buildFullPath(&tc, hChildItem, fullPath);
				populateTreeNode(&tc, hChildItem, hKey, fullPath);
				free(fullPath);
			}

			hNextItem = tc.GetNextItem(hChildItem, TVGN_NEXT);
			hChildItem = hNextItem;
			count++;
		}

		int iii;
		iii = 0;
	}
	else
	{
		tc.SetItemImage(hItem, 5, 5);
	}

	*pResult = 0;
}

void CRegistryTimeDlg::OnClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	UINT uiAction = pNMTreeView->action;

	CTreeCtrl& tc = *GetTreeCtrl();
	CString tmp = tc.GetItemText(pNMTreeView->itemNew.hItem);

	*pResult = 0;
}

int setRegistryKey(HKEY hRootKey, LPCSTR sSubKeyPath, SYSTEMTIME lw)
{
	HKEY hKey;

	SYSTEMTIME oldSysTime;
	GetSystemTime(&oldSysTime);
	SetSystemTime(&lw);

	RegOpenKeyExA(hRootKey, (LPCSTR)sSubKeyPath, 0, KEY_SET_VALUE, &hKey);
	RegSetValueEx(hKey, L"NewValue", 0, REG_BINARY, NULL, 0);
	RegDeleteValue(hKey, L"NewValue");
	RegCloseKey(hKey);

	SetSystemTime(&oldSysTime);

	return ERROR_SUCCESS;
}

void recurse(char *path, HKEY hKey, SYSTEMTIME st)
{
	char *fullPath = (char *)calloc(1, 1500);
	WCHAR *achClass = (WCHAR *)calloc(sizeof(WCHAR), 1500);
	WCHAR *achKey = (WCHAR *)calloc(sizeof(WCHAR), 1500);
	WCHAR *s = (WCHAR *)calloc(sizeof(WCHAR), 1500);

	for (int i = 0; i < (int)strlen(path); i++)
	{
		s[i] = (WCHAR)path[i];
		s[i + 1] = 0;
	}

	HKEY _hKey;
	int ret = RegOpenKeyEx(hKey, s, 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, &_hKey);
	if (_hKey == NULL)
	{
		goto FreeMem;
	}

	DWORD cSubKeys = 0;
	DWORD cchClassName = 255;
	DWORD cbMaxSubKey, cchMaxClass, cValues, cchMaxValue, cbMaxValueData, cbSecurityDescriptor;
	FILETIME ftLastWriteTime;

	int result = RegQueryInfoKey(
		_hKey,                  // key handle 
		achClass,                // buffer for class name 
		&cchClassName,           // size of class string 
		NULL,                    // reserved 
		&cSubKeys,               // number of subkeys 
		&cbMaxSubKey,            // longest subkey size 
		&cchMaxClass,            // longest class string 
		&cValues,                // number of values for this key 
		&cchMaxValue,            // longest value name 
		&cbMaxValueData,         // longest value data 
		&cbSecurityDescriptor,   // security descriptor 
		&ftLastWriteTime);       // last write time

	for (int i = 0; i<(int)cSubKeys; i++)
	{
		FILETIME ftLastWriteTime;
		DWORD cbName;
		cbName = 500;

		RegEnumKeyEx(_hKey, i,
			achKey,
			&cbName,
			NULL,
			NULL,
			NULL,
			&ftLastWriteTime);

		strcpy(fullPath, path);
		strcat(fullPath, "\\");
		int j = 0, k = strlen(fullPath);
		while (achKey[j] != 0)
		{
			fullPath[k] = (char)achKey[j++];
			fullPath[++k] = 0;
		}

		if (fullPath[0] == '\\')
		{
			memmove(fullPath, &fullPath[1], strlen(fullPath));
		}

		memset(s, 1500 * sizeof(WCHAR), 0);

		for (int i = 0; i < (int)strlen(fullPath); i++)
		{
			s[i] = (WCHAR)fullPath[i];
			s[i + 1] = 0;
		}

		setRegistryKey(hKey, fullPath, st);
		recurse(fullPath, hKey, st);
	}

	RegCloseKey(_hKey);

FreeMem:
	free(fullPath);
	free(achClass);
	free(achKey);
	free(s);
}

void CRegistryTimeDlg::OnRightClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	USES_CONVERSION;

	CTreeCtrl& tc = *GetTreeCtrl();
	HTREEITEM treeItem = tc.GetSelectedItem();
	CString tmp = tc.GetItemText(treeItem);

	FILETIME ft;
	SYSTEMTIME st;
	char *displayString = (char *)calloc(1, 1500);
	getTimeAndDate(treeItem, &ft, &st, displayString);
	free(displayString);

	if (st.wDay < 1 || st.wDay > 31)
	{
		AfxMessageBox(L"Permissions don't allow access to that key.");
		return;
	}

	if (st.wMonth < 1 || st.wMonth > 12)
	{
		AfxMessageBox(L"Permissions don't allow access to that key.");
		return;
	}

	CDateTimeDlg dlg;
	dlg.st = st;
	if (dlg.DoModal() == IDOK)
	{
		CTreeCtrl& tc = *GetTreeCtrl();
		char *fullPath = (char *)calloc(1, 1500);
		char *timeDatePath = (char *)calloc(1, 1500);
		buildFullPath(&tc, treeItem, fullPath);

		setRegistryKey(backUpToHKEY(&tc, treeItem), fullPath, dlg.st2);

		sprintf(timeDatePath, "%d/%d/%d %02d:%02d:%02d", dlg.st2.wMonth, dlg.st2.wDay, dlg.st2.wYear, dlg.st2.wHour, dlg.st2.wMinute, dlg.st2.wSecond);
		SetDlgItemText(IDC_DATETIME, A2W(timeDatePath));

		CButton *m_ctlCheck = (CButton*)GetDlgItem(IDC_RECURSETREE);
		int ChkBox = m_ctlCheck->GetCheck();
		if (ChkBox == BST_CHECKED)
		{
			recurse(fullPath, backUpToHKEY(&tc, treeItem), dlg.st2);
		}

		free(fullPath);
		free(timeDatePath);
	}

	*pResult = 0;
}

void CRegistryTimeDlg::getTimeAndDate(HTREEITEM item, FILETIME *ft, SYSTEMTIME *st, char *displayString)
{
	CTreeCtrl& tc = *GetTreeCtrl();

	HKEY hKey = backUpToHKEY(&tc, item);

	char *fullPath = (char *)calloc(1, 1500);

	buildFullPath(&tc, item, fullPath);

	FILETIME tme;

	getTimeAndDateFromRegistryKey(hKey, fullPath, &tme);
	// Convert the last-write time to local time.
	FileTimeToSystemTime(&tme, st);

	*ft = tme;

	if (tme.dwHighDateTime == 0 &&
		tme.dwLowDateTime == 0)
	{
		strcpy(displayString, "N/A");
	}
	else
	{
		sprintf(displayString, "%d/%d/%d %02d:%02d:%02d", st->wMonth, st->wDay, st->wYear, st->wHour, st->wMinute, st->wSecond);
	}

	free(fullPath);
}

void CRegistryTimeDlg::OnSelChanged(NMHDR *pNMHDR, LRESULT *pResult)
{
	USES_CONVERSION;

	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	UINT uiAction = pNMTreeView->action;

	if (uiAction == 1)
	{
		FILETIME ft;
		SYSTEMTIME st;
		char *displayString = (char *)calloc(1, 1500);
		getTimeAndDate(pNMTreeView->itemNew.hItem, &ft, &st, displayString);
		SetDlgItemText(IDC_DATETIME, A2W(displayString));
		free(displayString);
	}

	*pResult = 0;
}

void CRegistryTimeDlg::getTimeAndDateFromRegistryKey(HKEY hKey, char *fullPath, FILETIME *tme)
{
	HKEY _hKey;

	tme->dwHighDateTime = tme->dwLowDateTime = 0;

	WCHAR *s = (WCHAR *)calloc(sizeof(WCHAR), 1500);
	for (int i = 0; i < (int)strlen(fullPath); i++)
	{
		s[i] = (WCHAR)fullPath[i];
	}

	int ret = RegOpenKeyEx(hKey, s, 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, &_hKey);
	if (_hKey == NULL)
	{
		return;
	}

	WCHAR *achClass = (WCHAR *)calloc(sizeof(WCHAR), 1500);
	WCHAR *achKey = (WCHAR *)calloc(sizeof(WCHAR), 1500);

	DWORD cSubKeys = 0;
	DWORD cchClassName = 255;
	DWORD cbMaxSubKey, cchMaxClass, cValues, cchMaxValue, cbMaxValueData, cbSecurityDescriptor;

	int result = RegQueryInfoKey(
		_hKey,                  // key handle 
		achClass,                // buffer for class name 
		&cchClassName,           // size of class string 
		NULL,                    // reserved 
		&cSubKeys,               // number of subkeys 
		&cbMaxSubKey,            // longest subkey size 
		&cchMaxClass,            // longest class string 
		&cValues,                // number of values for this key 
		&cchMaxValue,            // longest value name 
		&cbMaxValueData,         // longest value data 
		&cbSecurityDescriptor,   // security descriptor 
		tme);       // last write time

	free(achClass);
	free(achKey);
}


void CRegistryTimeDlg::OnBnClickedBail()
{
	PostQuitMessage(0);
}

void CRegistryTimeDlg::OnTimer(UINT_PTR nIDEvent)
{
	timerKicked = true;

	pSplash->startMe();
	InitTree();

	KillTimer(nIDEvent);

	CDialogEx::OnTimer(nIDEvent);
}
