// Report.h - レポート
//
#pragma once

// 外部参照
//
#include <stdbool.h>
#include "pd_api.h"
#include "Actor.h"


// ビットマップ
//
enum {
    kReportBitmapSizeX = 128, 
    kReportBitmapSizeY = 160, 
};

// レポート
//
struct Report {

    // アクタ
    struct Actor actor;

    // ビットマップ
    LCDBitmap *bitmap;

};

// 外部参照関数
//
extern void ReportInitialize(void);
extern void ReportLoad(void);
