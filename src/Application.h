// Application.h - アプリケーション
//
#pragma once

// 外部参照
//
#include <stdbool.h>
#include "pd_api.h"


// シーン
//
typedef enum {
    kApplicationSceneNull = 0, 
    kApplicationSceneTitle, 
    kApplicationSceneGame, 
    kApplicationSceneSize, 
} ApplicationScene;

// スコア
//
enum {
    kApplicationScoreDefault = 5000, 
};

// アプリケーション
//
struct Application {

    int score;
};


// 外部参照関数
//
extern void ApplicationInitialize(void);
extern void ApplicationTransition(ApplicationScene scene);
extern int ApplicationGetScore(void);
extern bool ApplicationSetScore(int score);
