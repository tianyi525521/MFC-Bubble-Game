
#include "pch.h"
#include "framework.h"
#include "GameConfig.h"
#include "MainFrm.h"
#include "BubbleGameUIView.h"
#ifndef SHARED_HANDLERS
#include "BubbleBattle.h"
#endif
#include "BubbleBattleDoc.h"
#include "BubbleBattleView.h"
#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <queue>           
#include <utility>   

#pragma comment(lib, "msimg32.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const int CBubbleBattleView::GAME_WIDTH;
const int CBubbleBattleView::GAME_HEIGHT;
const int CBubbleBattleView::TILE_SIZE;
const int CBubbleBattleView::MAP_COLS;
const int CBubbleBattleView::MAP_ROWS;
const UINT_PTR CBubbleBattleView::TIMER_ID;
const double CBubbleBattleView::PIXEL_SPEED = 4.0;

IMPLEMENT_DYNCREATE(CBubbleBattleView, CView)

BEGIN_MESSAGE_MAP(CBubbleBattleView, CView)
    ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
    ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
    ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
    ON_WM_TIMER()
    ON_WM_KEYDOWN()
    ON_WM_KEYUP()
END_MESSAGE_MAP()

// ---------------------- 构造 / 析构 ----------------------
CBubbleBattleView::CBubbleBattleView() noexcept
{
    srand(static_cast<unsigned>(GetTickCount()));

    m_gameMode = MODE_BUBBLE;
    m_enemyCount = 0;
    m_playerDir = DIR_DOWN;
    m_player2Dir = DIR_DOWN;
    m_gameOver = false;
    m_endDialogShown = false;
    m_playerPixelX = 1.0 * TILE_SIZE;
    m_playerPixelY = 1.0 * TILE_SIZE;
    m_player2PixelX = (MAP_COLS - 2) * TILE_SIZE;
    m_player2PixelY = 1.0 * TILE_SIZE;
    m_playerLives = 3;
    m_player2Lives = 3;
    m_keyW = m_keyA = m_keyS = m_keyD = false;

   
}

CBubbleBattleView::~CBubbleBattleView()
{
}

// ---------------------- 位图加载（来自BubbleBobbleFinal） ----------------------
void CBubbleBattleView::LoadBitmaps()
{
    struct CharRes {
        UINT up, down, left, right;
    };

    // 每个角色 4 方向
    CharRes charRes[4] = {
        { IDB_PLAYER_UP, IDB_PLAYER_DOWN, IDB_PLAYER_LEFT, IDB_PLAYER_RIGHT },
        { IDB_ENEMY0_UP, IDB_ENEMY0_DOWN, IDB_ENEMY0_LEFT, IDB_ENEMY0_RIGHT },
        { IDB_ENEMY1_UP, IDB_ENEMY1_DOWN, IDB_ENEMY1_LEFT, IDB_ENEMY1_RIGHT },
        { IDB_ENEMY2_UP, IDB_ENEMY2_DOWN, IDB_ENEMY2_LEFT, IDB_ENEMY2_RIGHT }
    };

    // 辅助宏：加载后立即检查
#define SAFE_LOAD(bmp, id)                                          \
        do {                                                            \
            if (!(bmp).LoadBitmap(id)) {                                \
                CString s;                                              \
                s.Format(_T("位图加载失败！ID = %d (0x%X)"), id, id);   \
                AfxMessageBox(s);                                       \
                TRACE(_T("[LoadBitmaps] 失败: ID=%d (0x%X)\n"), id, id); \
            }                                                           \
        } while(0)

    int c1 = (g_gameConfig.p1Char >= 0 && g_gameConfig.p1Char < 4)
        ? g_gameConfig.p1Char : 0;
    int c2 = (g_gameConfig.p2Char >= 0 && g_gameConfig.p2Char < 4)
        ? g_gameConfig.p2Char : 1;

    // 玩家1 位图
    SAFE_LOAD(m_bmpPlayerUp, charRes[c1].up);
    SAFE_LOAD(m_bmpPlayerDown, charRes[c1].down);
    SAFE_LOAD(m_bmpPlayerLeft, charRes[c1].left);
    SAFE_LOAD(m_bmpPlayerRight, charRes[c1].right);

    // 玩家2 位图
    SAFE_LOAD(m_bmpPlayer2Up, charRes[c2].up);
    SAFE_LOAD(m_bmpPlayer2Down, charRes[c2].down);
    SAFE_LOAD(m_bmpPlayer2Left, charRes[c2].left);
    SAFE_LOAD(m_bmpPlayer2Right, charRes[c2].right);

    // 地图砖块
    SAFE_LOAD(m_bmpTileBrick, IDB_TILE_BRICK);
    SAFE_LOAD(m_bmpTileSteel, IDB_TILE_STEEL);

    // 地图背景
    SAFE_LOAD(m_bmpBackground, IDB_MAP_BACKGROUND);

    // 4 种角色（敌人用，索引顺序 0=LEFT, 1=RIGHT, 2=UP, 3=DOWN）
    SAFE_LOAD(m_enemyBitmaps[0][0], IDB_PLAYER_LEFT);
    SAFE_LOAD(m_enemyBitmaps[0][1], IDB_PLAYER_RIGHT);
    SAFE_LOAD(m_enemyBitmaps[0][2], IDB_PLAYER_UP);
    SAFE_LOAD(m_enemyBitmaps[0][3], IDB_PLAYER_DOWN);

    SAFE_LOAD(m_enemyBitmaps[1][0], IDB_ENEMY0_LEFT);
    SAFE_LOAD(m_enemyBitmaps[1][1], IDB_ENEMY0_RIGHT);
    SAFE_LOAD(m_enemyBitmaps[1][2], IDB_ENEMY0_UP);
    SAFE_LOAD(m_enemyBitmaps[1][3], IDB_ENEMY0_DOWN);

    SAFE_LOAD(m_enemyBitmaps[2][0], IDB_ENEMY1_LEFT);
    SAFE_LOAD(m_enemyBitmaps[2][1], IDB_ENEMY1_RIGHT);
    SAFE_LOAD(m_enemyBitmaps[2][2], IDB_ENEMY1_UP);
    SAFE_LOAD(m_enemyBitmaps[2][3], IDB_ENEMY1_DOWN);

    SAFE_LOAD(m_enemyBitmaps[3][0], IDB_ENEMY2_LEFT);
    SAFE_LOAD(m_enemyBitmaps[3][1], IDB_ENEMY2_RIGHT);
    SAFE_LOAD(m_enemyBitmaps[3][2], IDB_ENEMY2_UP);
    SAFE_LOAD(m_enemyBitmaps[3][3], IDB_ENEMY2_DOWN);

#undef SAFE_LOAD
}
// ---------------------- 初始化：模式选择 ----------------------
void CBubbleBattleView::OnInitialUpdate()
{
    CView::OnInitialUpdate();

    // 从全局配置读取模式（由 UI 设置）
    m_gameMode = (g_gameConfig.mode == 0) ? MODE_BUBBLE : MODE_CLASSIC;
    m_enemyCount = g_gameConfig.enemyCount;

    LoadBitmaps();
    ResetGame();
    SetTimer(TIMER_ID, 30, nullptr);
    SetFocus();
}

