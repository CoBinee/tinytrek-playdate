// Scene.h - シーン
//
#pragma once

// 外部参照
//
#include <stdbool.h>
#include "pd_api.h"


// シーン関数
//
typedef void (*SceneFunction)(void *);

// シーンコントローラ
//
struct SceneController {

    // 更新処理
    SceneFunction update;

    // 解放処理
    SceneFunction unload;

    // 遷移
    SceneFunction transition;

    // ユーザデータ
    void *userdata;

};


// 外部参照関数
//
extern void SceneInitialize(void);
extern void SceneUpdateBegin(void);
extern void SceneUpdateEnd(void);
extern void SceneTransition(SceneFunction transition);
extern void SceneSetUnload(SceneFunction unload);
extern void SceneSetUserdata(void *userdata);
extern void *SceneGetUserdata(void);

