// DateTimeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "RegistryTime.h"
#include "DateTimeDlg.h"
#include "afxdialogex.h"

// CDateTimeDlg dialog

IMPLEMENT_DYNAMIC(CDateTimeDlg, CDialogEx)

CDateTimeDlg::CDateTimeDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDateTimeDlg::IDD, pParent)
{
}

CDateTimeDlg::~CDateTimeDlg()
{
}

void CDateTimeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDateTimeDlg, CDialogEx)
	ON_BN_CLICKED(IDCANCEL, &CDateTimeDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDOK, &CDateTimeDlg::OnBnClickedOk)
	ON_WM_TIMER()
	ON_WM_PAINT()
END_MESSAGE_MAP()

// CDateTimeDlg message handlers

void CDateTimeDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnCancel();
}

void CDateTimeDlg::OnBnClickedOk()
{
	CDateTimeCtrl *dtp1 = (CDateTimeCtrl *)GetDlgItem(IDC_DATETIMEPICKER1);
	CDateTimeCtrl *dtp2 = (CDateTimeCtrl *)GetDlgItem(IDC_DATETIMEPICKER2);

	SYSTEMTIME _st1;
	dtp1->GetTime(&_st1);
	SYSTEMTIME _st2;
	dtp2->GetTime(&_st2);

//	int month, day, year, hour, minute, second;
	month = _st1.wMonth;
	day = _st1.wDay;
	year = _st1.wYear;

	hour = _st2.wHour;
	minute = _st2.wMinute;
	second = _st2.wSecond;

	st2.wMonth = month;
	st2.wDay = day;
	st2.wYear = year;
	st2.wHour = hour;
	st2.wMinute = minute;
	st2.wSecond = second;

	CDialogEx::OnOK();
}

void CDateTimeDlg::OnTimer(UINT_PTR nIDEvent)
{
	KillTimer(nIDEvent);

	CDateTimeCtrl *dtp1 = (CDateTimeCtrl *)GetDlgItem(IDC_DATETIMEPICKER1);
	CDateTimeCtrl *dtp2 = (CDateTimeCtrl *)GetDlgItem(IDC_DATETIMEPICKER2);

	dtp1->SetTime(st);
	dtp2->SetTime(st);

	CDialogEx::OnTimer(nIDEvent);
}


void CDateTimeDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	SetTimer(1, 200, NULL);

}