// ---------------------- 重置 ----------------------
void CBubbleBattleView::ResetGame()
{
    m_playerPixelX = 1.0 * TILE_SIZE;
    m_playerPixelY = 1.0 * TILE_SIZE;
    m_player2PixelX = (MAP_COLS - 2) * TILE_SIZE;
    m_player2PixelY = 1.0 * TILE_SIZE;
    m_playerLives = 3;
    m_player2Lives = 3;
    m_playerDir = DIR_DOWN;
    m_player2Dir = DIR_DOWN;
    m_keyW = m_keyA = m_keyS = m_keyD = false;
    m_gameOver = false;

    m_bombs.clear();
    m_explosions.clear();
    m_enemies.clear();

    InitMap();
    InitEnemies();
}

// ---------------------- 清空区域 ----------------------
void CBubbleBattleView::ClearArea(int cx, int cy)
{
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            int nx = cx + dx;
            int ny = cy + dy;
            if (nx >= 0 && nx < MAP_COLS && ny >= 0 && ny < MAP_ROWS) {
                if (m_map[ny][nx] == TileType::BRICK) {
                    m_map[ny][nx] = TileType::EMPTY;
                }
            }
        }
    }
}

bool CBubbleBattleView::IsMapConnected() const
{
    bool visited[MAP_ROWS][MAP_COLS] = { false };
    std::queue<std::pair<int, int>> q;

    q.push(std::make_pair(1, 1));
    visited[1][1] = true;

    const int dx[] = { 0, 0, -1, 1 };
    const int dy[] = { -1, 1, 0, 0 };

    while (!q.empty()) {
        int x = q.front().first;
        int y = q.front().second;
        q.pop();

        for (int i = 0; i < 4; ++i) {
            int nx = x + dx[i];
            int ny = y + dy[i];

            if (nx < 0 || nx >= MAP_COLS || ny < 0 || ny >= MAP_ROWS) continue;
            if (visited[ny][nx]) continue;
            if (m_map[ny][nx] == TileType::STEEL) continue;   // STEEL 是永久障碍

            visited[ny][nx] = true;
            q.push(std::make_pair(nx, ny));
        }
    }

    // 检查另外三个出生点
    if (!visited[1][MAP_COLS - 2]) return false;                 // 玩家2 (右上)
    if (!visited[MAP_ROWS - 2][1]) return false;                 // 敌人 (左下)
    if (!visited[MAP_ROWS - 2][MAP_COLS - 2]) return false;      // 敌人 (右下)

    return true;
}
// ---------------------- 初始化地图 ----------------------
void CBubbleBattleView::InitMap()
{
    // 最多尝试 30 次，直到生成一个连通的布局
    const int MAX_ATTEMPTS = 30;
    const int STEEL_CHANCE = 22;     // STEEL 概率（%）：建议 15~30，越大越密

    for (int attempt = 0; attempt < MAX_ATTEMPTS; ++attempt)
    {
        // 1. 初始化：边界 STEEL，内部 EMPTY
        for (int y = 0; y < MAP_ROWS; ++y) {
            for (int x = 0; x < MAP_COLS; ++x) {
                if (x == 0 || x == MAP_COLS - 1 ||
                    y == 0 || y == MAP_ROWS - 1) {
                    m_map[y][x] = TileType::STEEL;
                }
                else {
                    m_map[y][x] = TileType::EMPTY;
                }
            }
        }

        // 2. 随机放置 STEEL（跳过 4 个出生点）
        for (int y = 1; y < MAP_ROWS - 1; ++y) {
            for (int x = 1; x < MAP_COLS - 1; ++x) {

                // 出生点必须是空的
                if ((x == 1 && y == 1) ||
                    (x == MAP_COLS - 2 && y == 1) ||
                    (x == 1 && y == MAP_ROWS - 2) ||
                    (x == MAP_COLS - 2 && y == MAP_ROWS - 2)) {
                    continue;
                }

                if ((rand() % 100) < STEEL_CHANCE) {
                    m_map[y][x] = TileType::STEEL;
                }
            }
        }

        // 3. 检查连通性
        if (IsMapConnected()) {
            // 连通 OK → 在剩余空地上随机放可炸砖块
            int brickChance = (m_gameMode == MODE_BUBBLE) ? 25 : 40;
            for (int y = 1; y < MAP_ROWS - 1; ++y) {
                for (int x = 1; x < MAP_COLS - 1; ++x) {
                    if (m_map[y][x] != TileType::EMPTY) continue;

                    // 出生点周围保持空地
                    if ((x == 1 && y == 1) ||
                        (x == MAP_COLS - 2 && y == 1) ||
                        (x == 1 && y == MAP_ROWS - 2) ||
                        (x == MAP_COLS - 2 && y == MAP_ROWS - 2)) {
                        continue;
                    }

                    if ((rand() % 100) < brickChance) {
                        m_map[y][x] = TileType::BRICK;
                    }
                }
            }

            // 清空出生点周围，保证有路可走
            ClearArea(1, 1);
            ClearArea(MAP_COLS - 2, 1);
            ClearArea(1, MAP_ROWS - 2);
            ClearArea(MAP_COLS - 2, MAP_ROWS - 2);

            return;
        }
        // 不连通 → 重试
    }

    // 30 次都没连通 → 兜底：只保留边界 STEEL，内部全空
    for (int y = 0; y < MAP_ROWS; ++y) {
        for (int x = 0; x < MAP_COLS; ++x) {
            if (x == 0 || x == MAP_COLS - 1 ||
                y == 0 || y == MAP_ROWS - 1) {
                m_map[y][x] = TileType::STEEL;
            }
            else {
                m_map[y][x] = TileType::EMPTY;
            }
        }
    }
}
// ---------------------- 初始化敌人 ----------------------
void CBubbleBattleView::InitEnemies()
{
    m_enemies.clear();

    // 双人模式：无敌人
    if (m_gameMode == MODE_BUBBLE) {
        return;
    }

    // 单人模式：固定 3 个敌人
    int playerChar = (g_gameConfig.p1Char >= 0 && g_gameConfig.p1Char < 4)
        ? g_gameConfig.p1Char : 0;

    // 从 4 种里排除玩家选的
    std::vector<int> availTypes;
    for (int t = 0; t < 4; ++t) {
        if (t != playerChar) {
            availTypes.push_back(t);
        }
    }
    if (availTypes.empty()) {
        availTypes.push_back(0);
    }

    // ★ 调试输出：看看到底分了哪些角色
    TRACE(_T("InitEnemies: playerChar=%d, availTypes=["), playerChar);
    for (int t : availTypes) TRACE(_T("%d "), t);
    TRACE(_T("]\n"));

    int startPositions[3][2] = {
        { MAP_COLS - 2, 1 },
        { 1, MAP_ROWS - 2 },
        { MAP_COLS - 2, MAP_ROWS - 2 }
    };

    for (int i = 0; i < 3; ++i) {
        Enemy enemy;
        enemy.x = startPositions[i][0];
        enemy.y = startPositions[i][1];
        enemy.lives = 3;
        enemy.moveTimer = rand() % 500 + 300;
        enemy.bombTimer = rand() % 2000 + 1000;
        enemy.alive = true;
        enemy.type = availTypes[i % (int)availTypes.size()];   // ★ 明确取模
        enemy.dir = ENEMY_DOWN;
        m_map[enemy.y][enemy.x] = TileType::EMPTY;
        m_enemies.push_back(enemy);

        // ★ 调试输出：每个敌人分到的角色
        TRACE(_T("  Enemy[%d]: type=%d at (%d,%d)\n"),
            i, enemy.type, enemy.x, enemy.y);
    }
}
// ---------------------- 位置合法性 ----------------------
bool CBubbleBattleView::IsValidPosition(int x, int y) const
{
    if (x < 0 || x >= MAP_COLS || y < 0 || y >= MAP_ROWS)
        return false;
    if (m_map[y][x] != TileType::EMPTY)
        return false;
    for (const auto& bomb : m_bombs) {
        if (bomb.x == x && bomb.y == y)
            return false;
    }
    int pGridX = static_cast<int>(m_playerPixelX / TILE_SIZE);
    int pGridY = static_cast<int>(m_playerPixelY / TILE_SIZE);
    if (x == pGridX && y == pGridY)
        return false;
    for (const auto& enemy : m_enemies) {
        if (enemy.alive && enemy.x == x && enemy.y == y)
            return false;
    }
    return true;
}

