
// BubbleBattleView.h: CBubbleBattleView 类的接口
//

#pragma once

#include <vector>

struct BubbleState
{
	bool active;
	int row;
	int col;
	ULONGLONG placedTime;
};

class CBubbleBattleView : public CView
{
protected: // 仅从序列化创建
	CBubbleBattleView() noexcept;
	DECLARE_DYNCREATE(CBubbleBattleView)

// 特性
public:
	CBubbleBattleDoc* GetDocument() const;

// 操作
public:

// 重写
public:
	virtual void OnDraw(CDC* pDC);  // 重写以绘制该视图
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	// 常量：地图行列、单元格大小与偏移
	enum { ROWS = 13, COLS = 15, CELL_SIZE = 40, OFFSET_X = 190, OFFSET_Y = 90 };

	// 地图与玩家状态（玩家1）
	int gameMap[ROWS][COLS];

	// 不再使用格子坐标控制玩家，使用像素坐标（double）
	double playerX; // 左上角像素坐标
	double playerY;

	// 渲染与动画状态
	int playerDir; // 0 down,1 up,2 left,3 right
	int animFrame; // 0 or 1
	int animCounter;

	// 按键状态（连续移动）
	// 按键状态（连续移动）
	bool keyW;
	bool keyA;
	bool keyS;
	bool keyD;

	// 像素坐标与尺寸
	enum { PLAYER_SIZE = 28, SPEED = 4 };

	// 判定函数
	bool CanMoveTo(double newX, double newY) const;
	void DrawPixelPlayer(CDC* pDC, int x, int y, bool moving, int direction, int frame);
	// 泡泡与爆炸状态
	BubbleState bubble;
	bool spacePressed;
	bool explosionActive;
	ULONGLONG explosionStartTime;
	std::vector<CPoint> explosionCells;

	void PlaceBubble();
	void ExplodeBubble();
	void AddExplosionDirection(int startRow, int startCol, int dr, int dc);
	void DrawBubble(CDC* pDC);
	void DrawExplosion(CDC* pDC);
protected:
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	virtual void OnInitialUpdate();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);

// 实现
public:
	virtual ~CBubbleBattleView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 生成的消息映射函数
protected:
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // BubbleBattleView.cpp 中的调试版本
inline CBubbleBattleDoc* CBubbleBattleView::GetDocument() const
   { return reinterpret_cast<CBubbleBattleDoc*>(m_pDocument); }
#endif

