#pragma once


// CDateTimeDlg dialog

class CDateTimeDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CDateTimeDlg)

public:
	CDateTimeDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDateTimeDlg();

// Dialog Data
	enum { IDD = IDD_SELECTDATETIME };

	SYSTEMTIME st, st2;
	int month, day, year, hour, minute, second;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedOk();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnPaint();
};
