// Display.h - ディスプレイ
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
    kDisplayBitmapSizeX = 272, 
    kDisplayBitmapSizeY = 160, 
};

// マップ
//
typedef enum {
    kDisplayMapNull = 0, 
    kDisplayMapGalaxy, 
    kDisplayMapSector, 
} DisplayMap;

// ディスプレイ
//
struct Display {

    // アクタ
    struct Actor actor;

    // ビットマップ
    LCDBitmap *bitmap;

    // マップ
    DisplayMap map;

};

// 外部参照関数
//
extern void DisplayInitialize(void);
extern void DisplayLoad(void);
extern void DisplaySetMap(DisplayMap map);
