// Report.c - レポート
//

// 外部参照
//
#include <string.h>
#include "pd_api.h"
#include "Iocs.h"
#include "Actor.h"
#include "Game.h"
#include "Report.h"

// 内部関数
//
static void ReportUnload(struct Report *report);
static void ReportDraw(struct Report *report);
static void ReportLoop(struct Report *report);

// 内部変数
//


// レポートを初期化する
//
void ReportInitialize(void)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // アクタの確認
    if (sizeof (struct Report) > kActorBlockSize) {
        playdate->system->error("%s: %d: report actor size is over: %d bytes.", __FILE__, __LINE__, sizeof (struct Report));
    }
}

// レポートを読み込む
//
void ReportLoad(void)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // アクタの登録
    struct Report *report = (struct Report *)ActorLoad((ActorFunction)ReportLoop, kGamePriorityReport);
    if (report == NULL) {
        playdate->system->error("%s: %d: report actor is not loaded.", __FILE__, __LINE__);
    }

    // レポートの初期化
    {
        // 解放処理の設定
        ActorSetUnload(&report->actor, (ActorFunction)ReportUnload);

        // タグの設定
        ActorSetTag(&report->actor, kGameTagReport);

        // ビットマップの作成
        report->bitmap = playdate->graphics->newBitmap(kReportBitmapSizeX, kReportBitmapSizeY, kColorBlack);
        if (report->bitmap == NULL) {
            playdate->system->error("%s: %d: report bitmap is not created.", __FILE__, __LINE__);
        }
    }
}

// レポートを解放する
//
static void ReportUnload(struct Report *report)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // ビットマップの解放
    if (report->bitmap != NULL) {
        playdate->graphics->freeBitmap(report->bitmap);
    }
}

// レポートを描画する
//
static void ReportDraw(struct Report *report)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // レポートの描画
    GamePrintReport(report->bitmap);

    // ビットマップの描画
    playdate->graphics->setDrawMode(kDrawModeCopy);
    playdate->graphics->drawBitmap(report->bitmap, LCD_COLUMNS - kReportBitmapSizeX, 0, kBitmapUnflipped);
}

// レポートが待機する
//
static void ReportLoop(struct Report *report)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // 初期化
    if (report->actor.state == 0) {

        // 初期化の完了
        ++report->actor.state;
    }

    // 描画処理の設定
    ActorSetDraw(&report->actor, (ActorFunction)ReportDraw, kGameOrderReport);
}

