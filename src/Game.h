// Game.h - ゲーム
//
#pragma once

// 外部参照
//
#include <stdbool.h>
#include "pd_api.h"


// Tiny Basic
//
enum {
    kGameBasicArraySize = 1024, 
};
enum {
    kGameBasicReturnSize = 16, 
};
struct Pointer {
    int line;
    int sentence;
};
struct Basic {

    // 変数
    int A;
    int B;
    int C;
    int D;
    int E;
    int F;
    int G;
    int H;
    int I;
    int J;
    int K;
    int L;
    int M;
    int N;
    int O;
    int P;
    int Q;
    int R;
    int S;
    int T;
    int U;
    int V;
    int W;
    int X;
    int Y;
    int Z;

    // 配列
    int at[kGameBasicArraySize];

    // 実行
    struct Pointer run;

    // GOSUB - RETURN
    struct Pointer returns[kGameBasicReturnSize];
    int returnStack;

};

// ゲーム関数
//
typedef void (*GameFunction)(void *game);

// テキスト
//
enum {
    kGameTextSize = 1024, 
};

// ゲーム
//
struct Game {

    // 処理関数
    GameFunction function;

    // 状態
    int state;

    // Tiny Basic
    struct Basic basic;

    // テキスト
    char text[kGameTextSize];

    // クラシック
    bool classic;

};

// スプライト
//
typedef enum {
    kGameSpriteNameSize, 
} GameSpriteName;

// オーディオ
//
enum {
    kGameAudioSampleSize, 
};

// プライオリティ
//
enum {
    kGamePriorityNull = 0, 
    kGamePriorityConsole, 
    kGamePriorityDisplay, 
    kGamePriorityReport, 
};

// タグ
//
enum {
    kGameTagNull = 0, 
    kGameTagConsole, 
    kGameTagDisplay, 
    kGameTagReport, 
};

// 描画順
//
enum {
    kGameOrderNull = 0, 
    kGameOrderConsole, 
    kGameOrderDisplay, 
    kGameOrderReport, 
};


// 外部参照関数
//
extern void GameUpdate(struct Game *game);
extern bool GameIsClassic(void);
extern bool GameIsShortRangeSensorAvilable(void);
extern bool GameIsLongRangeSensorAvilable(void);
extern bool GameIsComputerDisplayAvilable(void);
extern void GamePrintNoMap(LCDBitmap *bitmap);
extern void GamePrintGalaxyMap(LCDBitmap *bitmap);
extern void GamePrintSectorMap(LCDBitmap *bitmap);
extern void GamePrintReport(LCDBitmap *bitmap);

