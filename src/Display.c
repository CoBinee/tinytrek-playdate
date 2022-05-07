// Display.c - ディスプレイ
//

// 外部参照
//
#include <string.h>
#include "pd_api.h"
#include "Iocs.h"
#include "Actor.h"
#include "Game.h"
#include "Display.h"

// 内部関数
//
static void DisplayUnload(struct Display *display);
static void DisplayDraw(struct Display *display);
static void DisplayLoop(struct Display *display);

// 内部変数
//


// ディスプレイを初期化する
//
void DisplayInitialize(void)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // アクタの確認
    if (sizeof (struct Display) > kActorBlockSize) {
        playdate->system->error("%s: %d: display actor size is over: %d bytes.", __FILE__, __LINE__, sizeof (struct Display));
    }
}

// ディスプレイを読み込む
//
void DisplayLoad(void)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // アクタの登録
    struct Display *display = (struct Display *)ActorLoad((ActorFunction)DisplayLoop, kGamePriorityDisplay);
    if (display == NULL) {
        playdate->system->error("%s: %d: display actor is not loaded.", __FILE__, __LINE__);
    }

    // ディスプレイの初期化
    {
        // 解放処理の設定
        ActorSetUnload(&display->actor, (ActorFunction)DisplayUnload);

        // タグの設定
        ActorSetTag(&display->actor, kGameTagDisplay);

        // ビットマップの作成
        display->bitmap = playdate->graphics->newBitmap(kDisplayBitmapSizeX, kDisplayBitmapSizeY, kColorBlack);
        if (display->bitmap == NULL) {
            playdate->system->error("%s: %d: display bitmap is not created.", __FILE__, __LINE__);
        }

        // マップの設定
        display->map = kDisplayMapNull;
    }
}

// ディスプレイを解放する
//
static void DisplayUnload(struct Display *display)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // ビットマップの解放
    if (display->bitmap != NULL) {
        playdate->graphics->freeBitmap(display->bitmap);
    }
}

// ディスプレイを描画する
//
static void DisplayDraw(struct Display *display)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // マップの描画
    if (display->map == kDisplayMapGalaxy) {
        GamePrintGalaxyMap(display->bitmap);
    } else if (display->map == kDisplayMapSector && GameIsShortRangeSensorAvilable()) {
        GamePrintSectorMap(display->bitmap);
    } else {
        GamePrintNoMap(display->bitmap);
    }

    // ビットマップの描画
    playdate->graphics->setDrawMode(kDrawModeCopy);
    playdate->graphics->drawBitmap(display->bitmap, 0, 0, kBitmapUnflipped);
}

// ディスプレイが待機する
//
static void DisplayLoop(struct Display *display)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // 初期化
    if (display->actor.state == 0) {

        // 初期化の完了
        ++display->actor.state;
    }

    // 描画処理の設定
    ActorSetDraw(&display->actor, (ActorFunction)DisplayDraw, kGameOrderDisplay);
}

// 表示するマップを設定する
//
void DisplaySetMap(DisplayMap map)
{
    struct Display *display = (struct Display *)ActorFindWithTag(kGameTagDisplay);
    if (display != NULL) {
        display->map = map;
    }
}