bool CBubbleBattleView::IsTileEmpty(int x, int y) const
{
    if (x < 0 || x >= MAP_COLS || y < 0 || y >= MAP_ROWS)
        return false;
    return m_map[y][x] == TileType::EMPTY;
}

bool CBubbleBattleView::IsPositionFreeForEnemy(int x, int y) const
{
    if (!IsTileEmpty(x, y)) return false;

    for (const auto& bomb : m_bombs) {
        if (bomb.x == x && bomb.y == y) return false;
    }

    int pGridX = static_cast<int>(m_playerPixelX / TILE_SIZE);
    int pGridY = static_cast<int>(m_playerPixelY / TILE_SIZE);
    if (x == pGridX && y == pGridY) return false;

    for (const auto& e : m_enemies) {
        if (e.alive && e.x == x && e.y == y) return false;
    }
    return true;
}

bool CBubbleBattleView::IsDangerousPosition(int x, int y) const
{
    for (const auto& bomb : m_bombs) {
        if (bomb.x == x && bomb.y == y) return true;
        if (abs(bomb.x - x) + abs(bomb.y - y) <= 1) return true;
    }
    return false;
}

bool CBubbleBattleView::IsOwnBombDangerous(int x, int y, int ownerEnemyIndex) const
{
    for (const auto& bomb : m_bombs) {
        if (bomb.ownerEnemyIndex != ownerEnemyIndex) continue;
        if (bomb.x == x && bomb.y == y) return true;
        if (abs(bomb.x - x) + abs(bomb.y - y) <= 1) return true;
    }
    return false;
}

