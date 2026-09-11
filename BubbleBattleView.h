
// BubbleBattleView.h : CBubbleBattleView 类的接口
// 项目：BubbleBattle
// 合并泡泡模式（项目1）与经典模式（项目2），统一使用位图资源

#pragma once

#include <afxwin.h>
#include <vector>

// ---------------------- 数据结构 ----------------------
enum class TileType {
    EMPTY,
    BRICK,
    STEEL
};

struct Bomb {
    int x, y;                   // 格子坐标
    int timeLeft;               // 剩余帧数
    bool exploded;
    bool isPlayerBomb;
    int ownerEnemyIndex;        // -1表示玩家，>=0表示敌人索引
};

struct Explosion {
    int x, y;
    int timeLeft;
    bool fromPlayer;
};

struct Enemy {
    int x, y;
    int lives;
    int moveTimer;
    int bombTimer;
    bool alive;
    int type;                   // 0,1,2
    int dir;                    // 0=上,1=下,2=左,3=右
};

// ---------------------- 视图类 ---------------------------
class CBubbleBattleView : public CView
{
protected:
    CBubbleBattleView() noexcept;
    DECLARE_DYNCREATE(CBubbleBattleView)

public:
    CBubbleBattleDoc* GetDocument() const;

    // 游戏模式
    enum GameMode {
        MODE_BUBBLE = 0,        // 泡泡模式（无敌人）
        MODE_CLASSIC = 1        // 经典模式（1个或3个敌人）
    };

    void SetGameMode(GameMode mode) { m_gameMode = mode; }

    virtual void OnDraw(CDC* pDC);
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual void OnInitialUpdate();

protected:
    virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
    virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
    virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

public:
    virtual ~CBubbleBattleView();

#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

protected:
    // ---------- 辅助 ----------
    bool IsMapConnected() const;      
    // ---------- 常量 ----------
    static const int TILE_SIZE = 48;
    static const int MAP_COLS = 12;
    static const int MAP_ROWS = 12;
    static const int GAME_WIDTH = TILE_SIZE * MAP_COLS;    // 576
    static const int GAME_HEIGHT = TILE_SIZE * MAP_ROWS;   // 576
    static const int PLAYER_SIZE = 40;
    static const double PIXEL_SPEED;                        // 像素/帧
    static const UINT_PTR TIMER_ID = 1;

    // ---------- 模式 ----------
    GameMode m_gameMode;
    int m_enemyCount;              // 0=泡泡模式，1或3=经典模式

    // ---------- 地图 ----------
    TileType m_map[MAP_ROWS][MAP_COLS];

    

    // ---------- 游戏结束消息 ----------
    CString m_endDialogMsg;

    // 玩家方向
    enum PlayerDir { DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT };
  // ---------- 玩家1（方向键 + Enter） ----------
    double m_playerPixelX;
    double m_playerPixelY;
    int m_playerLives;
    PlayerDir m_playerDir;

    // ---------- 玩家2（WASD + 空格）— 仅双人模式有效 ----------
    double m_player2PixelX;
    double m_player2PixelY;
    int m_player2Lives;
    PlayerDir m_player2Dir;

    // 敌人方向（与位图加载顺序一致）
    enum EnemyDir {
        ENEMY_LEFT = 0,
        ENEMY_RIGHT = 1,
        ENEMY_UP = 2,
        ENEMY_DOWN = 3
    };

    // 连续按键
    bool m_keyW, m_keyA, m_keyS, m_keyD;

    // ---------- 游戏对象 ----------
    std::vector<Bomb> m_bombs;
    std::vector<Explosion> m_explosions;
    std::vector<Enemy> m_enemies;
    bool m_gameOver;

    
    CBitmap m_bmpPlayerUp;
    CBitmap m_bmpPlayerDown;
    CBitmap m_bmpPlayerLeft;
    CBitmap m_bmpPlayerRight;

    CBitmap m_bmpPlayer2Up;
    CBitmap m_bmpPlayer2Down;
    CBitmap m_bmpPlayer2Left;
    CBitmap m_bmpPlayer2Right;

    // 地图
    CBitmap m_bmpTileBrick;
    CBitmap m_bmpTileSteel;
    CBitmap m_bmpBackground;

    // 敌人 3 类型 × 4 方向
    CBitmap m_enemyBitmaps[4][4];

    // ---------- 初始化 ----------
    void LoadBitmaps();
    void ResetGame();
    void InitMap();
    void InitEnemies();
    void ClearArea(int cx, int cy);

    // ---------- 更新 ----------
    void TryMovePlayer(int playerIndex, int dx, int dy);
    void UpdateEnemies();
    void UpdateBombsAndExplosions();

    // ---------- 操作 ----------
    void PlaceBomb(int x, int y, bool isPlayerBomb, int ownerEnemyIndex = -1);
    void ExplodeBomb(int index);
    void CheckPlayerHit(int playerIndex, int x, int y);
    void CheckEnemyHit(int x, int y, bool fromPlayer, int ownerEnemyIndex = -1);

    // ---------- 碰撞检测 ----------
    bool IsValidPosition(int x, int y) const;
    bool IsTileEmpty(int x, int y) const;
    bool IsPositionFreeForEnemy(int x, int y) const;
    bool IsDangerousPosition(int x, int y) const;
    bool IsOwnBombDangerous(int x, int y, int ownerEnemyIndex) const;
    bool CanMoveTo(double newPixelX, double newPixelY) const;

    // ---------- 绘制 ----------
    void DrawTile(CDC& dc, int x, int y, TileType type);
    void DrawPlayer(CDC& dc, int playerIndex);
    void DrawEnemy(CDC& dc, const Enemy& enemy);
    void DrawBomb(CDC& dc, const Bomb& bomb);
    void DrawExplosion(CDC& dc, const Explosion& exp);

    // ---------- 消息 ----------
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
    DECLARE_MESSAGE_MAP()
    bool m_endDialogShown;   // 防止游戏结束弹窗重入
};

#ifndef _DEBUG
inline CBubbleBattleDoc* CBubbleBattleView::GetDocument() const
{
    return reinterpret_cast<CBubbleBattleDoc*>(m_pDocument);
}
#endif