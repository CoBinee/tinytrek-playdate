// Title.h - タイトル
//
#pragma once

// 外部参照
//
#include <stdbool.h>
#include "pd_api.h"


// タイトル関数
//
typedef void (*TitleFunction)(void *game);

// タイトル
//
struct Title {

    // 処理関数
    TitleFunction function;

    // 状態
    int state;
};

// スプライト
//
typedef enum {
    kTitleSpriteNameNull = 0, 
    kTitleSpriteNameSize, 
} TitleSpriteName;

// プライオリティ
//
enum {
    kTitlePriorityNull = 0, 
};

// タグ
//
enum {
    kTitleTagNull = 0, 
};

// 描画順
//
enum {
    kTitleOrderNull = 0, 
};


// 外部参照関数
//
extern void TitleUpdate(struct Title *title);