// ---------------------- 放置炸弹 ----------------------
void CBubbleBattleView::PlaceBomb(int x, int y, bool isPlayerBomb, int ownerEnemyIndex)
{
    if (x < 0 || x >= MAP_COLS || y < 0 || y >= MAP_ROWS) return;
    if (m_map[y][x] != TileType::EMPTY) return;

    for (const auto& bomb : m_bombs) {
        if (bomb.x == x && bomb.y == y) return;
    }

    Bomb bomb;
    bomb.x = x;
    bomb.y = y;
    // 泡泡模式 3 秒（90帧），经典模式 2 秒（60帧）
    bomb.timeLeft = (m_gameMode == MODE_BUBBLE) ? 90 : 60;
    bomb.exploded = false;
    bomb.isPlayerBomb = isPlayerBomb;
    bomb.ownerEnemyIndex = ownerEnemyIndex;
    m_bombs.push_back(bomb);
}

// ---------------------- 玩家受伤 ----------------------
void CBubbleBattleView::CheckPlayerHit(int playerIndex, int x, int y)
{
    if (m_gameOver) return;

    // 经典模式没有玩家2
    if (m_gameMode != MODE_BUBBLE && playerIndex == 1) return;

    double px = (playerIndex == 0) ? m_playerPixelX : m_player2PixelX;
    double py = (playerIndex == 0) ? m_playerPixelY : m_player2PixelY;

    int pGridX = static_cast<int>(px / TILE_SIZE);
    int pGridY = static_cast<int>(py / TILE_SIZE);

    if (x == pGridX && y == pGridY) {
        if (playerIndex == 0) {
            m_playerLives--;
            if (m_playerLives <= 0) m_playerLives = 0;
        }
        else {
            m_player2Lives--;
            if (m_player2Lives <= 0) m_player2Lives = 0;
        }

        // ============================================
        // 判定游戏结束
        // ============================================
        if (m_gameMode == MODE_BUBBLE) {
            // 双人模式：一方死亡，另一方获胜
            if (m_playerLives <= 0) {
                m_gameOver = true;
                m_endDialogMsg = _T("玩家2 获胜！玩家1 被炸死了。");
            }
            else if (m_player2Lives <= 0) {
                m_gameOver = true;
                m_endDialogMsg = _T("玩家1 获胜！玩家2 被炸死了。");
            }
        }
        else {
            // 经典模式：玩家死亡即失败
            if (m_playerLives <= 0) {
                m_gameOver = true;
                m_endDialogMsg = _T("游戏失败！你被炸死了。");
            }
        }
    }
}
// ---------------------- 敌人受伤 ----------------------
void CBubbleBattleView::CheckEnemyHit(int x, int y, bool fromPlayer, int ownerEnemyIndex)
{
    if (m_gameOver) return;

    // 没有敌人（双人模式）直接返回
    if (m_enemies.empty()) return;

    for (size_t i = 0; i < m_enemies.size(); ++i) {
        Enemy& enemy = m_enemies[i];
        if (!enemy.alive) continue;

        if (enemy.x == x && enemy.y == y) {
            enemy.lives--;
            if (enemy.lives <= 0) {
                enemy.alive = false;
                enemy.lives = 0;

                bool allDead = true;
                for (const auto& e : m_enemies) {
                    if (e.alive) { allDead = false; break; }
                }

                if (allDead) {
                    m_gameOver = true;
                    m_endDialogMsg = _T("游戏胜利！你消灭了所有敌人。");
                }
            }
        }
    }
}

// ---------------------- 爆炸 ----------------------
void CBubbleBattleView::ExplodeBomb(int index)
{
    if (index < 0 || index >= static_cast<int>(m_bombs.size())) return;

    Bomb& bomb = m_bombs[index];
    bomb.exploded = true;

    // 中心爆炸
    Explosion exp;
    exp.x = bomb.x;
    exp.y = bomb.y;
    exp.timeLeft = 15;
    exp.fromPlayer = bomb.isPlayerBomb;
    m_explosions.push_back(exp);

    CheckPlayerHit(0, bomb.x, bomb.y);
    CheckPlayerHit(1, bomb.x, bomb.y);      
    if (m_gameOver) return;
    CheckEnemyHit(bomb.x, bomb.y, bomb.isPlayerBomb, bomb.ownerEnemyIndex);
    if (m_gameOver) return;

    if (m_map[bomb.y][bomb.x] == TileType::BRICK) {
        m_map[bomb.y][bomb.x] = TileType::EMPTY;
    }

    // 十字火焰（长度 1）
    const int dx[] = { 0, 0, -1, 1 };
    const int dy[] = { -1, 1, 0, 0 };

    for (int dir = 0; dir < 4; ++dir) {
        int nx = bomb.x + dx[dir];
        int ny = bomb.y + dy[dir];

        if (nx < 0 || nx >= MAP_COLS || ny < 0 || ny >= MAP_ROWS) continue;
        if (m_map[ny][nx] == TileType::STEEL) continue;

        if (m_map[ny][nx] == TileType::BRICK) {
            m_map[ny][nx] = TileType::EMPTY;
        }

        Explosion flameExp;
        flameExp.x = nx;
        flameExp.y = ny;
        flameExp.timeLeft = 15;
        flameExp.fromPlayer = bomb.isPlayerBomb;
        m_explosions.push_back(flameExp);

        CheckPlayerHit(0, nx, ny);
        CheckPlayerHit(1, nx, ny);              // 新增
        if (m_gameOver) return;
        CheckEnemyHit(nx, ny, bomb.isPlayerBomb, bomb.ownerEnemyIndex);
        if (m_gameOver) return;
    }

    m_bombs.erase(m_bombs.begin() + index);
}

