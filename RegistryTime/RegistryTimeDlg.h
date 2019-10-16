
// RegistryTimeDlg.h : header file
//

#pragma once

#include "NtRegistry.h"

#define FAKEDITEM L"faked item"

// CRegistryTimeDlg dialog
class CRegistryTimeDlg : public CDialogEx
{
// Construction
public:
	CRegistryTimeDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_REGISTRYTIME_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	void InitTree();
	CTreeCtrl * GetTreeCtrl();

	HTREEITEM	m_hRoot;
	HTREEITEM	m_hHKLM;
	HTREEITEM	m_hHKCU;
	HTREEITEM	m_hHKU;
	HTREEITEM	m_hHKCR;
	HTREEITEM	m_hHKCC;

	CNtRegistry m_ntReg;

	CString m_csSID;

	CImageList  m_pImageTree;

	int getChildCount(CTreeCtrl *tc, HTREEITEM item);
	HKEY backUpToHKEY(CTreeCtrl *tc, HTREEITEM item);
	void buildFullPath(CTreeCtrl *tc, HTREEITEM hChildItem, char *fullPath);

	void getTimeAndDateFromRegistryKey(HKEY hKey, char *fullPath, FILETIME *tme);
	void getTimeAndDate(HTREEITEM item, FILETIME *ft, SYSTEMTIME *st, char *displayString);

	bool timerKicked;

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnItemExpanded(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnItemExpanding(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnClick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnRightClick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnSelChanged(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedBail();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
