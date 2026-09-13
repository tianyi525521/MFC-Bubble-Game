#include "pch.h"
#include "framework.h"
#include "MainFrm.h"
#include "GameConfig.h"

#ifndef SHARED_HANDLERS
#include "BubbleBattle.h"
#endif

#include "BubbleBattleDoc.h"
#include "BubbleBattleView.h"
#include "BubbleGameUIView.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace
{
    // 菜单最初就是按照 980 x 680 设计的。
    // 绘制时始终使用这套逻辑坐标，再整体等比例映射到实际窗口。
    constexpr int DESIGN_WIDTH = 980;
    constexpr int DESIGN_HEIGHT = 680;

    // 玩法说明窗口同样按逻辑坐标设计，再整体等比例映射到实际窗口。
    constexpr int HELP_DESIGN_WIDTH = 720;
    constexpr int HELP_DESIGN_HEIGHT = 440;

    // 下面两个辅助函数定义在文件后面，这里先声明，供玩法说明窗口调用。
    double GetHelpWindowScale(CWnd* pOwner);
    void SetDesignCoordinateSystemFor(
        CDC* pDC,
        const CRect& clientRect,
        int designWidth,
        int designHeight);

    // 生成“按设计坐标定尺寸”的字体。
    // 直接用 CreatePointFont 时，字号会先按系统 DPI 放大一次，再被窗口映射放大一次；
    // 在 125% / 150% 缩放的屏幕上文字会比版式大出一截，顶部或底部被矩形裁掉。
    // 这里固定按 96 DPI 折算字高，让字体只随窗口映射缩放，始终和版式保持设计时的比例。
    void CreateDesignFont(CFont& font, int pointSize10)
    {
        font.CreateFont(
            -MulDiv(pointSize10, 96, 720),
            0, 0, 0, FW_NORMAL,
            FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
            _T("Microsoft YaHei"));
    }

    HWND g_helpWindowHandle = nullptr;

    class CGameHelpWindow : public CFrameWnd
    {
    public:
        BOOL CreateHelpWindow(CWnd* pOwner)
        {
            // 客户区按和主界面一致的比例换算，再补上标题栏和边框。
            // 这样在高 DPI 或高分屏上，说明文字的大小与主界面相当，不会显得又小又挤。
            const double scale = GetHelpWindowScale(pOwner);
            const int clientWidth = (int)(HELP_DESIGN_WIDTH * scale + 0.5);
            const int clientHeight = (int)(HELP_DESIGN_HEIGHT * scale + 0.5);

            const DWORD windowStyle =
                WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MAXIMIZEBOX;

            CRect windowRect(0, 0, clientWidth, clientHeight);
            ::AdjustWindowRectEx(&windowRect, windowStyle, FALSE, 0);

            CRect ownerRect;
            pOwner->GetWindowRect(&ownerRect);

            windowRect.OffsetRect(
                ownerRect.left + (ownerRect.Width() - windowRect.Width()) / 2,
                ownerRect.top + (ownerRect.Height() - windowRect.Height()) / 2);

            return Create(
                nullptr,
                _T("玩法说明"),
                windowStyle,
                windowRect,
                pOwner);
        }

    protected:
        afx_msg BOOL OnEraseBkgnd(CDC* /*pDC*/)
        {
            // 背景在 OnPaint 中一次性绘制，避免调整窗口大小时闪烁。
            return TRUE;
        }

        afx_msg void OnPaint()
        {
            CPaintDC dc(this);

            CRect clientRect;
            GetClientRect(&clientRect);

            // 先把实际窗口铺满底色，缩放后四周的留白与页面颜色一致。
            dc.FillSolidRect(clientRect, RGB(246, 249, 255));
            dc.SetBkMode(TRANSPARENT);

            const int savedDC = dc.SaveDC();

            // 与主界面一致：先按逻辑坐标绘制，再整体等比缩放到实际窗口。
            // 文字会随窗口同步放大，不会撑破卡片而被截断。
            SetDesignCoordinateSystemFor(
                &dc, clientRect, HELP_DESIGN_WIDTH, HELP_DESIGN_HEIGHT);

            const int W = HELP_DESIGN_WIDTH;

            // ---- 大标题 ----
            CFont titleFont;
            CreateDesignFont(titleFont, 240);
            CFont* oldFont = dc.SelectObject(&titleFont);
            dc.SetTextColor(RGB(38, 82, 155));

            CRect titleRect(0, 24, W, 86);
            dc.DrawText(_T("玩法说明"), &titleRect,
                DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            dc.SelectObject(oldFont);

            // ---- 两张玩法卡片 ----
            CRect player1Card(60, 100, W - 60, 230);
            CRect player2Card(60, 250, W - 60, 380);

            CBrush cardBrush(RGB(255, 255, 255));
            CPen cardPen(PS_SOLID, 2, RGB(170, 195, 230));
            CBrush* oldBrush = dc.SelectObject(&cardBrush);
            CPen* oldPen = dc.SelectObject(&cardPen);
            dc.RoundRect(player1Card, CPoint(22, 22));
            dc.RoundRect(player2Card, CPoint(22, 22));
            dc.SelectObject(oldBrush);
            dc.SelectObject(oldPen);

            CFont playerFont;
            CreateDesignFont(playerFont, 175);
            oldFont = dc.SelectObject(&playerFont);
            dc.SetTextColor(RGB(45, 80, 145));

            CRect player1Title(
                player1Card.left, player1Card.top + 20,
                player1Card.right, player1Card.top + 62);
            dc.DrawText(_T("玩家 1"), &player1Title,
                DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            CRect player2Title(
                player2Card.left, player2Card.top + 20,
                player2Card.right, player2Card.top + 62);
            dc.DrawText(_T("玩家 2"), &player2Title,
                DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            // 操作方式单独占一行，与玩家标题拉开间距。
            CFont detailFont;
            CreateDesignFont(detailFont, 140);
            dc.SelectObject(&detailFont);
            dc.SetTextColor(RGB(80, 100, 135));

            CRect player1Detail(
                player1Card.left + 24, player1Card.top + 66,
                player1Card.right - 24, player1Card.top + 110);
            dc.DrawText(_T("方向键移动    Enter 放置炸弹"), &player1Detail,
                DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            CRect player2Detail(
                player2Card.left + 24, player2Card.top + 66,
                player2Card.right - 24, player2Card.top + 110);
            dc.DrawText(_T("W A S D 移动    空格键放置炸弹"), &player2Detail,
                DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            dc.SelectObject(oldFont);

            dc.RestoreDC(savedDC);
        }

        virtual void PostNcDestroy() override
        {
            g_helpWindowHandle = nullptr;
            // CFrameWnd 的默认实现会自动释放动态创建的窗口对象。
            CFrameWnd::PostNcDestroy();
        }

        DECLARE_MESSAGE_MAP()
    };

    BEGIN_MESSAGE_MAP(CGameHelpWindow, CFrameWnd)
        ON_WM_ERASEBKGND()
        ON_WM_PAINT()
    END_MESSAGE_MAP()

    void ShowGameHelpWindow(CWnd* pOwner)
    {
        if (::IsWindow(g_helpWindowHandle))
        {
            ::ShowWindow(g_helpWindowHandle, SW_RESTORE);
            ::SetForegroundWindow(g_helpWindowHandle);
            return;
        }

        CGameHelpWindow* pHelpWindow = new CGameHelpWindow();
        if (!pHelpWindow->CreateHelpWindow(pOwner))
        {
            delete pHelpWindow;
            AfxMessageBox(_T("无法创建玩法说明窗口。"));
            return;
        }

        g_helpWindowHandle = pHelpWindow->GetSafeHwnd();
        pHelpWindow->ShowWindow(SW_SHOW);
        pHelpWindow->UpdateWindow();
    }

    CRect GetDesignRect()
    {
        return CRect(0, 0, DESIGN_WIDTH, DESIGN_HEIGHT);
    }

    void SetDesignCoordinateSystem(CDC* pDC, const CRect& clientRect)
    {
        SetDesignCoordinateSystemFor(pDC, clientRect, DESIGN_WIDTH, DESIGN_HEIGHT);
    }

    // 玩法说明窗口的缩放比例：既不小于当前显示器的缩放（高 DPI 屏幕文字够大），
    // 也不小于主界面自身的缩放（说明文字与主界面大小相当）。
    double GetHelpWindowScale(CWnd* pOwner)
    {
        double dpiScale = 1.0;

        HWND hwnd = (pOwner != nullptr) ? pOwner->GetSafeHwnd() : nullptr;
        HDC hdc = ::GetDC(hwnd);
        if (hdc != nullptr)
        {
            const int dpi = ::GetDeviceCaps(hdc, LOGPIXELSY);
            ::ReleaseDC(hwnd, hdc);
            if (dpi > 0)
                dpiScale = static_cast<double>(dpi) / 96.0;
        }

        double uiScale = 1.0;
        if (pOwner != nullptr)
        {
            CRect ownerClient;
            pOwner->GetClientRect(&ownerClient);
            if (ownerClient.Width() > 0 && ownerClient.Height() > 0)
            {
                const double scaleX = static_cast<double>(ownerClient.Width()) / DESIGN_WIDTH;
                const double scaleY = static_cast<double>(ownerClient.Height()) / DESIGN_HEIGHT;
                uiScale = (scaleX < scaleY) ? scaleX : scaleY;
            }
        }

        return (dpiScale > uiScale) ? dpiScale : uiScale;
    }

    void SetDesignCoordinateSystemFor(
        CDC* pDC,
        const CRect& clientRect,
        int designWidth,
        int designHeight)
    {
        if (clientRect.Width() <= 0 || clientRect.Height() <= 0 ||
            designWidth <= 0 || designHeight <= 0)
            return;

        const double scaleX = static_cast<double>(clientRect.Width()) / designWidth;
        const double scaleY = static_cast<double>(clientRect.Height()) / designHeight;
        const double scale = (scaleX < scaleY) ? scaleX : scaleY;

        int viewportWidth = static_cast<int>(designWidth * scale + 0.5);
        int viewportHeight = static_cast<int>(designHeight * scale + 0.5);
        if (viewportWidth < 1) viewportWidth = 1;
        if (viewportHeight < 1) viewportHeight = 1;
        const int offsetX = (clientRect.Width() - viewportWidth) / 2;
        const int offsetY = (clientRect.Height() - viewportHeight) / 2;

        pDC->SetMapMode(MM_ANISOTROPIC);
        pDC->SetWindowExt(designWidth, designHeight);
        pDC->SetViewportExt(viewportWidth, viewportHeight);
        pDC->SetViewportOrg(offsetX, offsetY);
    }

    CPoint ClientPointToDesignPoint(const CRect& clientRect, CPoint point)
    {
        if (clientRect.Width() <= 0 || clientRect.Height() <= 0)
            return point;

        const double scaleX = static_cast<double>(clientRect.Width()) / DESIGN_WIDTH;
        const double scaleY = static_cast<double>(clientRect.Height()) / DESIGN_HEIGHT;
        const double scale = (scaleX < scaleY) ? scaleX : scaleY;

        int viewportWidth = static_cast<int>(DESIGN_WIDTH * scale + 0.5);
        int viewportHeight = static_cast<int>(DESIGN_HEIGHT * scale + 0.5);
        if (viewportWidth < 1) viewportWidth = 1;
        if (viewportHeight < 1) viewportHeight = 1;
        const int offsetX = (clientRect.Width() - viewportWidth) / 2;
        const int offsetY = (clientRect.Height() - viewportHeight) / 2;

        return CPoint(
            MulDiv(point.x - offsetX, DESIGN_WIDTH, viewportWidth),
            MulDiv(point.y - offsetY, DESIGN_HEIGHT, viewportHeight));
    }
}

IMPLEMENT_DYNCREATE(CBubbleGameUIView, CView)

BEGIN_MESSAGE_MAP(CBubbleGameUIView, CView)
    ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
    ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
    ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
    ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

// ============================================================
// 构造 / 析构
// ============================================================

CBubbleGameUIView::CBubbleGameUIView() noexcept
{
    m_bmpCharPlayer.LoadBitmap(IDB_PLAYER_UP);
    m_bmpCharEnemy0.LoadBitmap(IDB_ENEMY0_UP);
    m_bmpCharEnemy1.LoadBitmap(IDB_ENEMY1_UP);
    m_bmpCharEnemy2.LoadBitmap(IDB_ENEMY2_UP);
}

CBubbleGameUIView::~CBubbleGameUIView()
{
}

// ============================================================
// 窗口
// ============================================================

BOOL CBubbleGameUIView::PreCreateWindow(CREATESTRUCT& cs)
{
    return CView::PreCreateWindow(cs);
}

// ============================================================
// 总绘制
// ============================================================

void CBubbleGameUIView::OnDraw(CDC* pDC)
{
    CRect clientRect;
    GetClientRect(&clientRect);

    // 先填满实际窗口，宽屏下两侧也保持与页面相同的背景颜色。
    pDC->FillSolidRect(clientRect, RGB(246, 249, 255));

    const int savedDC = pDC->SaveDC();
    SetDesignCoordinateSystem(pDC, clientRect);

    switch (gameState)
    {
    case GameState::MAIN_MENU:
        DrawMainMenu(pDC);
        break;
    case GameState::MODE_SELECT:
        DrawModeSelect(pDC);
        break;
    case GameState::CHARACTER_SELECT:
        DrawCharacterSelect(pDC);
        break;
    case GameState::READY:
        DrawReadyPage(pDC);
        break;
    }

    pDC->RestoreDC(savedDC);
}

// ============================================================
// 公共背景
// ============================================================

void CBubbleGameUIView::DrawPageBackground(CDC* pDC)
{
    CRect clientRect = GetDesignRect();

    // 整体浅背景
    pDC->FillSolidRect(clientRect, RGB(246, 249, 255));
    pDC->SetBkMode(TRANSPARENT);
}

// ============================================================
// 公共标题
// ============================================================

void CBubbleGameUIView::DrawTitle(CDC* pDC, const CString& title, int y)
{
    CRect clientRect = GetDesignRect();

    CFont font;
    CreateDesignFont(font, 300);

    CFont* oldFont = pDC->SelectObject(&font);
    pDC->SetTextColor(RGB(35, 82, 155));

    CRect rect(0, y, clientRect.Width(), y + 65);
    pDC->DrawText(title, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    pDC->SelectObject(oldFont);
}

// ============================================================
// 公共页眉（大标题 + 副标题）
// ============================================================

void CBubbleGameUIView::DrawHeader(CDC* pDC, const CString& title, const CString& subtitle)
{
    CRect clientRect = GetDesignRect();
    int W = clientRect.Width();
    int H = clientRect.Height();

    pDC->SetBkMode(TRANSPARENT);

    // ---- 大标题 ----
    CFont titleFont;
    CreateDesignFont(titleFont, 320);

    CFont* oldFont = pDC->SelectObject(&titleFont);
    pDC->SetTextColor(RGB(38, 82, 155));

    // 标题区域要比字体本身更高，否则在整体放大后中文顶部或底部会被裁切。
    CRect titleRect(0, (int)(H * 0.05), W, (int)(H * 0.20));
    pDC->DrawText(title, &titleRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    pDC->SelectObject(oldFont);

    // ---- 副标题 ----
    // 传入空字符串时跳过副标题绘制，页面只保留大标题。
    if (!subtitle.IsEmpty())
    {
        CFont subFont;
        CreateDesignFont(subFont, 130);

        oldFont = pDC->SelectObject(&subFont);
        pDC->SetTextColor(RGB(110, 125, 150));

        // 副标题单独占一行，与主标题之间保留间距。
        CRect subRect(0, (int)(H * 0.20), W, (int)(H * 0.28));
        pDC->DrawText(subtitle, &subRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        pDC->SelectObject(oldFont);
    }
}

// ============================================================
// 公共副标题
// ============================================================

void CBubbleGameUIView::DrawSubtitle(CDC* pDC, const CString& text, int y)
{
    CRect clientRect = GetDesignRect();

    CFont font;
    CreateDesignFont(font, 120);

    CFont* oldFont = pDC->SelectObject(&font);
    pDC->SetTextColor(RGB(105, 125, 155));

    CRect rect(0, y, clientRect.Width(), y + 32);
    pDC->DrawText(text, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    pDC->SelectObject(oldFont);
}

// ============================================================
// 公共按钮
// ============================================================

void CBubbleGameUIView::DrawButton(
    CDC* pDC,
    const CRect& rect,
    const CString& text,
    bool primary,
    bool selected)
{
    COLORREF fillColor;
    COLORREF borderColor;
    COLORREF textColor;

    if (primary)
    {
        fillColor = RGB(72, 127, 220);
        borderColor = RGB(55, 100, 180);
        textColor = RGB(255, 255, 255);
    }
    else
    {
        fillColor = RGB(255, 255, 255);
        borderColor = RGB(130, 165, 220);
        textColor = RGB(45, 85, 145);
    }

    if (selected)
        borderColor = RGB(255, 150, 35);

    CBrush brush(fillColor);
    CPen pen(PS_SOLID, selected ? 4 : 2, borderColor);

    CBrush* oldBrush = pDC->SelectObject(&brush);
    CPen* oldPen = pDC->SelectObject(&pen);

    pDC->RoundRect(rect, CPoint(22, 22));

    CFont font;
    CreateDesignFont(font, 145);
    CFont* oldFont = pDC->SelectObject(&font);

    pDC->SetTextColor(textColor);
    pDC->SetBkMode(TRANSPARENT);

    CRect textRect = rect;
    textRect.DeflateRect(8, 4);
    pDC->DrawText(text, &textRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    if (selected)
    {
        // 静态橙色圆点表示当前选中的操作。
        CBrush markerBrush(RGB(255, 150, 35));
        CPen markerPen(PS_SOLID, 1, RGB(230, 120, 20));
        CBrush* previousBrush = pDC->SelectObject(&markerBrush);
        CPen* previousPen = pDC->SelectObject(&markerPen);
        const int markerY = rect.top + rect.Height() / 2;
        pDC->Ellipse(rect.left + 12, markerY - 6, rect.left + 24, markerY + 6);
        pDC->SelectObject(previousBrush);
        pDC->SelectObject(previousPen);
    }

    pDC->SelectObject(oldFont);
    pDC->SelectObject(oldBrush);
    pDC->SelectObject(oldPen);
}

// ============================================================
// 公共卡片
// ============================================================

void CBubbleGameUIView::DrawCard(CDC* pDC, const CRect& rect, bool selected)
{
    CBrush brush(RGB(255, 255, 255));

    COLORREF borderColor = selected ? RGB(255, 165, 60) : RGB(195, 210, 235);
    int borderWidth = selected ? 4 : 2;

    CPen pen(PS_SOLID, borderWidth, borderColor);

    CBrush* oldBrush = pDC->SelectObject(&brush);
    CPen* oldPen = pDC->SelectObject(&pen);

    pDC->RoundRect(rect, CPoint(24, 24));

    pDC->SelectObject(oldBrush);
    pDC->SelectObject(oldPen);
}

// ============================================================
// 主菜单
// ============================================================

void CBubbleGameUIView::DrawMainMenu(CDC* pDC)
{
    DrawPageBackground(pDC);

    DrawHeader(pDC, _T("泡 泡 堂"), _T("C++ / MFC 课程设计"));

    CRect clientRect = GetDesignRect();
    int W = clientRect.Width();
    int H = clientRect.Height();

    const int buttonWidth = 320;
    const int buttonHeight = 65;
    int centerX = W / 2;

    // 三个按钮在页面中下部均匀分布
    int y1 = (int)(H * 0.40);
    int y2 = (int)(H * 0.55);
    int y3 = (int)(H * 0.70);

    startButton = CRect(centerX - buttonWidth / 2, y1, centerX + buttonWidth / 2, y1 + buttonHeight);
    helpButton = CRect(centerX - buttonWidth / 2, y2, centerX + buttonWidth / 2, y2 + buttonHeight);
    exitButton = CRect(centerX - buttonWidth / 2, y3, centerX + buttonWidth / 2, y3 + buttonHeight);

    DrawButton(pDC, startButton, _T("开始游戏"), true);
    DrawButton(pDC, helpButton, _T("玩法说明"));
    DrawButton(pDC, exitButton, _T("退出游戏"));
}

// ============================================================
// 模式选择
// ============================================================

void CBubbleGameUIView::DrawModeSelect(CDC* pDC)
{
    DrawPageBackground(pDC);

    DrawHeader(pDC, _T("选择游戏模式"), _T(""));

    CRect clientRect = GetDesignRect();
    int W = clientRect.Width();
    int H = clientRect.Height();
    int centerX = W / 2;

    const int cardWidth = 300;
    const int cardHeight = 220;
    const int gap = 90;

    int top = (int)(H * 0.38);
    if (H < 600)
        top = (int)(H * 0.30);

    singleButton = CRect(centerX - gap / 2 - cardWidth, top, centerX - gap / 2, top + cardHeight);
    multiButton = CRect(centerX + gap / 2, top, centerX + gap / 2 + cardWidth, top + cardHeight);

    DrawCard(pDC, singleButton, selectedMode == 1);
    DrawCard(pDC, multiButton, selectedMode == 2);

    // ---- 模式标题 ----
    CFont titleFont;
    CreateDesignFont(titleFont, 190);
    CFont* oldFont = pDC->SelectObject(&titleFont);
    pDC->SetTextColor(RGB(45, 80, 145));

    CRect rect1 = singleButton;
    rect1.top += 55;
    rect1.bottom = rect1.top + 50;
    pDC->DrawText(_T("单人模式"), &rect1, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    CRect rect2 = multiButton;
    rect2.top += 55;
    rect2.bottom = rect2.top + 50;
    pDC->DrawText(_T("双人模式"), &rect2, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    pDC->SelectObject(oldFont);

    // ---- 小说明 ----
    CFont subFont;
    CreateDesignFont(subFont, 120);
    oldFont = pDC->SelectObject(&subFont);
    pDC->SetTextColor(RGB(115, 130, 155));

    CRect sub1 = singleButton;
    sub1.top += 130;
    sub1.bottom = sub1.top + 35;
    pDC->DrawText(_T("挑战电脑玩家"), &sub1, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    CRect sub2 = multiButton;
    sub2.top += 130;
    sub2.bottom = sub2.top + 35;
    pDC->DrawText(_T("本地双人对战"), &sub2, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    pDC->SelectObject(oldFont);

    // 先选择模式，再按确认；返回按钮放在确认按钮右侧。
    const int actionY = (int)(H * 0.78);
    modeConfirmButton = CRect(centerX - 230, actionY, centerX - 20, actionY + 58);
    backButton = CRect(centerX + 20, actionY, centerX + 230, actionY + 58);
    DrawButton(pDC, modeConfirmButton, _T("确认选择"), selectedMode != 0);
    DrawButton(pDC, backButton, _T("返回"));
}

// ============================================================
// 人物选择
// ============================================================

void CBubbleGameUIView::DrawCharacterSelect(CDC* pDC)
{
    DrawPageBackground(pDC);

    CString title;
    if (selectedMode == 1)
        title = _T("选择角色");
    else
        title = (selectingPlayer == 1) ? _T("玩家1选择角色") : _T("玩家2选择角色");

    DrawHeader(pDC, title, _T(""));

    CRect clientRect = GetDesignRect();
    int W = clientRect.Width();
    int H = clientRect.Height();

    const int cardWidth = 150;
    const int cardHeight = 180;
    const int gap = 26;

    int totalWidth = cardWidth * 4 + gap * 3;
    int startX = (clientRect.Width() - totalWidth) / 2;
    int top = (int)(H * 0.36);

    characterButton1 = CRect(startX, top, startX + cardWidth, top + cardHeight);
    characterButton2 = CRect(startX + cardWidth + gap, top, startX + cardWidth * 2 + gap, top + cardHeight);
    characterButton3 = CRect(startX + (cardWidth + gap) * 2, top, startX + (cardWidth + gap) * 2 + cardWidth, top + cardHeight);
    characterButton4 = CRect(startX + (cardWidth + gap) * 3, top, startX + (cardWidth + gap) * 3 + cardWidth, top + cardHeight);

    CRect cards[4] = { characterButton1, characterButton2, characterButton3, characterButton4 };

    int currentSelected = -1;
    if (selectedMode == 1)
        currentSelected = player1Character;
    else
        currentSelected = (selectingPlayer == 1) ? player1Character : player2Character;

    COLORREF colors[4] = { RGB(90, 165, 245), RGB(245, 110, 120), RGB(100, 195, 130), RGB(170, 120, 230) };
    CString names[4] = { _T("角色1"), _T("角色2"), _T("角色3"), _T("角色4") };

    for (int i = 0; i < 4; i++)
    {
        DrawCard(pDC, cards[i], currentSelected == i);

        CBitmap* pCharBmp = nullptr;
        switch (i)
        {
        case 0: pCharBmp = &m_bmpCharPlayer; break;
        case 1: pCharBmp = &m_bmpCharEnemy0; break;
        case 2: pCharBmp = &m_bmpCharEnemy1; break;
        case 3: pCharBmp = &m_bmpCharEnemy2; break;
        }

        if (pCharBmp && pCharBmp->GetSafeHandle() != NULL)
        {
            BITMAP bm;
            pCharBmp->GetBitmap(&bm);

            // 计算图片绘制区域（卡片上方居中，适当放大）
            int avatarSize = 90;
            int cx = cards[i].left + cards[i].Width() / 2;
            int cy = cards[i].top + 70;

            int drawLeft = cx - avatarSize / 2;
            int drawTop = cy - avatarSize / 2;

            CDC memDC;
            memDC.CreateCompatibleDC(pDC);
            CBitmap* pOld = memDC.SelectObject(pCharBmp);
            pDC->TransparentBlt(drawLeft, drawTop, avatarSize, avatarSize,
                &memDC, 0, 0, bm.bmWidth, bm.bmHeight, RGB(255, 255, 255));
            memDC.SelectObject(pOld);
            memDC.DeleteDC();
        }

        CFont font;
        CreateDesignFont(font, 135);
        CFont* oldFont = pDC->SelectObject(&font);

        pDC->SetTextColor(RGB(45, 75, 125));

        CRect nameRect = cards[i];
        nameRect.top += 115;
        nameRect.bottom -= 12;
        pDC->DrawText(names[i], &nameRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        pDC->SelectObject(oldFont);
    }

    int centerX = clientRect.Width() / 2;
    int buttonY = (int)(H * 0.78);

    characterConfirmButton = CRect(centerX - 250, buttonY, centerX - 25, buttonY + 60);
    characterBackButton = CRect(centerX + 25, buttonY, centerX + 250, buttonY + 60);

    DrawButton(pDC, characterConfirmButton, _T("确认选择"), true);
    DrawButton(pDC, characterBackButton, _T("返回"));
}

// ============================================================
// READY 页面
// ============================================================

void CBubbleGameUIView::DrawReadyPage(CDC* pDC)
{
    DrawPageBackground(pDC);

    CRect clientRect = GetDesignRect();
    int W = clientRect.Width();
    int H = clientRect.Height();
    int centerX = W / 2;

    DrawHeader(pDC, _T("准备开始"), _T(""));

    CString modeText = (selectedMode == 1) ? _T("单人模式") : _T("双人模式");

    CString characters[4] = { _T("角色1"), _T("角色2"), _T("角色3"), _T("角色4") };
    CString characterText;

    if (selectedMode == 1)
    {
        if (player1Character >= 0)
            characterText = characters[player1Character];
        else
            characterText = _T("未选择");
    }
    else
    {
        if (player1Character >= 0 && player2Character >= 0)
            characterText.Format(
                _T("%s / %s"),
                characters[player1Character].GetString(),
                characters[player2Character].GetString());
        else
            characterText = _T("未选择");
    }

    // ---- 两张信息卡片 ----
    const int cardWidth = 290;
    const int cardHeight = 190;
    const int gap = 90;

    // 当前只有两张卡片。原代码按三张卡片计算总宽度，导致整体向左偏移。
    int totalWidth = cardWidth * 2 + gap;
    int startX = centerX - totalWidth / 2;
    int cardTop = (int)(H * 0.35);

    CRect modeCard(startX, cardTop, startX + cardWidth, cardTop + cardHeight);
    CRect characterCard(startX + cardWidth + gap, cardTop, startX + cardWidth * 2 + gap, cardTop + cardHeight);

    DrawCard(pDC, modeCard, false);
    DrawCard(pDC, characterCard, false);

    // ---- 小标题字体 ----
    CFont labelFont;
    CreateDesignFont(labelFont, 120);
    CFont* oldFont = pDC->SelectObject(&labelFont);
    pDC->SetTextColor(RGB(120, 135, 160));

    CRect modeLabel = modeCard;
    modeLabel.top += 24;
    modeLabel.bottom = modeLabel.top + 35;
    pDC->DrawText(_T("游戏模式"), &modeLabel, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    CRect characterLabel = characterCard;
    characterLabel.top += 24;
    characterLabel.bottom = characterLabel.top + 35;
    pDC->DrawText(_T("角色配置"), &characterLabel, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    pDC->SelectObject(oldFont);

    // ---- 值字体 ----
    CFont valueFont;
    CreateDesignFont(valueFont, 170);
    oldFont = pDC->SelectObject(&valueFont);
    pDC->SetTextColor(RGB(45, 80, 145));

    CRect modeValue = modeCard;
    modeValue.top += 88;
    modeValue.bottom = modeValue.top + 60;
    pDC->DrawText(modeText, &modeValue, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    CRect characterValue = characterCard;
    // 左侧留给角色贴图，右侧单独显示角色名称，二者不再重叠。
    characterValue.left += 115;
    characterValue.right -= 12;
    characterValue.top += 78;
    characterValue.bottom = characterValue.top + 75;

    // 显示玩家1角色的位图
    if (player1Character >= 0)
    {
        CBitmap* pBmp1 = nullptr;
        switch (player1Character)
        {
        case 0: pBmp1 = &m_bmpCharPlayer; break;
        case 1: pBmp1 = &m_bmpCharEnemy0; break;
        case 2: pBmp1 = &m_bmpCharEnemy1; break;
        case 3: pBmp1 = &m_bmpCharEnemy2; break;
        }

        if (pBmp1 && pBmp1->GetSafeHandle() != NULL)
        {
            BITMAP bm;
            pBmp1->GetBitmap(&bm);
            const int sz = 70;
            const int cx = characterCard.left + 68;
            const int cy = characterCard.top + 115;

            CDC memDC;
            memDC.CreateCompatibleDC(pDC);
            CBitmap* pOld = memDC.SelectObject(pBmp1);
            pDC->TransparentBlt(cx - sz / 2, cy - sz / 2, sz, sz,
                &memDC, 0, 0, bm.bmWidth, bm.bmHeight, RGB(255, 255, 255));
            memDC.SelectObject(pOld);
            memDC.DeleteDC();
        }
    }

    pDC->DrawText(characterText, &characterValue, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    pDC->SelectObject(oldFont);

    // ---- 底部按钮 ----
    const int buttonWidth = 230;
    const int buttonHeight = 62;
    const int buttonGap = 40;
    int buttonY = (int)(H * 0.73);

    readyStartButton = CRect(centerX - buttonGap / 2 - buttonWidth, buttonY,
        centerX - buttonGap / 2, buttonY + buttonHeight);
    readyBackButton = CRect(centerX + buttonGap / 2, buttonY,
        centerX + buttonGap / 2 + buttonWidth, buttonY + buttonHeight);

    DrawButton(pDC, readyStartButton, _T("开始游戏"), true);
    DrawButton(pDC, readyBackButton, _T("返回修改"));
}

// ============================================================
// 鼠标点击
// ============================================================

void CBubbleGameUIView::OnLButtonDown(UINT nFlags, CPoint point)
{
    // 按钮矩形使用 980 x 680 的设计坐标，因此点击位置也要换算
    // 到同一套坐标中，保证放大后的按钮仍能准确点击。
    CRect clientRect;
    GetClientRect(&clientRect);
    point = ClientPointToDesignPoint(clientRect, point);

    // ---- 主菜单 ----
    if (gameState == GameState::MAIN_MENU)
    {
        if (startButton.PtInRect(point))
        {
            selectedMode = 0;
            player1Character = -1;
            player2Character = -1;
            selectingPlayer = 1;
            gameState = GameState::MODE_SELECT;
            Invalidate();
        }
        else if (helpButton.PtInRect(point))
        {
            ShowGameHelpWindow(AfxGetMainWnd());
        }
        else if (exitButton.PtInRect(point))
        {
            const int result = AfxMessageBox(
                _T("确定要退出游戏吗？"),
                MB_YESNO | MB_ICONQUESTION);
            if (result == IDYES)
                AfxGetMainWnd()->SendMessage(WM_CLOSE);
        }
    }
    // ---- 模式选择 ----
    else if (gameState == GameState::MODE_SELECT)
    {
        if (singleButton.PtInRect(point))
        {
            selectedMode = 1;
            Invalidate();
        }
        else if (multiButton.PtInRect(point))
        {
            selectedMode = 2;
            Invalidate();
        }
        else if (modeConfirmButton.PtInRect(point))
        {
            if (selectedMode == 0)
            {
                AfxMessageBox(_T("请先选择单人模式或双人模式。"));
            }
            else
            {
                player1Character = -1;
                player2Character = -1;
                selectingPlayer = 1;
                gameState = GameState::CHARACTER_SELECT;
                Invalidate();
            }
        }
        else if (backButton.PtInRect(point))
        {
            selectedMode = 0;
            gameState = GameState::MAIN_MENU;
            Invalidate();
        }
    }
    // ---- 人物选择 ----
    else if (gameState == GameState::CHARACTER_SELECT)
    {
        int character = -1;

        if (characterButton1.PtInRect(point))
            character = 0;
        else if (characterButton2.PtInRect(point))
            character = 1;
        else if (characterButton3.PtInRect(point))
            character = 2;
        else if (characterButton4.PtInRect(point))
            character = 3;

        if (character != -1)
        {
            if (selectedMode == 1)
                player1Character = character;
            else
            {
                if (selectingPlayer == 1)
                    player1Character = character;
                else
                    player2Character = character;
            }
            Invalidate();
        }
        else if (characterConfirmButton.PtInRect(point))
        {
            // 单人
            if (selectedMode == 1)
            {
                if (player1Character == -1)
                    AfxMessageBox(_T("请先选择一个角色"));
                else
                {
                    gameState = GameState::READY;
                    Invalidate();
                }
            }
            // 双人
            else
            {
                if (selectingPlayer == 1)
                {
                    if (player1Character == -1)
                        AfxMessageBox(_T("玩家1请先选择角色"));
                    else
                    {
                        selectingPlayer = 2;
                        Invalidate();
                    }
                }
                else
                {
                    if (player2Character == -1)
                        AfxMessageBox(_T("玩家2请先选择角色"));
                    else
                    {
                        gameState = GameState::READY;
                        Invalidate();
                    }
                }
            }
        }
        else if (characterBackButton.PtInRect(point))
        {
            if (selectedMode == 2 && selectingPlayer == 2)
            {
                selectingPlayer = 1;
                player2Character = -1;
                Invalidate();
            }
            else
            {
                gameState = GameState::MODE_SELECT;
                player1Character = -1;
                player2Character = -1;
                selectingPlayer = 1;
                Invalidate();
            }
        }
    }
    // ---- READY ----
    else if (gameState == GameState::READY)
    {
        if (readyStartButton.PtInRect(point))
        {
            if (selectedMode == 1)
            {
                // 单人 → 经典模式
                g_gameConfig.mode = 1;
                g_gameConfig.enemyCount = 3;
                g_gameConfig.p1Char = player1Character;
                g_gameConfig.p2Char = -1;
            }
            else
            {
                // 双人 → 泡泡模式
                g_gameConfig.mode = 0;
                g_gameConfig.enemyCount = 0;
                g_gameConfig.p1Char = player1Character;
                g_gameConfig.p2Char = player2Character;
            }

            CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
            if (pFrame != nullptr)
                pFrame->SwitchToView(RUNTIME_CLASS(CBubbleBattleView));
            return;
        }
        else if (readyBackButton.PtInRect(point))
        {
            gameState = GameState::CHARACTER_SELECT;
            if (selectedMode == 2)
                selectingPlayer = 2;
            Invalidate();
        }

        CView::OnLButtonDown(nFlags, point);
    }
}

// ============================================================
// 打印
// ============================================================

BOOL CBubbleGameUIView::OnPreparePrinting(CPrintInfo* pInfo)
{
    return DoPreparePrinting(pInfo);
}

void CBubbleGameUIView::OnBeginPrinting(CDC*, CPrintInfo*)
{
}

void CBubbleGameUIView::OnEndPrinting(CDC*, CPrintInfo*)
{
}

// ============================================================
// Debug
// ============================================================

#ifdef _DEBUG

void CBubbleGameUIView::AssertValid() const
{
    CView::AssertValid();
}

void CBubbleGameUIView::Dump(CDumpContext& dc) const
{
    CView::Dump(dc);
}

CBubbleBattleDoc* CBubbleGameUIView::GetDocument() const
{
    ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CBubbleBattleDoc)));
    return (CBubbleBattleDoc*)m_pDocument;
}

#endif