// ---------------------- 敌人AI ----------------------
void CBubbleBattleView::UpdateEnemies()
{
    for (size_t idx = 0; idx < m_enemies.size(); ++idx) {
        Enemy& enemy = m_enemies[idx];
        if (!enemy.alive) continue;

        int enemyIndex = static_cast<int>(idx);

        // ============================================================
        // 1. 优先逃离自己的炸弹（允许进入危险格，只要更接近安全）
        // ============================================================
        if (IsOwnBombDangerous(enemy.x, enemy.y, enemyIndex)) {
            int dirs[4][2] = { {-1,0}, {1,0}, {0,-1}, {0,1} };
            int enemyDirs[4] = { ENEMY_LEFT, ENEMY_RIGHT, ENEMY_UP, ENEMY_DOWN };

            int bestDir = -1;
            int bestDist = -1;   // 越大越好（离炸弹越远）

            for (int i = 0; i < 4; ++i) {
                int nx = enemy.x + dirs[i][0];
                int ny = enemy.y + dirs[i][1];

                // 边界和地图判断
                if (!IsValidPosition(nx, ny)) continue;
                if (!IsTileEmpty(nx, ny)) continue;

                // 目标格不能有炸弹（防止踩到炸弹）
                bool hasBomb = false;
                for (const auto& b : m_bombs) {
                    if (b.x == nx && b.y == ny) { hasBomb = true; break; }
                }
                if (hasBomb) continue;

                // 目标格不能有其他活着的敌人
                bool occupied = false;
                for (size_t j = 0; j < m_enemies.size(); ++j) {
                    if (static_cast<int>(j) == enemyIndex) continue;
                    if (m_enemies[j].alive &&
                        m_enemies[j].x == nx && m_enemies[j].y == ny) {
                        occupied = true; break;
                    }
                }
                if (occupied) continue;

                // 计算该格到自己所有炸弹的最小曼哈顿距离（越大越好）
                int minDist = 999;
                for (const auto& bomb : m_bombs) {
                    if (bomb.ownerEnemyIndex != enemyIndex) continue;
                    int d = abs(bomb.x - nx) + abs(bomb.y - ny);
                    if (d < minDist) minDist = d;
                }

                // 只有比当前位置更远（或至少持平）才考虑
                if (minDist > bestDist) {
                    bestDist = minDist;
                    bestDir = i;
                }
            }

            // 执行逃跑：只要找到一个方向就移动
            if (bestDir != -1) {
                enemy.x += dirs[bestDir][0];
                enemy.y += dirs[bestDir][1];
                enemy.dir = enemyDirs[bestDir];
            }
            // 无论是否成功逃脱，本帧不再进行随机移动和放炸弹
            continue;
        }

        // ============================================================
        // 2. 随机移动
        // ============================================================
        enemy.moveTimer -= 30;
        if (enemy.moveTimer <= 0) {
            enemy.moveTimer = rand() % 500 + 300;
            int dir = rand() % 4;
            int nx = enemy.x, ny = enemy.y;
            switch (dir) {
            case ENEMY_LEFT:  nx--; break;
            case ENEMY_RIGHT: nx++; break;
            case ENEMY_UP:    ny--; break;
            case ENEMY_DOWN:  ny++; break;
            }
            if (IsPositionFreeForEnemy(nx, ny) &&
                !IsOwnBombDangerous(nx, ny, enemyIndex)) {
                enemy.x = nx;
                enemy.y = ny;
                enemy.dir = dir;
            }
        }

        // ============================================================
        // 3. 放置炸弹（降低频率 + 检查逃路）
        // ============================================================
        enemy.bombTimer -= 30;
        if (enemy.bombTimer <= 0) {
            enemy.bombTimer = rand() % 3000 + 3000;   // 3~6 秒冷却

            if (m_map[enemy.y][enemy.x] == TileType::EMPTY &&
                !IsOwnBombDangerous(enemy.x, enemy.y, enemyIndex)) {

                bool hasBomb = false;
                for (const auto& b : m_bombs) {
                    if (b.x == enemy.x && b.y == enemy.y) {
                        hasBomb = true; break;
                    }
                }

                if (!hasBomb) {
                    // 检查放完后是否有可逃路线（至少1格邻接可走）
                    bool canEscape = false;
                    int dirs[4][2] = { {-1,0}, {1,0}, {0,-1}, {0,1} };
                    for (int i = 0; i < 4; ++i) {
                        int nx = enemy.x + dirs[i][0];
                        int ny = enemy.y + dirs[i][1];
                        if (IsValidPosition(nx, ny) &&
                            IsTileEmpty(nx, ny) &&
                            IsPositionFreeForEnemy(nx, ny)) {
                            canEscape = true;
                            break;
                        }
                    }

                    if (canEscape) {
                        PlaceBomb(enemy.x, enemy.y, false, enemyIndex);
                    }
                }
            }
        }
    }
}
// ---------------------- 玩家移动（一格一格走） ----------------------
void CBubbleBattleView::TryMovePlayer(int playerIndex, int dx, int dy)
{
    double curX = (playerIndex == 0) ? m_playerPixelX : m_player2PixelX;
    double curY = (playerIndex == 0) ? m_playerPixelY : m_player2PixelY;

    int gridX = static_cast<int>(curX / TILE_SIZE);
    int gridY = static_cast<int>(curY / TILE_SIZE);
    int newGridX = gridX + dx;
    int newGridY = gridY + dy;

    // 1. 边界
    if (newGridX < 0 || newGridX >= MAP_COLS) return;
    if (newGridY < 0 || newGridY >= MAP_ROWS) return;

    // 2. 地图必须为空
    if (m_map[newGridY][newGridX] != TileType::EMPTY) return;

    // 3. 不能有炸弹
    for (const auto& bomb : m_bombs) {
        if (bomb.x == newGridX && bomb.y == newGridY) return;
    }

    // 4. 不能有敌人
    for (const auto& enemy : m_enemies) {
        if (enemy.alive && enemy.x == newGridX && enemy.y == newGridY) return;
    }

    // 5. 不能踩到另一个玩家
    if (playerIndex == 0) {
        int ox = static_cast<int>(m_player2PixelX / TILE_SIZE);
        int oy = static_cast<int>(m_player2PixelY / TILE_SIZE);
        if (newGridX == ox && newGridY == oy) return;
    }
    else {
        int ox = static_cast<int>(m_playerPixelX / TILE_SIZE);
        int oy = static_cast<int>(m_playerPixelY / TILE_SIZE);
        if (newGridX == ox && newGridY == oy) return;
    }

    // 6. 更新坐标
    if (playerIndex == 0) {
        m_playerPixelX = newGridX * TILE_SIZE;
        m_playerPixelY = newGridY * TILE_SIZE;
    }
    else {
        m_player2PixelX = newGridX * TILE_SIZE;
        m_player2PixelY = newGridY * TILE_SIZE;
    }
}
// ---------------------- 炸弹/爆炸更新 ----------------------
void CBubbleBattleView::UpdateBombsAndExplosions()
{
    for (size_t i = 0; i < m_bombs.size(); ++i) {
        m_bombs[i].timeLeft--;
        if (m_bombs[i].timeLeft <= 0 && !m_bombs[i].exploded) {
            ExplodeBomb(static_cast<int>(i));
            if (m_gameOver) return;
            --i;
        }
    }

    for (size_t i = 0; i < m_explosions.size(); ) {
        m_explosions[i].timeLeft--;
        if (m_explosions[i].timeLeft <= 0) {
            m_explosions.erase(m_explosions.begin() + i);
        }
        else {
            ++i;
        }
    }
}

