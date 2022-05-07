// Scene.c - シーン
//

// 外部参照
//
#include <string.h>
#include "pd_api.h"
#include "Iocs.h"
#include "Scene.h"

// 内部関数
//

// 内部変数
//
static struct SceneController *sceneController = NULL;


// シーンを初期化する
//
void SceneInitialize(void)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // シーンコントローラの作成
    sceneController = playdate->system->realloc(NULL, sizeof (struct SceneController));
    if (sceneController == NULL) {
        playdate->system->error("%s: %d: scene controller instance is not created.", __FILE__, __LINE__);
        return;
    }
    memset(sceneController, 0, sizeof (struct SceneController));
}

// シーンの更新を開始する
//
void SceneUpdateBegin(void)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // シーンの更新
    if (sceneController->update != NULL) {
        (*sceneController->update)(sceneController->userdata);
    }
}

// シーンの更新を終了する
//
void SceneUpdateEnd(void)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // 予約されたシーンの遷移
    if (sceneController->transition != NULL) {
        if (sceneController->unload != NULL) {
            (*sceneController->unload)(sceneController->userdata);
            sceneController->unload = NULL;
        }
        if (sceneController->userdata != NULL) {
            playdate->system->realloc(sceneController->userdata, 0);
            sceneController->userdata = NULL;
        }
        sceneController->update = sceneController->transition;
        sceneController->transition = NULL;
    }
}

// 遷移を予約する
//
void SceneTransition(SceneFunction transition)
{
    sceneController->transition = transition;
}

// シーンの解放処理を設定する
//
void SceneSetUnload(SceneFunction unload)
{
    sceneController->unload = unload;
}

// ユーザデータを設定する
//
void SceneSetUserdata(void *userdata)
{
    sceneController->userdata = userdata;
}

// ユーザデータを取得する
//
void *SceneGetUserdata(void)
{
   return  sceneController->userdata;
}

