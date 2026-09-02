// BubbleBattleView.cpp: CBubbleBattleView 类的实现
//

#include "pch.h"
#include "framework.h"
#ifndef SHARED_HANDLERS
#include "BubbleBattle.h"
#endif

#include "BubbleBattleDoc.h"
#include "BubbleBattleView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CBubbleBattleView

IMPLEMENT_DYNCREATE(CBubbleBattleView, CView)

BEGIN_MESSAGE_MAP(CBubbleBattleView, CView)
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_TIMER()
	ON_WM_KILLFOCUS()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

// CBubbleBattleView 构造/析构

CBubbleBattleView::CBubbleBattleView() noexcept
{
	// 初始化地图成员（只在构造函数中初始化一次）
	for (int r = 0; r < ROWS; ++r)
		for (int c = 0; c < COLS; ++c)
			gameMap[r][c] = 0;

	// 最外圈不可破坏墙
	for (int c = 0; c < COLS; ++c) {
		gameMap[0][c] = 1;
		gameMap[ROWS-1][c] = 1;
	}
	// 最左/最右列不可破坏墙
	for (int r = 0; r < ROWS; ++r) {
		gameMap[r][0] = 1;
		gameMap[r][COLS-1] = 1;
	}

	// 内部暂时全部可通行（测试用），不放置内部固定墙或木箱

	// 设置玩家初始像素位置（放在第1行第1列格子中央）
	int startRow = 1;
	int startCol = 1;
	playerX = OFFSET_X + startCol * CELL_SIZE + (CELL_SIZE - PLAYER_SIZE) / 2.0;
	playerY = OFFSET_Y + startRow * CELL_SIZE + (CELL_SIZE - PLAYER_SIZE) / 2.0;

	// 初始化按键状态
	keyW = keyA = keyS = keyD = false;

	// 初始化动画状态
	playerDir = 0;
	animFrame = 0;
	animCounter = 0;
}

void CBubbleBattleView::DrawPixelPlayer(CDC* pDC, int x, int y, bool moving, int direction, int frame)
{
	// Simple pixel art ~28x32 using FillSolidRect
	COLORREF outline = RGB(10, 40, 80);
	COLORREF hat = RGB(35, 100, 180);
	COLORREF skin = RGB(255, 220, 190);
	COLORREF eye = RGB(30, 30, 60);
	COLORREF shirt = RGB(50, 130, 200);
	COLORREF glove = RGB(255, 255, 255);
	COLORREF shoe = RGB(10, 30, 60);

	// torso
	pDC->FillSolidRect(x + 6, y + 10, 16, 12, shirt);
	// outline
	pDC->FillSolidRect(x + 5, y + 9, 18, 1, outline);
	pDC->FillSolidRect(x + 5, y + 22, 18, 1, outline);
	pDC->FillSolidRect(x + 5, y + 10, 1, 12, outline);
	pDC->FillSolidRect(x + 22, y + 10, 1, 12, outline);

	// head
	pDC->FillSolidRect(x + 8, y + 2, 12, 10, skin);
	pDC->FillSolidRect(x + 7, y + 1, 14, 1, outline);
	pDC->FillSolidRect(x + 7, y + 11, 14, 1, outline);
	pDC->FillSolidRect(x + 7, y + 2, 1, 10, outline);
	pDC->FillSolidRect(x + 20, y + 2, 1, 10, outline);

	// hat
	pDC->FillSolidRect(x + 6, y - 2, 16, 6, hat);
	pDC->FillSolidRect(x + 6, y - 3, 16, 1, outline);

	// eyes
	pDC->FillSolidRect(x + 10, y + 5, 2, 2, eye);
	pDC->FillSolidRect(x + 16, y + 5, 2, 2, eye);

	// gloves
	pDC->FillSolidRect(x + 4, y + 16, 4, 4, glove);
	pDC->FillSolidRect(x + 20, y + 16, 4, 4, glove);

	// legs (animate)
	if (moving && frame == 1) {
		pDC->FillSolidRect(x + 8, y + 24, 6, 6, shoe);
		pDC->FillSolidRect(x + 16, y + 26, 6, 4, shoe);
	} else {
		pDC->FillSolidRect(x + 8, y + 24, 6, 6, shoe);
		pDC->FillSolidRect(x + 16, y + 24, 6, 6, shoe);
	}
}

