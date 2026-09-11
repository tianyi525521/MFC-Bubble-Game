#pragma once

struct GameConfig
{
    int mode;           // 0 = 泡泡双人，1 = 经典单人
    int enemyCount;     // 经典模式敌人数量：1 或 3
    int p1Char;         // 玩家1角色 0~3
    int p2Char;         // 玩家2角色 0~3
};

extern GameConfig g_gameConfig;