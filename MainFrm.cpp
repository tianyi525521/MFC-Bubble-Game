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

    // 设置窗口标题并移除默认菜单。
    SetWindowTextW(L"Bubble Battle");
    SetMenu(NULL);

    // 如果工具栏已经创建，则将其隐藏。
    if (::IsWindow(m_wndToolBar.m_hWnd))
    {
        m_wndToolBar.ShowWindow(SW_HIDE);
    }

    // 设置游戏窗口的客户区大小为 1280 x 800，并让窗口居中显示。
    const int clientW = 1280;
    const int clientH = 800;

    DWORD dwStyle = (DWORD)GetStyle();
    DWORD dwExStyle = (DWORD)GetExStyle();

    RECT rc = { 0, 0, clientW, clientH };
    ::AdjustWindowRectEx(&rc, dwStyle, FALSE, dwExStyle);

    const int winW = rc.right - rc.left;
    const int winH = rc.bottom - rc.top;
    const int screenW = ::GetSystemMetrics(SM_CXSCREEN);
    const int screenH = ::GetSystemMetrics(SM_CYSCREEN);
    const int x = (screenW - winW) / 2;
    const int y = (screenH - winH) / 2;

    SetWindowPos(
        NULL,
        x,
        y,
        winW,
        winH,
        SWP_NOZORDER | SWP_SHOWWINDOW);

    // 将最大化命令放入消息队列，等主窗口创建完成后再执行。
    // 这样可以避免 MFC 后续的初始化流程把窗口恢复成原来的小尺寸。
    PostMessage(WM_SYSCOMMAND, SC_MAXIMIZE, 0);

    return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
    if (!CFrameWnd::PreCreateWindow(cs))
        return FALSE;

    // 保留普通主窗口样式，并允许最小化、最大化和拖动边框缩放。
    cs.style = WS_OVERLAPPED
        | WS_CAPTION
        | FWS_ADDTOTITLE
        | WS_MINIMIZEBOX
        | WS_MAXIMIZEBOX
        | WS_THICKFRAME
        | WS_SYSMENU;

    return TRUE;
}

void CMainFrame::SwitchToView(CRuntimeClass* pViewClass)
{
    CView* pOldView = GetActiveView();
    if (pOldView == nullptr)
        return;

    if (pOldView->IsKindOf(pViewClass))
        return;

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
        if (pTargetView == nullptr)
            return;

        CRect rcClient;
        GetClientRect(&rcClient);

        UINT nID = AFX_IDW_PANE_FIRST + 1;

        // 最后一个参数是 nullptr，不使用 CCreateContext。
        if (!pTargetView->Create(
            nullptr,
            nullptr,
            AFX_WS_DEFAULT_VIEW,
            rcClient,
            this,
            nID,
            nullptr))
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
    //    （MFC 的 CreateView 会自动调用，我们使用 Create，所以手动调用）
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
#endif // _DEBUG

// CMainFrame 消息处理程序
