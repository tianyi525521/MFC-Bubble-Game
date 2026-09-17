#pragma once

class CBubbleBattleDoc;


// ============================================================
// CBubbleGameUIView
// ============================================================

class CBubbleGameUIView : public CView
{
protected:
    CBubbleGameUIView() noexcept;
    DECLARE_DYNCREATE(CBubbleGameUIView)

public:
    CBubbleBattleDoc* GetDocument() const;
public:
    virtual void OnDraw(CDC* pDC);
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

protected:
    virtual ~CBubbleGameUIView();

protected:
    virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
    virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
    virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

public:

#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

protected:
    DECLARE_MESSAGE_MAP()


    // ============================================================
    // 页面状态
    // ============================================================

private:

    enum class GameState
    {
        MAIN_MENU,
        MODE_SELECT,
        CHARACTER_SELECT,
        READY
    };
private:
    CBitmap m_bmpCharPlayer;    // 角色1：玩家
    CBitmap m_bmpCharEnemy0;    // 角色2：敌人0
    CBitmap m_bmpCharEnemy1;    // 角色3：敌人1
    CBitmap m_bmpCharEnemy2;    // 角色4：敌人2

    GameState gameState = GameState::MAIN_MENU;


    // ============================================================
    // 游戏配置
    // ============================================================

private:

    // 1 = 单人
    // 2 = 双人
    int selectedMode = 0;

    // 0~3
    int player1Character = -1;
    int player2Character = -1;

    int selectingPlayer = 1;



    // ============================================================
    // 主菜单
    // ============================================================

private:

    CRect startButton;
    CRect helpButton;
    CRect exitButton;


    // ============================================================
    // 模式选择
    // ============================================================

private:

    CRect singleButton;
    CRect multiButton;
    CRect modeConfirmButton;
    CRect backButton;


    // ============================================================
    // 人物选择
    // ============================================================

private:

    CRect characterButton1;
    CRect characterButton2;
    CRect characterButton3;
    CRect characterButton4;

    CRect characterConfirmButton;
    CRect characterBackButton;


    // ============================================================
    // 地图选择
    // ============================================================

private:


    // ============================================================
    // READY
    // ============================================================

private:

    CRect readyStartButton;
    CRect readyBackButton;

    // ============================================================
    // 页面绘制
    // ============================================================

private:

    void DrawMainMenu(CDC* pDC);

    void DrawModeSelect(CDC* pDC);

    void DrawCharacterSelect(CDC* pDC);

    void DrawReadyPage(CDC* pDC);


    // ============================================================
    // 公共UI绘制
    // ============================================================

private:

    void DrawPageBackground(CDC* pDC);
    void DrawHeader(CDC* pDC, const CString& title,const CString& subtitle );

    void DrawButton( CDC* pDC,const CRect& rect,const CString& text,bool primary = false, bool selected = false);

    void DrawCard(CDC* pDC,const CRect& rect,bool selected = false );

    // ============================================================
    // 鼠标
    // ============================================================

public:

    afx_msg void OnLButtonDown(
        UINT nFlags,
        CPoint point
    );

    // 一局结束后回到角色选择页，方便重新选择角色（由主框架在切回本视图时调用）
    void ResetToCharacterSelect();
};


#ifndef _DEBUG

inline CBubbleBattleDoc* CBubbleGameUIView::GetDocument() const
{
    return reinterpret_cast<CBubbleBattleDoc*>(m_pDocument);
}

#endif

