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
}

// ============================================================
// 公共背景
// ============================================================

void CBubbleGameUIView::DrawPageBackground(CDC* pDC)
{
    CRect clientRect;
    GetClientRect(&clientRect);

    // 整体浅背景
    pDC->FillSolidRect(clientRect, RGB(246, 249, 255));
    pDC->SetBkMode(TRANSPARENT);
}

// ============================================================
// 公共标题
// ============================================================

void CBubbleGameUIView::DrawTitle(CDC* pDC, const CString& title, int y)
{
    CRect clientRect;
    GetClientRect(&clientRect);

    CFont font;
    font.CreatePointFont(300, _T("Microsoft YaHei"));

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
    CRect clientRect;
    GetClientRect(&clientRect);
    int W = clientRect.Width();
    int H = clientRect.Height();

    pDC->SetBkMode(TRANSPARENT);

    // ---- 大标题 ----
    CFont titleFont;
    titleFont.CreatePointFont(320, _T("Microsoft YaHei"));

    CFont* oldFont = pDC->SelectObject(&titleFont);
    pDC->SetTextColor(RGB(38, 82, 155));

    CRect titleRect(0, (int)(H * 0.08), W, (int)(H * 0.18));
    pDC->DrawText(title, &titleRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    pDC->SelectObject(oldFont);

    // ---- 副标题 ----
    CFont subFont;
    subFont.CreatePointFont(130, _T("Microsoft YaHei"));

    oldFont = pDC->SelectObject(&subFont);
    pDC->SetTextColor(RGB(110, 125, 150));

    CRect subRect(0, (int)(H * 0.19), W, (int)(H * 0.24));
    pDC->DrawText(subtitle, &subRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    pDC->SelectObject(oldFont);
}

// ============================================================
// 公共副标题
// ============================================================

void CBubbleGameUIView::DrawSubtitle(CDC* pDC, const CString& text, int y)
{
    CRect clientRect;
    GetClientRect(&clientRect);

    CFont font;
    font.CreatePointFont(120, _T("Microsoft YaHei"));

    CFont* oldFont = pDC->SelectObject(&font);
    pDC->SetTextColor(RGB(105, 125, 155));

    CRect rect(0, y, clientRect.Width(), y + 32);
    pDC->DrawText(text, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    pDC->SelectObject(oldFont);
}

// ============================================================
// 公共按钮
// ============================================================

void CBubbleGameUIView::DrawButton(CDC* pDC, const CRect& rect, const CString& text, bool primary)
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

    CBrush brush(fillColor);
    CPen pen(PS_SOLID, 2, borderColor);

    CBrush* oldBrush = pDC->SelectObject(&brush);
    CPen* oldPen = pDC->SelectObject(&pen);

    pDC->RoundRect(rect, CPoint(22, 22));

    CFont font;
    font.CreatePointFont(145, _T("Microsoft YaHei"));
    CFont* oldFont = pDC->SelectObject(&font);

    pDC->SetTextColor(textColor);
    pDC->SetBkMode(TRANSPARENT);

    CRect textRect = rect;
    textRect.DeflateRect(8, 4);
    pDC->DrawText(text, &textRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

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

    CRect clientRect;
    GetClientRect(&clientRect);
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
    DrawButton(pDC, helpButton, _T("游戏说明"));
    DrawButton(pDC, exitButton, _T("退出游戏"));
}

// ============================================================
// 模式选择
// ============================================================

void CBubbleGameUIView::DrawModeSelect(CDC* pDC)
{
    DrawPageBackground(pDC);

    DrawHeader(pDC, _T("选择游戏模式"), _T("选择一种玩法开始游戏"));

    CRect clientRect;
    GetClientRect(&clientRect);
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

    DrawCard(pDC, singleButton);
    DrawCard(pDC, multiButton);

    // ---- 模式标题 ----
    CFont titleFont;
    titleFont.CreatePointFont(190, _T("Microsoft YaHei"));
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
    subFont.CreatePointFont(120, _T("Microsoft YaHei"));
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

    // 返回按钮放在底部
    backButton = CRect(centerX - 110, (int)(H * 0.78), centerX + 110, (int)(H * 0.78) + 58);
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

    DrawHeader(pDC, title, _T("点击角色卡片后确认选择"));

    CRect clientRect;
    GetClientRect(&clientRect);
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
        font.CreatePointFont(135, _T("Microsoft YaHei"));
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

    CRect clientRect;
    GetClientRect(&clientRect);
    int W = clientRect.Width();
    int H = clientRect.Height();
    int centerX = W / 2;

    DrawHeader(pDC, _T("准备开始"), _T("确认配置后开始游戏"));

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
            characterText.Format(_T("%s / %s"), characters[player1Character], characters[player2Character]);
        else
            characterText = _T("未选择");
    }

    // ---- 两张信息卡片 ----
    const int cardWidth = 250;
    const int cardHeight = 180;
    const int gap = 45;

    int totalWidth = cardWidth * 3 + gap * 2;
    int startX = centerX - totalWidth / 2;
    int cardTop = (int)(H * 0.34);

    CRect modeCard(startX, cardTop, startX + cardWidth, cardTop + cardHeight);
    CRect characterCard(startX + cardWidth + gap, cardTop, startX + cardWidth * 2 + gap, cardTop + cardHeight);

    DrawCard(pDC, modeCard, false);
    DrawCard(pDC, characterCard, false);

    // ---- 小标题字体 ----
    CFont labelFont;
    labelFont.CreatePointFont(120, _T("Microsoft YaHei"));
    CFont* oldFont = pDC->SelectObject(&labelFont);
    pDC->SetTextColor(RGB(120, 135, 160));

    CRect modeLabel = modeCard;
    modeLabel.top += 30;
    modeLabel.bottom = modeLabel.top + 35;
    pDC->DrawText(_T("游戏模式"), &modeLabel, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    CRect characterLabel = characterCard;
    characterLabel.top += 30;
    characterLabel.bottom = characterLabel.top + 35;
    pDC->DrawText(_T("角色配置"), &characterLabel, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    pDC->SelectObject(oldFont);

    // ---- 值字体 ----
    CFont valueFont;
    valueFont.CreatePointFont(180, _T("Microsoft YaHei"));
    oldFont = pDC->SelectObject(&valueFont);
    pDC->SetTextColor(RGB(45, 80, 145));

    CRect modeValue = modeCard;
    modeValue.top += 85;
    modeValue.bottom = modeValue.top + 55;
    pDC->DrawText(modeText, &modeValue, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    CRect characterValue = characterCard;
    characterValue.top += 85;
    characterValue.bottom = characterValue.top + 55;

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
            int sz = 60;
            int cx = characterCard.left + characterCard.Width() / 2;
            int cy = characterCard.top + 60;

            CDC memDC;
            memDC.CreateCompatibleDC(pDC);
            CBitmap* pOld = memDC.SelectObject(pBmp1);
            pDC->StretchBlt(cx - sz / 2, cy - sz / 2, sz, sz,
                &memDC, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
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
    // ---- 主菜单 ----
    if (gameState == GameState::MAIN_MENU)
    {
        if (startButton.PtInRect(point))
        {
            gameState = GameState::MODE_SELECT;
            Invalidate();
        }
        else if (helpButton.PtInRect(point))
        {
            AfxMessageBox(
                _T("游戏操作说明\n\n")
                _T("玩家1：W A S D 移动\n")
                _T("空格键：放置炸弹\n\n")
                _T("玩家2：方向键移动\n")
                _T("Enter：放置炸弹"));
        }
        else if (exitButton.PtInRect(point))
        {
            int result = AfxMessageBox(_T("确定要退出游戏吗？"), MB_YESNO | MB_ICONQUESTION);
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
            player1Character = -1;
            player2Character = -1;
            selectingPlayer = 1;
            gameState = GameState::CHARACTER_SELECT;
            Invalidate();
        }
        else if (multiButton.PtInRect(point))
        {
            selectedMode = 2;
            player1Character = -1;
            player2Character = -1;
            selectingPlayer = 1;
            gameState = GameState::CHARACTER_SELECT;
            Invalidate();
        }
        else if (backButton.PtInRect(point))
        {
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