BOOL CBubbleBattleView::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;

	for (int r = 0; r < ROWS; ++r) {
		gameMap[r][0] = 1;
		gameMap[r][COLS-1] = 1;
	}

	// 内部暂时全部可通行（测试用），不放置内部固定墙或木箱

	// 设置玩家初始像素位置（放在第1行第1列格子中央）
	int startRow = 1;
	int startCol = 1;
	playerX = OFFSET_X + startCol * CELL_SIZE + (CELL_SIZE - PLAYER_SIZE) / 2.0;
	playerY = OFFSET_Y + startRow * CELL_SIZE + (CELL_SIZE - PLAYER_SIZE) / 2.0;

	// 初始化按键状态
	keyW = keyA = keyS = keyD = false;

	// 初始化动画状态
	playerDir = 0;
	animFrame = 0;
	animCounter = 0;

}

CBubbleBattleView::~CBubbleBattleView()
{
	KillTimer(1);
}

BOOL CBubbleBattleView::PreCreateWindow(CREATESTRUCT& cs)
{
	return CView::PreCreateWindow(cs);
}

void CBubbleBattleView::OnInitialUpdate()
{
	CView::OnInitialUpdate();
	SetFocus();
	SetTimer(1, 16, nullptr);
}

// CBubbleBattleView 绘图

