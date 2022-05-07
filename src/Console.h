// Console.h - コンソール
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
    kConsoleBitmapSizeX = 400, 
    kConsoleBitmapSizeY = 240, 
};

// コンソール
//
struct Console {

    // アクタ
    struct Actor actor;

    // ビットマップ
    LCDBitmap *bitmap;

    // カーソル位置
    int cursorX;
    int cursorY;

    // テキスト
    const char *text;

    // メニュー
    const char **menuItems;
    int menuSize;
    int menuWidth;
    int menuCursor;
    int menuDone;
    float menuCrank;
    float menuCrankInterval;
    bool menuUpdate;

    // 数値入力
    int numberDigit;
    int numberWidth;
    int numberMinimum;
    int numberMaximum;
    int numberInput;
    int numberDone;
    float numberCrank;
    float numberCrankInterval;
    bool numberUpdate;

    // 角度入力
    int angleWidth;
    float angleInput;
    float angleDone;
    bool angleUpdate;

};

// 外部参照関数
//
extern void ConsoleInitialize(void);
extern void ConsoleLoad(void);
extern void ConsolePrintText(const char *text);
extern bool ConsoleIsPrintText(void);
extern void ConsoleOpenMenu(const char **items, int size);
extern bool ConsoleIsOpenMenu(void);
extern int ConsoleGetSelectedMenu(void);
extern void ConsoleInputNumber(int number, int minimum, int maximum);
extern bool ConsoleIsInputNumber(void);
extern int ConsoleGetInputedNumber(void);
extern void ConsoleInputAngle(void);
extern bool ConsoleIsInputAngle(void);
extern int ConsoleGetInputedAngle(void);

