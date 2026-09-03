// BubbleBattleView.cpp: CBubbleBattleView 类的实现
//

#include "pch.h"
#include "framework.h"
#include <cmath>
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
	// 使用固定初始地图，仅在构造函数中初始化一次
	static const int initialMap[ROWS][COLS] =
	{
		{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
		{1,0,0,0,2,0,1,0,0,2,0,0,2,0,1},
		{1,0,2,0,1,0,0,0,2,0,1,0,0,0,1},
		{1,0,0,2,0,0,1,0,0,0,2,0,1,0,1},
		{1,2,1,0,0,2,0,0,1,0,0,2,0,0,1},
		{1,0,0,0,1,0,2,0,0,1,0,0,2,0,1},
		{1,0,2,0,0,0,1,0,2,0,0,1,0,0,1},
		{1,0,1,0,2,0,0,0,1,0,2,0,0,0,1},
		{1,2,0,0,0,1,0,2,0,0,1,0,2,0,1},
		{1,0,0,1,0,0,2,0,0,1,0,0,0,0,1},
		{1,0,2,0,1,0,0,0,1,0,2,0,1,0,1},
		{1,0,0,0,2,0,1,0,0,2,0,0,0,0,1},
		{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
	};

	for (int r = 0; r < ROWS; ++r) {
		for (int c = 0; c < COLS; ++c) {
			gameMap[r][c] = initialMap[r][c];
		}
	}

	// 设置玩家初始像素位置（放在第1行第1列格子中央）
	{
		const int startRow = 1;
		const int startCol = 1;
		playerX = OFFSET_X + startCol * CELL_SIZE + (CELL_SIZE - PLAYER_SIZE) / 2.0;
		playerY = OFFSET_Y + startRow * CELL_SIZE + (CELL_SIZE - PLAYER_SIZE) / 2.0;
	}

	// 初始化按键状态
	keyW = keyA = keyS = keyD = false;

	// 初始化动画状态
	playerDir = 0;
	animFrame = 0;
	animCounter = 0;

	// 初始化水泡与爆炸状态
	bubble.active = false;
	bubble.row = -1;
	bubble.col = -1;
	bubble.placedTime = 0;
	spacePressed = false;
	explosionActive = false;
	explosionStartTime = 0;
	explosionCells.clear();

}

void CBubbleBattleView::PlaceBubble()
{
	// cannot place if already active or explosion ongoing
	if (bubble.active || explosionActive) return;

	int centerX = int(playerX + PLAYER_SIZE / 2);
	int centerY = int(playerY + PLAYER_SIZE / 2);
	int col = (centerX - OFFSET_X) / CELL_SIZE;
	int row = (centerY - OFFSET_Y) / CELL_SIZE;

	if (col < 0 || col >= COLS || row < 0 || row >= ROWS) return;
	if (gameMap[row][col] != 0) return;

	bubble.active = true;
	bubble.row = row;
	bubble.col = col;
	bubble.placedTime = GetTickCount64();
}

void CBubbleBattleView::ExplodeBubble()
{
	if (!bubble.active) return;
	int row = bubble.row;
	int col = bubble.col;
	bubble.active = false;

	explosionActive = true;
	explosionStartTime = GetTickCount64();
	explosionCells.clear();
	// center
	explosionCells.push_back(CPoint(col, row));

	// four directions
	AddExplosionDirection(row, col, -1, 0);
	AddExplosionDirection(row, col, 1, 0);
	AddExplosionDirection(row, col, 0, -1);
	AddExplosionDirection(row, col, 0, 1);
}

void CBubbleBattleView::AddExplosionDirection(int startRow, int startCol, int dr, int dc)
{
	for (int step = 1; step <= 2; ++step) {
		int r = startRow + dr * step;
		int c = startCol + dc * step;
		if (r < 0 || r >= ROWS || c < 0 || c >= COLS) break;
		int val = gameMap[r][c];
		if (val == 1) break; // stone blocks
		if (val == 3) break; // coral blocks
		if (val == 0) {
			explosionCells.push_back(CPoint(c, r));
			continue;
		}
		if (val == 2) {
			// destroy box
			explosionCells.push_back(CPoint(c, r));
			gameMap[r][c] = 0;
			break;
		}
	}
}

void CBubbleBattleView::DrawBubble(CDC* pDC)
{
	if (!bubble.active) return;
	int left = OFFSET_X + bubble.col * CELL_SIZE;
	int top = OFFSET_Y + bubble.row * CELL_SIZE;
	CRect rc(left, top, left + CELL_SIZE, top + CELL_SIZE);

	ULONGLONG now = GetTickCount64();
	double elapsed = double(now - bubble.placedTime);
	double t = min(2200.0, elapsed);
	double scale = 1.0 + 0.05 * sin(t / 100.0);
	int size = int(PLAYER_SIZE * 0.8 * scale);
	int cx = left + CELL_SIZE / 2;
	int cy = top + CELL_SIZE / 2;
	CRect rcb(cx - size/2, cy - size/2, cx + size/2, cy + size/2);

	// outline
	CBrush brOutline(RGB(10, 40, 80));
	CBrush brBlue(RGB(40, 140, 220));
	CBrush brLight(RGB(160, 220, 250));
	CBrush* pOld = pDC->SelectObject(&brOutline);
	pDC->Ellipse(&rcb);
	pDC->SelectObject(&brBlue);
	CRect inner = rcb;
	inner.DeflateRect(3,3);
	pDC->FillSolidRect(inner.left, inner.top, inner.Width(), inner.Height(), RGB(40,140,220));
	pDC->SelectObject(&brLight);
	CRect rHighlight(inner.left+2, inner.top+2, inner.left+inner.Width()/2, inner.top+inner.Height()/2);
	pDC->FillSolidRect(&rHighlight, RGB(180,240,255));
	pDC->SelectObject(pOld);

	// clean up brushes
	brOutline.DeleteObject();
	brBlue.DeleteObject();
	brLight.DeleteObject();
}

void CBubbleBattleView::DrawExplosion(CDC* pDC)
{
	if (!explosionActive) return;
	for (const CPoint& pt : explosionCells) {
		int left = OFFSET_X + pt.x * CELL_SIZE;
		int top = OFFSET_Y + pt.y * CELL_SIZE;
		CRect rc(left, top, left + CELL_SIZE, top + CELL_SIZE);
		// outer blue
		pDC->FillSolidRect(&rc, RGB(30,180,220));
		// inner highlight
		CRect inner = rc;
		inner.DeflateRect(4,4);
		pDC->FillSolidRect(&inner, RGB(200,245,255));
		// center splash for center cell
		// draw small white circle
		int cx = left + CELL_SIZE/2;
		int cy = top + CELL_SIZE/2;
		CBrush brSplash(RGB(240,255,255));
		CBrush* pOld = pDC->SelectObject(&brSplash);
		pDC->Ellipse(cx-6, cy-6, cx+6, cy+6);
		pDC->SelectObject(pOld);
		brSplash.DeleteObject();
	}
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

	// eyes - change position by direction (simple facing)
	int eyeLx = x + 10;
	int eyeRx = x + 16;
	if (direction == 2) { // left
		eyeLx = x + 16; eyeRx = x + 10;
	} else if (direction == 1) { // up
		eyeLx = x + 10; eyeRx = x + 16; // same
	} else if (direction == 3) { // right
		eyeLx = x + 10; eyeRx = x + 16;
	}
	pDC->FillSolidRect(eyeLx, y + 5, 2, 2, eye);
	pDC->FillSolidRect(eyeRx, y + 5, 2, 2, eye);

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
			} else if (gameMap[r][c] == 2) {
				// 绘制木箱（带三种细微变化）
				// 外框
				memDC.FillSolidRect(&rc, RGB(90, 50, 20));
				CRect inner = rc;
				inner.DeflateRect(3, 3);
				// 主体颜色与轻微变化
				int variant = (r + c) % 3;
				if (variant == 0) memDC.FillSolidRect(&inner, RGB(180, 110, 60));
				else if (variant == 1) memDC.FillSolidRect(&inner, RGB(170, 100, 50));
				else memDC.FillSolidRect(&inner, RGB(195, 125, 70));
				// 对角木板与钉子位置变化
				CPen penBoard(PS_SOLID, 3, RGB(140, 80, 40));
				CPen* pOldPen = memDC.SelectObject(&penBoard);
				memDC.MoveTo(inner.left + 2, inner.top + 2);
				memDC.LineTo(inner.right - 2, inner.bottom - 2);
				memDC.MoveTo(inner.right - 2, inner.top + 2);
				memDC.LineTo(inner.left + 2, inner.bottom - 2);
				// 钉子（小圆点）位置依 variant
				CBrush brNail(RGB(100, 70, 50));
				CBrush* pOldBr = memDC.SelectObject(&brNail);
				if (variant == 0) memDC.FillRect(CRect(inner.left + 4, inner.top + 4, inner.left + 6, inner.top + 6), &brNail);
				else if (variant == 1) memDC.FillRect(CRect(inner.right - 6, inner.top + 4, inner.right - 4, inner.top + 6), &brNail);
				else memDC.FillRect(CRect((inner.left + inner.right) / 2 - 1, inner.bottom - 6, (inner.left + inner.right) / 2 + 1, inner.bottom - 4), &brNail);
				memDC.SelectObject(pOldBr);
				memDC.SelectObject(pOldPen);
				penBoard.DeleteObject();
				// 顶部高光及右下阴影
				CRect topHigh(inner.left, inner.top, inner.right, inner.top + 6);
				memDC.FillSolidRect(&topHigh, RGB(255, 220, 180));
				CRect shadow(inner.right - 6, inner.top + 6, inner.right, inner.bottom);
				memDC.FillSolidRect(&shadow, RGB(110, 70, 40));
			} else if (gameMap[r][c] == 3) {
				// 绘制珊瑚绿植障碍（像素风）
				// 整格先填充明显亮绿色底色
				memDC.FillSolidRect(&rc, RGB(50,180,100));
				// 底座深青色石台
				CRect base = rc;
				base.top = rc.bottom - 10;
				memDC.FillSolidRect(&base, RGB(20, 60, 60));
				// 外轮廓深绿色像素块
				memDC.FillSolidRect(rc.left + 6, rc.top + 10, 18, 12, RGB(6,80,40));
				// 叶片亮色（多个小块形成叶片形状）
				memDC.FillSolidRect(rc.left + 8, rc.top + 6, 4, 6, RGB(90,200,120));
				memDC.FillSolidRect(rc.left + 14, rc.top + 6, 4, 6, RGB(90,200,120));
				memDC.FillSolidRect(rc.left + 10, rc.top + 8, 6, 6, RGB(60,180,110));
				// 点缀粉红珊瑚
				memDC.FillSolidRect(rc.left + 12, rc.top + 12, 2, 2, RGB(255,150,180));
				memDC.FillSolidRect(rc.left + 9, rc.top + 14, 2, 2, RGB(255,150,180));
			} else {
				// 水池瓷砖，棋盘交替色，并加入固定公式生成装饰
				bool alt = ((r + c) % 2) == 0;
				memDC.FillSolidRect(&rc, alt ? tileA : tileB);
				// 顶部高光
				CRect topHigh(rc.left + 4, rc.top + 4, rc.right - 4, rc.top + 8);
				memDC.FillSolidRect(&topHigh, tileHighlight);
				// 非黑色分隔线（非常浅）
				memDC.FillSolidRect(rc.right - 1, rc.top + 1, 1, rc.Height() - 2, separator);
				memDC.FillSolidRect(rc.left + 1, rc.bottom - 1, rc.Width() - 2, 1, separator);
				// 固定公式装饰 (r*7 + c*3) % 5
				int deco = (r * 7 + c * 3) % 5;
				if (deco == 0) {
					// 小贝壳
					memDC.FillSolidRect(rc.left + 6, rc.top + 18, 4, 2, RGB(240,220,200));
				} else if (deco == 1) {
					// 小气泡
					memDC.Ellipse(rc.left + 10, rc.top + 10, rc.left + 12, rc.top + 12);
				} else if (deco == 2) {
					// 水波一小条
					memDC.FillSolidRect(rc.left + 5, rc.top + 14, 10, 1, RGB(150,230,240));
				} // deco 3,4 留空
			}
		}
	}

	// 顶部 HUD and bottom hint will be drawn after player to maintain draw order

	// 绘制水泡与爆炸（位于人物之前）
	DrawBubble(&memDC);
	DrawExplosion(&memDC);

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

	// 绘制 HUD：左/中/右 信息卡 和 底部提示（在人物之后）
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
	case VK_SPACE:
		if (!spacePressed) {
			PlaceBubble();
			spacePressed = true;
		}
		break;
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
	case VK_SPACE:
		spacePressed = false; break;
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

	// 更新朝向（最后移动方向）
	if (dx > 0.0) playerDir = 3; // right
	else if (dx < 0.0) playerDir = 2; // left
	else if (dy > 0.0) playerDir = 0; // down
	else if (dy < 0.0) playerDir = 1; // up

	// 泡泡与爆炸计时处理
	ULONGLONG now = GetTickCount64();
	if (bubble.active) {
		if (now - bubble.placedTime >= 2200) {
			ExplodeBubble();
		}
	}
	if (explosionActive) {
		if (now - explosionStartTime >= 450) {
			explosionActive = false;
			explosionCells.clear();
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