void CBubbleBattleView::OnDraw(CDC* pDC)
{
	CBubbleBattleDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;
	// 双缓冲：在内存DC上绘制然后一次性复制到屏幕
	CRect clientRect;
	GetClientRect(&clientRect);
	int w = clientRect.Width();
	int h = clientRect.Height();

	CDC memDC;
	memDC.CreateCompatibleDC(pDC);
	CBitmap memBmp;
	memBmp.CreateCompatibleBitmap(pDC, w, h);
	CBitmap* pOldBmp = memDC.SelectObject(&memBmp);

	// 背景填充：深蓝色
	COLORREF bg = RGB(18, 35, 58);
	CBrush brBg(bg);
	memDC.FillRect(&clientRect, &brBg);

	// 地图面板与阴影
	int mapW = COLS * CELL_SIZE; // 600
	int mapH = ROWS * CELL_SIZE; // 520
	CRect panelRect(OFFSET_X - 12, OFFSET_Y - 12, OFFSET_X + mapW + 12, OFFSET_Y + mapH + 12);
	// 阴影
	CRect shadowRect = panelRect;
	shadowRect.OffsetRect(6, 6);
	CBrush brShadow(RGB(8, 18, 28));
	memDC.RoundRect(&shadowRect, CPoint(16, 16));
	// 主面板
	CBrush brPanel(RGB(38, 91, 125));
	memDC.RoundRect(&panelRect, CPoint(16, 16));

	// 绘制地图格子（无黑色网格线）
	COLORREF tileA = RGB(111, 211, 221);
	COLORREF tileB = RGB(92, 194, 211);
	COLORREF tileHighlight = RGB(180, 245, 250);
	COLORREF separator = RGB(220, 250, 253);

	for (int r = 0; r < ROWS; ++r) {
		for (int c = 0; c < COLS; ++c) {
			int left = OFFSET_X + c * CELL_SIZE;
			int top = OFFSET_Y + r * CELL_SIZE;
			CRect rc(left, top, left + CELL_SIZE, top + CELL_SIZE);

			if (gameMap[r][c] == 1) {
				// 绘制石墙基色占位（稍后可丰富）
				memDC.FillSolidRect(&rc, RGB(232, 218, 177));
				// 内部留白边距 2px
				CRect inner = rc;
				inner.DeflateRect(2, 2);
				// 顶部高光
				CRect topHigh(inner.left, inner.top, inner.right, inner.top + 6);
				memDC.FillSolidRect(&topHigh, RGB(255, 250, 220));
				// 底部阴影
				CRect botShade(inner.left, inner.bottom - 6, inner.right, inner.bottom);
				memDC.FillSolidRect(&botShade, RGB(160, 120, 90));
				// 简单砖纹
				for (int by = inner.top + 8; by < inner.bottom - 8; by += 12) {
					memDC.FillSolidRect(inner.left + 4, by, inner.Width() - 8, 4, RGB(210, 190, 160));
				}
			} else {
				// 水池瓷砖，棋盘交替色
				bool alt = ((r + c) % 2) == 0;
				memDC.FillSolidRect(&rc, alt ? tileA : tileB);
				// 顶部高光
				CRect topHigh(rc.left + 4, rc.top + 4, rc.right - 4, rc.top + 8);
				memDC.FillSolidRect(&topHigh, tileHighlight);
				// 非黑色分隔线（非常浅）
				CBrush brSep(separator);
				memDC.FillSolidRect(rc.right - 1, rc.top + 1, 1, rc.Height() - 2, separator);
				memDC.FillSolidRect(rc.left + 1, rc.bottom - 1, rc.Width() - 2, 1, separator);
			}
		}
	}

	// 顶部 HUD：左/中/右 信息卡
	CFont fontLarge;
	LOGFONT lf = { 0 };
	lf.lfHeight = -18; // ~18pt
	lf.lfWeight = FW_BOLD;
	wcscpy_s(lf.lfFaceName, L"Segoe UI");
	fontLarge.CreateFontIndirectW(&lf);

	CFont* pOldFont = memDC.SelectObject(&fontLarge);

	// 左卡
	CRect leftCard(panelRect.left + 12, panelRect.top + 8, panelRect.left + 140, panelRect.top + 68);
	memDC.FillSolidRect(&leftCard, RGB(60, 140, 200));
	memDC.SetTextColor(RGB(255, 255, 255));
	memDC.SetBkMode(TRANSPARENT);
	memDC.DrawTextW(L"P1\nWASD", &leftCard, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

	// 中间计时器卡
	CRect midCard(panelRect.left + (panelRect.Width() / 2) - 60, panelRect.top + 8, panelRect.left + (panelRect.Width() / 2) + 60, panelRect.top + 68);
	memDC.FillSolidRect(&midCard, RGB(30, 60, 80));
	memDC.SetTextColor(RGB(255, 255, 255));
	memDC.DrawTextW(L"03:00", &midCard, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

	// 右卡
	CRect rightCard(panelRect.right - 140, panelRect.top + 8, panelRect.right - 12, panelRect.top + 68);
	memDC.FillSolidRect(&rightCard, RGB(200, 60, 60));
	memDC.SetTextColor(RGB(255, 255, 255));
	memDC.DrawTextW(L"P2\nCOMING SOON", &rightCard, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

	memDC.SelectObject(pOldFont);
	fontLarge.DeleteObject();

	// 底部提示
	CFont smallFont;
	LOGFONT lf2 = { 0 };
	lf2.lfHeight = -14;
	lf2.lfWeight = FW_NORMAL;
	wcscpy_s(lf2.lfFaceName, L"Segoe UI");
	smallFont.CreateFontIndirectW(&lf2);
	CFont* pOldSmall = memDC.SelectObject(&smallFont);
	memDC.SetTextColor(RGB(230, 230, 230));
	CRect hintRect(OFFSET_X, OFFSET_Y + mapH + 18, OFFSET_X + mapW, OFFSET_Y + mapH + 48);
	memDC.DrawTextW(L"WASD MOVE   |   SPACE BUBBLE", &hintRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	memDC.SelectObject(pOldSmall);
	smallFont.DeleteObject();

	// 绘制像素人物与影子
	// Shadow
	COLORREF shadowCol = RGB(20, 30, 45);
	CBrush brShadowEl(shadowCol);
	CBrush* pOldBrush = memDC.SelectObject(&brShadowEl);
	int shW = PLAYER_SIZE + 8;
	int shH = 10;
	int shX = (int)(playerX + PLAYER_SIZE / 2) - shW / 2;
	int shY = (int)(playerY + PLAYER_SIZE) - 6;
	memDC.Ellipse(shX, shY, shX + shW, shY + shH);
	memDC.SelectObject(pOldBrush);

	// 人物（使用简单两帧动画）
	bool moving = (keyW || keyA || keyS || keyD);
	// update anim frame
	if (moving) {
		animCounter++;
		if (animCounter >= 6) { animCounter = 0; animFrame = 1 - animFrame; }
	} else {
		animFrame = 0;
		animCounter = 0;
	}

	DrawPixelPlayer(&memDC, (int)playerX, (int)playerY, moving, playerDir, animFrame);

	// 复制到屏幕
	pDC->BitBlt(0, 0, w, h, &memDC, 0, 0, SRCCOPY);

	// 清理
	memDC.SelectObject(pOldBmp);
	memBmp.DeleteObject();
	memDC.DeleteDC();
}

// CBubbleBattleView 诊断

#ifdef _DEBUG
void CBubbleBattleView::AssertValid() const
{
	CView::AssertValid();
}

void CBubbleBattleView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CBubbleBattleDoc* CBubbleBattleView::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CBubbleBattleDoc)));
	return (CBubbleBattleDoc*)m_pDocument;
}
#endif //_DEBUG

