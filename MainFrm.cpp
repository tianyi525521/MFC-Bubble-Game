
// MainFrm.cpp: CMainFrame 类的实现
//

#include "pch.h"
#include "framework.h"
#include "BubbleBattle.h"

#include "MainFrm.h"
#include "BubbleGameUIView.h"        
#include "BubbleBattleView.h"

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
    : m_pUIView(nullptr)
    , m_pGameView(nullptr)
{
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

void CMainFrame::SwitchToView(CRuntimeClass* pViewClass)
{
    CView* pOldView = GetActiveView();
    if (pOldView == nullptr) return;
    if (pOldView->IsKindOf(pViewClass)) return;

    // ============================================================
    // 1. 先查缓存，目标视图可能已创建过
    // ============================================================
    CView* pTargetView = nullptr;
    if (pViewClass == RUNTIME_CLASS(CBubbleGameUIView))
        pTargetView = m_pUIView;
    else if (pViewClass == RUNTIME_CLASS(CBubbleBattleView))
        pTargetView = m_pGameView;

    // ============================================================
    // 2. 首次切换：创建视图（不用文档 context）
    // ============================================================
    BOOL bNewlyCreated = FALSE;
    if (pTargetView == nullptr)
    {
        pTargetView = (CView*)pViewClass->CreateObject();
        if (pTargetView == nullptr) return;

        CRect rcClient;
        GetClientRect(&rcClient);

        UINT nID = AFX_IDW_PANE_FIRST + 1;

        // ★★★ 最后一个参数是 nullptr，不用 CCreateContext
        if (!pTargetView->Create(nullptr, nullptr, AFX_WS_DEFAULT_VIEW,
            rcClient, this, nID, nullptr))
        {
            delete pTargetView;
            return;
        }

        pTargetView->ShowWindow(SW_HIDE);
        bNewlyCreated = TRUE;

        if (pViewClass == RUNTIME_CLASS(CBubbleGameUIView))
            m_pUIView = pTargetView;
        else if (pViewClass == RUNTIME_CLASS(CBubbleBattleView))
            m_pGameView = pTargetView;
    }

    // ============================================================
    // 3. 切换显示（不销毁任何视图）
    // ============================================================
    pOldView->ShowWindow(SW_HIDE);
    pTargetView->ShowWindow(SW_SHOW);
    SetActiveView(pTargetView);
    pTargetView->SetFocus();
    RecalcLayout();
    pTargetView->Invalidate();

    // ============================================================
    // 4. 新创建的视图，调用一次 OnInitialUpdate
    //    （MFC 的 CreateView 会自动调，我们用 Create 所以手动调）
    // ============================================================
    if (bNewlyCreated)
    {
        pTargetView->OnInitialUpdate();
    }
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

