
// MainFrm.cpp: CMainFrame 类的实现
//

#include "pch.h"
#include "framework.h"
#include "BubbleBattle.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
END_MESSAGE_MAP()

// CMainFrame 构造/析构

CMainFrame::CMainFrame() noexcept
{
	// TODO: 在此添加成员初始化代码
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	// Remove default toolbar/menu. We'll keep a simple fixed-size window.
	SetWindowTextW(L"Bubble Battle");
	// Remove menu if present
	SetMenu(NULL);

	// Ensure no toolbar shown: if created, destroy
	if (::IsWindow(m_wndToolBar.m_hWnd)) {
		m_wndToolBar.ShowWindow(SW_HIDE);
	}

	// Set desired client size ~980x680 and center window
	const int clientW = 980;
	const int clientH = 680;
	// Get current window styles
	DWORD dwStyle = (DWORD)GetStyle();
	DWORD dwExStyle = (DWORD)GetExStyle();
	RECT rc = { 0, 0, clientW, clientH };
	::AdjustWindowRectEx(&rc, dwStyle, FALSE, dwExStyle);
	int winW = rc.right - rc.left;
	int winH = rc.bottom - rc.top;
	int screenW = ::GetSystemMetrics(SM_CXSCREEN);
	int screenH = ::GetSystemMetrics(SM_CYSCREEN);
	int x = (screenW - winW) / 2;
	int y = (screenH - winH) / 2;
	SetWindowPos(NULL, x, y, winW, winH, SWP_NOZORDER | SWP_SHOWWINDOW);

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式

	cs.style = WS_OVERLAPPED | WS_CAPTION | FWS_ADDTOTITLE
		 | WS_MINIMIZEBOX | WS_SYSMENU;

	return TRUE;
}

// CMainFrame 诊断

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}
#endif //_DEBUG


// CMainFrame 消息处理程序

