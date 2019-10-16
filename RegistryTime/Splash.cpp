// Splash.cpp : implementation file
//

#include "stdafx.h"
#include "RegistryTime.h"
#include "Splash.h"
#include "afxdialogex.h"


// CSplash dialog

IMPLEMENT_DYNAMIC(CSplash, CDialogEx)

CSplash::CSplash(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSplash::IDD, pParent)
{
	timerKicked = false;
}

CSplash::~CSplash()
{
}

void CSplash::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

int quit = 0;
CWnd *pMe;
HANDLE thread;
int current = 0;

CProgressCtrl *prg;

int x1 = 50;
int c = 260;
int x2 = 580;
int y2 = 280;

DWORD WINAPI ThreadProc(LPVOID lpParam)
{

	while (!IsWindow(pMe->m_hWnd))
	{
		::Sleep(1);
	}

//	CPaintDC dc(pMe);

	HDC hDC = ::GetDC(pMe->m_hWnd);
	HBRUSH hBrush = ::CreateSolidBrush(RGB(80,255,80));
	RECT rect;

	while (!quit)
	{
		current+=2;
		if (current > 200)
		{
			current = 200;
		}

		rect.left = x1;
		rect.top = c;
		rect.right = x2;
		rect.bottom = y2;

		int w = rect.right - rect.left + 1;
		rect.right = rect.left + (current * w / 200 );

		::FillRect(hDC, &rect, hBrush);

		::Sleep(200);
	}

	::ReleaseDC(pMe->m_hWnd,hDC);

	return 0;
}

void CSplash::killMe()
{
	quit = true;
	::Sleep(250);
}

void CSplash::startMe()
{
	if (!timerKicked)
	{
		pMe = this;

		DWORD dwThreadId;
		thread = CreateThread(
			NULL,                   // default security attributes
			0,                      // use default stack size  
			ThreadProc,       // thread function name
			NULL,          // argument to thread function 
			0,                      // use default creation flags 
			&dwThreadId);   // returns the thread identifier

		timerKicked = true;
	}
}

BEGIN_MESSAGE_MAP(CSplash, CDialogEx)
	ON_WM_TIMER()
	ON_WM_PAINT()
END_MESSAGE_MAP()

// CSplash message handlers

void CSplash::OnTimer(UINT_PTR nIDEvent)
{

	CDialogEx::OnTimer(nIDEvent);
}

CBitmap bm;

void CSplash::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	// Do not call CDialogEx::OnPaint() for painting messages

	RECT clientRect;
	GetClientRect(&clientRect);
	int w = clientRect.right - clientRect.left;
	int h = clientRect.bottom - clientRect.top;

	BOOL ret = bm.LoadBitmap(IDB_BITMAP1);

	CDC memDC;
	memDC.CreateCompatibleDC(&dc);
	memDC.SelectObject(&bm);

	dc.StretchBlt(0, 0, w+160, h+60, &memDC, 0, 0, 754, 400, SRCCOPY);

	CBrush br(RGB(0, 0, 0));
	RECT emptyRect;
	emptyRect.left = x1 - 2;
	emptyRect.top = c - 2;
	emptyRect.right = x2 + 2;
	emptyRect.bottom = y2 + 2;
	dc.FillRect(&emptyRect, &br);
}