// ---------------------- 定时器 ----------------------
void CBubbleBattleView::OnTimer(UINT_PTR nIDEvent)
{
    if (nIDEvent != TIMER_ID) {
        CView::OnTimer(nIDEvent);
        return;
    }

    // ---------- 游戏结束处理（只弹一次） ----------
    if (m_gameOver)
    {
        if (m_endDialogShown) return;
        m_endDialogShown = true;

        KillTimer(TIMER_ID);

        CString msg = m_endDialogMsg.IsEmpty()
            ? _T("游戏结束！")
            : m_endDialogMsg;
        AfxMessageBox(msg);

        m_endDialogMsg.Empty();
        m_endDialogShown = false;

        // ✅ 回到 UI 视图
        CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
        if (pFrame != nullptr)
        {
            pFrame->SwitchToView(RUNTIME_CLASS(CBubbleGameUIView));
        }
        return;
    }

    // ---------- 正常游戏逻辑 ----------
        // ---------- 正常游戏逻辑 ----------
    if (m_gameMode == MODE_CLASSIC) {
        UpdateEnemies();     // 只有经典模式有敌人
    }
    UpdateBombsAndExplosions();
    Invalidate(FALSE);

    CView::OnTimer(nIDEvent);
}

void CBubbleBattleView::OnDraw(CDC* pDC)
{

    // 双缓冲
    CDC memDC;
    memDC.CreateCompatibleDC(pDC);
    CBitmap memBitmap;
    memBitmap.CreateCompatibleBitmap(pDC, GAME_WIDTH, GAME_HEIGHT);
    CBitmap* pOldBitmap = memDC.SelectObject(&memBitmap);

    // 背景大图
    if (m_bmpBackground.GetSafeHandle() != NULL) {
        BITMAP bm;
        m_bmpBackground.GetBitmap(&bm);
        CDC bgDC;
        bgDC.CreateCompatibleDC(&memDC);
        CBitmap* pOldBg = bgDC.SelectObject(&m_bmpBackground);
        memDC.StretchBlt(0, 0, GAME_WIDTH, GAME_HEIGHT,
            &bgDC, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
        bgDC.SelectObject(pOldBg);
        bgDC.DeleteDC();
    }
    else {
        memDC.FillSolidRect(0, 0, GAME_WIDTH, GAME_HEIGHT, RGB(250, 224, 225));
    }

    // 地图瓦片
    for (int y = 0; y < MAP_ROWS; ++y) {
        for (int x = 0; x < MAP_COLS; ++x) {
            if (m_map[y][x] == TileType::BRICK || m_map[y][x] == TileType::STEEL) {
                DrawTile(memDC, x, y, m_map[y][x]);
            }
        }
    }

    // 炸弹
    for (const auto& bomb : m_bombs) {
        DrawBomb(memDC, bomb);
    }

    // 爆炸
    for (const auto& exp : m_explosions) {
        DrawExplosion(memDC, exp);
    }

    // 敌人
    for (const auto& enemy : m_enemies) {
        if (enemy.alive) {
            DrawEnemy(memDC, enemy);
        }
    }

    // 玩家1
    DrawPlayer(memDC, 0);

    // 玩家2（仅双人模式）
    if (m_gameMode == MODE_BUBBLE) {
        DrawPlayer(memDC, 1);
    }

    // 缩放输出
    CRect clientRect;
    GetClientRect(&clientRect);
    int side = min(clientRect.Width(), clientRect.Height());
    int offsetX = (clientRect.Width() - side) / 2;
    int offsetY = (clientRect.Height() - side) / 2;

    pDC->StretchBlt(offsetX, offsetY, side, side,
        &memDC, 0, 0, GAME_WIDTH, GAME_HEIGHT, SRCCOPY);

    memDC.SelectObject(pOldBitmap);
    memBitmap.DeleteObject();
    memDC.DeleteDC();
}
// ---------------------- 绘制砖块 ----------------------
void CBubbleBattleView::DrawTile(CDC& dc, int x, int y, TileType type)
{
    CRect rect(x * TILE_SIZE, y * TILE_SIZE,
        (x + 1) * TILE_SIZE, (y + 1) * TILE_SIZE);

    // 选择对应的位图
    CBitmap* pBmp = nullptr;
    if (type == TileType::BRICK) {
        pBmp = &m_bmpTileBrick;
    }
    else if (type == TileType::STEEL) {
        pBmp = &m_bmpTileSteel;   // ✅ 改用图片
    }

    // 用图片绘制
    if (pBmp && pBmp->GetSafeHandle() != NULL) {
        BITMAP bm;
        pBmp->GetBitmap(&bm);
        CDC memDC;
        memDC.CreateCompatibleDC(&dc);
        CBitmap* pOld = memDC.SelectObject(pBmp);
        dc.TransparentBlt(rect.left, rect.top, TILE_SIZE, TILE_SIZE,
            &memDC, 0, 0, bm.bmWidth, bm.bmHeight,
            RGB(255, 255, 255));
        memDC.SelectObject(pOld);
        memDC.DeleteDC();
    }
}

// ---------------------- 绘制玩家 ----------------------
void CBubbleBattleView::DrawPlayer(CDC& dc, int playerIndex)
{
    double px = (playerIndex == 0) ? m_playerPixelX : m_player2PixelX;
    double py = (playerIndex == 0) ? m_playerPixelY : m_player2PixelY;
    PlayerDir dir = (playerIndex == 0) ? m_playerDir : m_player2Dir;

    int drawX = static_cast<int>(px);
    int drawY = static_cast<int>(py);

    // 根据玩家索引和方向选择对应位图
    CBitmap* pBmp = nullptr;

    if (playerIndex == 0)
    {
        // 玩家1
        switch (dir) {
        case DIR_UP:    pBmp = &m_bmpPlayerUp;    break;
        case DIR_DOWN:  pBmp = &m_bmpPlayerDown;  break;
        case DIR_LEFT:  pBmp = &m_bmpPlayerLeft;  break;
        case DIR_RIGHT: pBmp = &m_bmpPlayerRight; break;
        }
    }
    else
    {
        // 玩家2
        switch (dir) {
        case DIR_UP:    pBmp = &m_bmpPlayer2Up;    break;
        case DIR_DOWN:  pBmp = &m_bmpPlayer2Down;  break;
        case DIR_LEFT:  pBmp = &m_bmpPlayer2Left;  break;
        case DIR_RIGHT: pBmp = &m_bmpPlayer2Right; break;
        }
    }

    if (!pBmp || pBmp->GetSafeHandle() == NULL) return;

    BITMAP bm;
    pBmp->GetBitmap(&bm);

    int offsetX = (TILE_SIZE - bm.bmWidth) / 2;
    int offsetY = (TILE_SIZE - bm.bmHeight) / 2;

    CDC memDC;
    memDC.CreateCompatibleDC(&dc);
    CBitmap* pOld = memDC.SelectObject(pBmp);
    dc.TransparentBlt(drawX + offsetX, drawY + offsetY,
        bm.bmWidth, bm.bmHeight,
        &memDC, 0, 0, bm.bmWidth, bm.bmHeight,
        RGB(255, 255, 255));
    memDC.SelectObject(pOld);
    memDC.DeleteDC();

    // 玩家2 头顶加一个小红点，便于区分
    if (playerIndex == 1)
    {
        CBrush brush(RGB(255, 50, 50));
        CPen pen(PS_SOLID, 2, RGB(180, 0, 0));
        CBrush* oldBrush = dc.SelectObject(&brush);
        CPen* oldPen = dc.SelectObject(&pen);
        int cx = drawX + TILE_SIZE / 2;
        int cy = drawY - 4;
        dc.Ellipse(cx - 6, cy - 6, cx + 6, cy + 6);
        dc.SelectObject(oldBrush);
        dc.SelectObject(oldPen);
    }
}
// ---------------------- 绘制敌人 ----------------------
void CBubbleBattleView::DrawEnemy(CDC& dc, const Enemy& enemy)
{
    if (enemy.type < 0 || enemy.type >= 4) return;
    if (enemy.dir < 0 || enemy.dir >= 4) return;

    CBitmap* pBmp = &m_enemyBitmaps[enemy.type][enemy.dir];
    if (!pBmp || pBmp->GetSafeHandle() == NULL) return;

    BITMAP bm;
    pBmp->GetBitmap(&bm);
    int destX = enemy.x * TILE_SIZE + (TILE_SIZE - bm.bmWidth) / 2;
    int destY = enemy.y * TILE_SIZE + (TILE_SIZE - bm.bmHeight) / 2;

    CDC memDC;
    memDC.CreateCompatibleDC(&dc);
    CBitmap* pOld = memDC.SelectObject(pBmp);
    dc.TransparentBlt(destX, destY, bm.bmWidth, bm.bmHeight,
        &memDC, 0, 0, bm.bmWidth, bm.bmHeight,
        RGB(255, 255, 255));
    memDC.SelectObject(pOld);
    memDC.DeleteDC();
}

// ---------------------- 绘制炸弹 ----------------------
void CBubbleBattleView::DrawBomb(CDC& dc, const Bomb& bomb)
{
    int left = bomb.x * TILE_SIZE + 8;
    int top = bomb.y * TILE_SIZE + 8;
    int size = TILE_SIZE - 16;
    CRect rect(left, top, left + size, top + size);

    CBrush bombBrush(RGB(60, 60, 60));
    CBrush* oldBrush = dc.SelectObject(&bombBrush);
    dc.Ellipse(&rect);
    dc.SelectObject(oldBrush);

    CBrush highlightBrush(RGB(120, 120, 120));
    oldBrush = dc.SelectObject(&highlightBrush);
    CRect hlRect(left + size / 4, top + size / 4,
        left + size / 2, top + size / 2);
    dc.Ellipse(&hlRect);
    dc.SelectObject(oldBrush);

    CPen fusePen(PS_SOLID, 2, RGB(255, 0, 0));
    CPen* oldPen = dc.SelectObject(&fusePen);
    dc.MoveTo(left + size / 2, top);
    dc.LineTo(left + size / 2 + 5, top - 6);
    dc.LineTo(left + size / 2 + 8, top - 2);

    CBrush sparkBrush(RGB(255, 255, 0));
    dc.SelectObject(&sparkBrush);
    CRect sparkRect(left + size / 2 + 8 - 2, top - 2 - 2,
        left + size / 2 + 8 + 2, top - 2 + 2);
    dc.Ellipse(&sparkRect);
    dc.SelectObject(oldBrush);
    dc.SelectObject(oldPen);
}

// ---------------------- 绘制爆炸 ----------------------
void CBubbleBattleView::DrawExplosion(CDC& dc, const Explosion& exp)
{
    int left = exp.x * TILE_SIZE;
    int top = exp.y * TILE_SIZE;
    CRect rect(left, top, left + TILE_SIZE, top + TILE_SIZE);

    dc.FillSolidRect(&rect, RGB(255, 150, 0));
    CRect inner = rect;
    inner.DeflateRect(8, 8);
    dc.FillSolidRect(&inner, RGB(255, 200, 0));
    CRect core = rect;
    core.DeflateRect(15, 15);
    dc.FillSolidRect(&core, RGB(255, 255, 200));
}

void CBubbleBattleView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    if (m_gameOver) return;

    bool twoPlayer = (m_gameMode == MODE_BUBBLE);

    switch (nChar)
    {
        // ============ 玩家1：方向键 + Enter ============
    case VK_LEFT:
        m_playerDir = DIR_LEFT;
        TryMovePlayer(0, -1, 0);
        break;
    case VK_RIGHT:
        m_playerDir = DIR_RIGHT;
        TryMovePlayer(0, 1, 0);
        break;
    case VK_UP:
        m_playerDir = DIR_UP;
        TryMovePlayer(0, 0, -1);
        break;
    case VK_DOWN:
        m_playerDir = DIR_DOWN;
        TryMovePlayer(0, 0, 1);
        break;
    case VK_RETURN:
    {
        int gx = static_cast<int>(m_playerPixelX / TILE_SIZE);
        int gy = static_cast<int>(m_playerPixelY / TILE_SIZE);
        PlaceBomb(gx, gy, true, -1);
        break;
    }

    // ============ 玩家2：WASD + 空格（仅双人模式） ============
    case 'W':
        if (twoPlayer) {
            m_player2Dir = DIR_UP;
            TryMovePlayer(1, 0, -1);
        }
        break;
    case 'S':
        if (twoPlayer) {
            m_player2Dir = DIR_DOWN;
            TryMovePlayer(1, 0, 1);
        }
        break;
    case 'A':
        if (twoPlayer) {
            m_player2Dir = DIR_LEFT;
            TryMovePlayer(1, -1, 0);
        }
        break;
    case 'D':
        if (twoPlayer) {
            m_player2Dir = DIR_RIGHT;
            TryMovePlayer(1, 1, 0);
        }
        break;
    case VK_SPACE:
        if (twoPlayer) {
            int gx = static_cast<int>(m_player2PixelX / TILE_SIZE);
            int gy = static_cast<int>(m_player2PixelY / TILE_SIZE);
            PlaceBomb(gx, gy, true, -1);
        }
        break;
    }

    Invalidate(FALSE);
    CView::OnKeyDown(nChar, nRepCnt, nFlags);
}
void CBubbleBattleView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    // 现在是一格一格移动，不需要按键状态处理
    CView::OnKeyUp(nChar, nRepCnt, nFlags);
}
// ---------------------- 其他 ----------------------
BOOL CBubbleBattleView::PreCreateWindow(CREATESTRUCT& cs)
{
    return CView::PreCreateWindow(cs);
}

BOOL CBubbleBattleView::OnPreparePrinting(CPrintInfo* pInfo)
{
    return DoPreparePrinting(pInfo);
}

void CBubbleBattleView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/) {}
void CBubbleBattleView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/) {}

#ifdef _DEBUG
void CBubbleBattleView::AssertValid() const
{
    CView::AssertValid();
}

void CBubbleBattleView::Dump(CDumpContext& dc) const
{
    CView::Dump(dc);
}

CBubbleBattleDoc* CBubbleBattleView::GetDocument() const
{
    ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CBubbleBattleDoc)));
    return (CBubbleBattleDoc*)m_pDocument;
}
#endif