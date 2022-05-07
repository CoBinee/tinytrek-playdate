// Application.c - アプリケーション
//

// 外部参照
//
#include <stdbool.h>
#include "pd_api.h"
#include "Iocs.h"
#include "Scene.h"
#include "Application.h"
#include "Title.h"
#include "Game.h"

// 内部関数
//

// 内部変数
//
static SceneFunction functions[kApplicationSceneSize] = {
    NULL, 
    (SceneFunction)TitleUpdate, 
    (SceneFunction)GameUpdate, 
};
static struct Application *application = NULL;


// アプリケーションを初期化する
//
void ApplicationInitialize(void)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // アプリケーションの作成
    application = playdate->system->realloc(NULL, sizeof (struct Application));
    if (application == NULL) {
        playdate->system->error("%s: %d: application instance is not created.", __FILE__, __LINE__);
        return;
    }
    memset(application, 0, sizeof (struct Application));

    // アプリケーションの初期化
    {
        // スコアの設定
        application->score = kApplicationScoreDefault;
    }

    // シーンの遷移
    ApplicationTransition(kApplicationSceneGame);
}

// アプリケーションのシーンを遷移する
//
void ApplicationTransition(ApplicationScene scene)
{
    SceneTransition(functions[scene]);
}

// スコアを取得する
//
int ApplicationGetScore(void)
{
    return application != NULL ? application->score : 0;
}

// スコアを設定する
//
bool ApplicationSetScore(int score)
{
    bool result = false;
    if (application != NULL && application->score < score) {
        application->score = score;
        result = true;
    }
    return result;
}