// CBubbleBattleView 消息处理程序

void CBubbleBattleView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch (nChar) {
	case 'W': case 'w':
		keyW = true; break;
	case 'S': case 's':
		keyS = true; break;
	case 'A': case 'a':
		keyA = true; break;
	case 'D': case 'd':
		keyD = true; break;
	default:
		return;
	}
}

void CBubbleBattleView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch (nChar) {
	case 'W': case 'w':
		keyW = false; break;
	case 'S': case 's':
		keyS = false; break;
	case 'A': case 'a':
		keyA = false; break;
	case 'D': case 'd':
		keyD = false; break;
	default:
		return;
	}
}

void CBubbleBattleView::OnKillFocus(CWnd* pNewWnd)
{
	keyW = keyA = keyS = keyD = false;
	CView::OnKillFocus(pNewWnd);
}

void CBubbleBattleView::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent != 1) {
		CView::OnTimer(nIDEvent);
		return;
	}

	double dx = 0.0, dy = 0.0;
	if (keyW) dy -= (double)SPEED;
	if (keyS) dy += (double)SPEED;
	if (keyA) dx -= (double)SPEED;
	if (keyD) dx += (double)SPEED;

	if (dx == 0.0 && dy == 0.0) return;

	// 横向和纵向分别检测
	if (dx != 0.0) {
		double candX = playerX + dx;
		if (CanMoveTo(candX, playerY)) {
			playerX = candX;
		}
	}
	if (dy != 0.0) {
		double candY = playerY + dy;
		if (CanMoveTo(playerX, candY)) {
			playerY = candY;
		}
	}

	Invalidate(FALSE);
}

bool CBubbleBattleView::CanMoveTo(double newX, double newY) const
{
	// Check left/top boundary before integer division
	double relLeft = newX - OFFSET_X + 3.0;
	double relTop = newY - OFFSET_Y + 3.0;
	double relRight = newX - OFFSET_X + PLAYER_SIZE - 4.0;
	double relBottom = newY - OFFSET_Y + PLAYER_SIZE - 4.0;

	if (relLeft < 0.0 || relTop < 0.0) return false;

	int left = int(relLeft / CELL_SIZE);
	int right = int(relRight / CELL_SIZE);
	int top = int(relTop / CELL_SIZE);
	int bottom = int(relBottom / CELL_SIZE);

	for (int r = top; r <= bottom; ++r) {
		for (int c = left; c <= right; ++c) {
			if (r < 0 || r >= ROWS || c < 0 || c >= COLS) return false;
			if (gameMap[r][c] != 0) return false;
		}
	}
	return true;
}
