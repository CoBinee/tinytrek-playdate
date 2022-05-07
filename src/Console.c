// Console.c - コンソール
//

// 外部参照
//
#include <string.h>
#include "pd_api.h"
#include "Iocs.h"
#include "Actor.h"
#include "Game.h"
#include "Console.h"

// 内部関数
//
static void ConsoleUnload(struct Console *console);
static void ConsoleDraw(struct Console *console);
static void ConsoleNewLine(struct Console *console);
static void ConsoleLoop(struct Console *console);

// 内部変数
//


// コンソールを初期化する
//
void ConsoleInitialize(void)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // アクタの確認
    if (sizeof (struct Console) > kActorBlockSize) {
        playdate->system->error("%s: %d: console actor size is over: %d bytes.", __FILE__, __LINE__, sizeof (struct Console));
    }
}

// コンソールを読み込む
//
void ConsoleLoad(void)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // アクタの登録
    struct Console *console = (struct Console *)ActorLoad((ActorFunction)ConsoleLoop, kGamePriorityConsole);
    if (console == NULL) {
        playdate->system->error("%s: %d: console actor is not loaded.", __FILE__, __LINE__);
    }

    // コンソールの初期化
    {
        // 解放処理の設定
        ActorSetUnload(&console->actor, (ActorFunction)ConsoleUnload);

        // タグの設定
        ActorSetTag(&console->actor, kGameTagConsole);

        // ビットマップの作成
        console->bitmap = playdate->graphics->newBitmap(kConsoleBitmapSizeX, kConsoleBitmapSizeY, kColorBlack);
        if (console->bitmap == NULL) {
            playdate->system->error("%s: %d: console bitmap is not created.", __FILE__, __LINE__);
        }

        // 位置の設定
        console->cursorX = 0;
        console->cursorY = kConsoleBitmapSizeY - IocsGetFontHeight(kIocsFontJapanese);

        // テキストの設定
        console->text = NULL;

        // メニューの設定
        console->menuItems = NULL;
        console->menuCrankInterval = 30.0f;
        console->menuUpdate = false;

        // 数値入力の設定
        console->numberInput = -1;
        console->numberUpdate = false;

        // 角度入力の設定
        console->angleWidth = IocsGetTextWidth(kIocsFontJapanese, "999");
        console->angleInput = -1.0f;
        console->angleUpdate = false;
    }
}

// コンソールを解放する
//
static void ConsoleUnload(struct Console *console)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // ビットマップの解放
    if (console->bitmap != NULL) {
        playdate->graphics->freeBitmap(console->bitmap);
    }
}

// コンソールを描画する
//
static void ConsoleDraw(struct Console *console)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // テキストの描画
    if (console->text != NULL) {
        bool skip = (IocsIsButtonPush(kButtonB) || IocsIsButtonPush(kButtonLeft)) ? true : false;
        do {
            char letter[8];
            int length = IocsGetCharByte(*console->text);
            {
                strncpy(letter, console->text, length);
                letter[length] = '\0';
                console->text += length;
                if (*console->text == '\0') {
                    console->text = NULL;
                }
            }
            if (letter[0] == '\n') {
                ConsoleNewLine(console);
            } else {
                int width = IocsGetTextWidth(kIocsFontJapanese, letter);
                int height = IocsGetFontHeight(kIocsFontJapanese);
                if (console->cursorX + width > kConsoleBitmapSizeX) {
                    ConsoleNewLine(console);
                }
                playdate->graphics->pushContext(console->bitmap);
                IocsSetFont(kIocsFontJapanese);
                playdate->graphics->setDrawMode(kDrawModeFillWhite);
                playdate->graphics->drawText(letter, length, kUTF8Encoding, console->cursorX, console->cursorY);
                playdate->graphics->popContext();
                console->cursorX += width;
            }
        } while (skip && console->text != NULL);

    // メニューの描画
    } else if (console->menuUpdate) {
        if (console->menuItems != NULL) {
            const char *text = console->menuItems[console->menuCursor];
            int height = IocsGetFontHeight(kIocsFontJapanese);
            playdate->graphics->pushContext(console->bitmap);
            IocsSetFont(kIocsFontJapanese);
            playdate->graphics->setDrawMode(kDrawModeCopy);
            playdate->graphics->fillRect(console->cursorX, console->cursorY, console->menuWidth, height, kColorWhite);
            playdate->graphics->setDrawMode(kDrawModeFillBlack);
            playdate->graphics->drawText(text, strlen(text), kUTF8Encoding, console->cursorX, console->cursorY);
            playdate->graphics->popContext();
        } else {
            ConsoleNewLine(console);
        }
        console->menuUpdate = false;

    // 数値入力の描画
    } else if (console->numberUpdate) {
        if (console->numberInput >= 0) {
            char *text;
            playdate->system->formatString(&text, "%d", console->numberInput);
            int height = IocsGetFontHeight(kIocsFontJapanese);
            playdate->graphics->pushContext(console->bitmap);
            IocsSetFont(kIocsFontJapanese);
            playdate->graphics->setDrawMode(kDrawModeCopy);
            playdate->graphics->fillRect(console->cursorX, console->cursorY, console->numberWidth, height, kColorWhite);
            playdate->graphics->setDrawMode(kDrawModeFillBlack);
            playdate->graphics->drawText(text, strlen(text), kUTF8Encoding, console->cursorX, console->cursorY);
            playdate->graphics->popContext();
            playdate->system->realloc(text, 0);
        } else {
            ConsoleNewLine(console);
        }
        console->numberUpdate = false;

    // 角度入力の描画
    } else if (console->angleUpdate) {
        if (console->angleInput >= 0.0f) {
            char *text;
            playdate->system->formatString(&text, "%d", (int)console->angleInput);
            int height = IocsGetFontHeight(kIocsFontJapanese);
            playdate->graphics->pushContext(console->bitmap);
            IocsSetFont(kIocsFontJapanese);
            playdate->graphics->setDrawMode(kDrawModeCopy);
            playdate->graphics->fillRect(console->cursorX, console->cursorY, console->angleWidth, height, kColorWhite);
            playdate->graphics->setDrawMode(kDrawModeFillBlack);
            playdate->graphics->drawText(text, strlen(text), kUTF8Encoding, console->cursorX, console->cursorY);
            playdate->graphics->popContext();
            playdate->system->realloc(text, 0);
        } else {
            ConsoleNewLine(console);
        }
        console->angleUpdate = false;
    }

    // タイトルの描画
    if (!GameIsClassic()) {
        int height0 = IocsGetFontHeight(kIocsFontJapanese);
        int height1 = IocsGetFontHeight(kIocsFontMini);
        int row = 5;
        playdate->graphics->pushContext(console->bitmap);
        IocsSetFont(kIocsFontMini);
        playdate->graphics->fillRect(0, kConsoleBitmapSizeY - row * height0 + 0 * height1, kConsoleBitmapSizeX, height0, kColorBlack);
        char *text = "CONSOLE";
        playdate->graphics->drawText(text, strlen(text), kUTF8Encoding, 0, kConsoleBitmapSizeY - row * height0 + 0 * height1);
        playdate->graphics->drawLine(0, kConsoleBitmapSizeY - row * height0 + 1 * height1, kConsoleBitmapSizeX - 1, kConsoleBitmapSizeY - row * height0 + 1 * height1, 1, kColorWhite);
        playdate->graphics->popContext();
    }

    // ビットマップの描画
    playdate->graphics->setDrawMode(kDrawModeCopy);
    playdate->graphics->drawBitmap(console->bitmap, 0, 0, kBitmapUnflipped);
}

// 改行する
//
static void ConsoleNewLine(struct Console *console)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // 改行
    {
        int height = IocsGetFontHeight(kIocsFontJapanese);
        console->cursorX = 0;
        console->cursorY += height;
        if (console->cursorY + height > kConsoleBitmapSizeY) {
            playdate->graphics->pushContext(console->bitmap);
            playdate->graphics->setDrawMode(kDrawModeCopy);
            playdate->graphics->drawBitmap(console->bitmap, 0, -height, kBitmapUnflipped);
            playdate->graphics->fillRect(0, kConsoleBitmapSizeY - height, kConsoleBitmapSizeX, height, kColorBlack);
            playdate->graphics->popContext();
            console->cursorY = kConsoleBitmapSizeY - height;
        }
    }
}

// コンソールが待機する
//
static void ConsoleLoop(struct Console *console)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // 初期化
    if (console->actor.state == 0) {

        // 初期化の完了
        ++console->actor.state;
    }

    // メニューの更新
    if (console->menuItems != NULL) {
        {
            bool beep = false;
            console->menuCrank += IocsGetCrankChange();
            while (console->menuCrank <= -console->menuCrankInterval) {
                --console->menuCursor;
                if (console->menuCursor < 0) {
                    console->menuCursor = console->menuSize - 1;
                }
                console->menuUpdate = true;
                console->menuCrank += console->menuCrankInterval;
                beep = true;
            }
            while (console->menuCrank >= console->menuCrankInterval) {
                ++console->menuCursor;
                if (console->menuCursor >= console->menuSize) {
                    console->menuCursor = 0;
                }
                console->menuUpdate = true;
                console->menuCrank -= console->menuCrankInterval;
                beep = true;
            }
            if (beep) {
                IocsPlayAudioSystem(kIocsAudioSystemSamplePo, 1);
            }
        }
        if (IocsIsButtonEdge(kButtonA) || IocsIsButtonEdge(kButtonRight)) {
            console->menuItems = NULL;
            console->menuDone = console->menuCursor;
            console->menuUpdate = true;
            IocsPlayAudioSystem(kIocsAudioSystemSamplePi, 1);
        /*
        } else if (IocsIsButtonRepeat(kButtonUp)) {
            --console->menuCursor;
            if (console->menuCursor < 0) {
                console->menuCursor = console->menuSize - 1;
            }
            console->menuUpdate = true;
            IocsPlayAudioSystem(kIocsAudioSystemSamplePo, 1);
        } else if (IocsIsButtonRepeat(kButtonDown)) {
            ++console->menuCursor;
            if (console->menuCursor >= console->menuSize) {
                console->menuCursor = 0;
            }
            console->menuUpdate = true;
            IocsPlayAudioSystem(kIocsAudioSystemSamplePo, 1);
        */
        }

    // 数値入力の更新
    } else if (console->numberInput >= 0) {
        {
            bool beep = false;
            console->numberCrank += IocsGetCrankChange();
            while (console->numberCrank <= -console->numberCrankInterval) {
                --console->numberInput;
                if (console->numberInput < console->numberMinimum) {
                    console->numberInput = console->numberMaximum;
                }
                console->numberUpdate = true;
                console->numberCrank += console->numberCrankInterval;
                beep = true;
            }
            while (console->numberCrank >= console->numberCrankInterval) {
                ++console->numberInput;
                if (console->numberInput > console->numberMaximum) {
                    console->numberInput = console->numberMinimum;
                }
                console->numberUpdate = true;
                console->numberCrank -= console->numberCrankInterval;
                beep = true;
            }
            if (beep) {
                IocsPlayAudioSystem(kIocsAudioSystemSamplePo, 1);
            }
        }
        if (IocsIsButtonEdge(kButtonA) || IocsIsButtonEdge(kButtonRight)) {
            console->numberDone = console->numberInput;
            console->numberInput = -1;
            console->numberUpdate = true;
            IocsPlayAudioSystem(kIocsAudioSystemSamplePi, 1);
        } else if (IocsIsButtonEdge(kButtonB) || IocsIsButtonEdge(kButtonLeft)) {
            console->numberDone = -1;
            console->numberInput = -1;
            console->numberUpdate = true;
            IocsPlayAudioSystem(kIocsAudioSystemSamplePo, 1);
        /*
        } else if (IocsIsButtonRepeat(kButtonUp)) {
            --console->numberInput;
            if (console->numberInput < console->numberMinimum) {
                console->numberInput = console->numberMaximum;
            }
            console->numberUpdate = true;
            IocsPlayAudioSystem(kIocsAudioSystemSamplePo, 1);
        } else if (IocsIsButtonRepeat(kButtonDown)) {
            ++console->numberInput;
            if (console->numberInput > console->numberMaximum) {
                console->numberInput = console->numberMinimum;
            }
            console->numberUpdate = true;
            IocsPlayAudioSystem(kIocsAudioSystemSamplePo, 1);
        */
        }

    // 角度入力の更新
    } else if (console->angleInput >= 0.0f) {
        if (IocsIsButtonEdge(kButtonA)) {
            console->angleDone = console->angleInput;
            console->angleInput = -1.0f;
            console->angleUpdate = true;
            IocsPlayAudioSystem(kIocsAudioSystemSamplePi, 1);
        } else {
            console->angleInput = IocsGetCrankAngle();
            console->angleUpdate = true;
        }
    }

    // 描画処理の設定
    ActorSetDraw(&console->actor, (ActorFunction)ConsoleDraw, kGameOrderConsole);
}

// テキストを表示する
//
void ConsolePrintText(const char *text)
{
    // コンソールの取得
    struct Console *console = (struct Console *)ActorFindWithTag(kGameTagConsole);
    if (console == NULL) {
        return;
    }

    // テキストの設定
    console->text = text;
}

// テキストの表示が完了したかどうかを判定する
//
bool ConsoleIsPrintText(void)
{
    struct Console *console = (struct Console *)ActorFindWithTag(kGameTagConsole);
    return (console != NULL && console->text != NULL) ? true : false;
}

// メニューを開く
//
void ConsoleOpenMenu(const char **items, int size)
{
    // コンソールの取得
    struct Console *console = (struct Console *)ActorFindWithTag(kGameTagConsole);
    if (console == NULL) {
        return;
    }

    // メニューの設定
    console->menuItems = items;
    console->menuSize = size;
    console->menuWidth = 0;
    console->menuCursor = 0;
    console->menuDone = -1;
    console->menuCrank = 0.0f;
    console->menuUpdate = true;
    for (int i = 0; i < size; i++) {
        int width = IocsGetTextWidth(kIocsFontJapanese, items[i]);
        if (console->menuWidth < width) {
            console->menuWidth = width;
        }
    }
}

// メニューが開かれているかどうかを判定する
//
bool ConsoleIsOpenMenu(void)
{
    struct Console *console = (struct Console *)ActorFindWithTag(kGameTagConsole);
    return (console != NULL && console->menuItems != NULL) ? true : false;
}

// 選択されたメニューを取得する
//
int ConsoleGetSelectedMenu(void)
{
    struct Console *console = (struct Console *)ActorFindWithTag(kGameTagConsole);
    return console != NULL ? console->menuDone : -1;
}

// 数値入力を開始する
//
void ConsoleInputNumber(int number, int minimum, int maximum)
{
    // コンソールの取得
    struct Console *console = (struct Console *)ActorFindWithTag(kGameTagConsole);
    if (console == NULL) {
        return;
    }

    // 数値入力の設定
    console->numberDigit = 0;
    console->numberWidth = 0;
    console->numberMinimum = minimum;
    console->numberMaximum = maximum;
    console->numberInput = number;
    console->numberDone = -1;
    console->numberCrank = 0.0f;
    console->numberCrankInterval = 1.0f;
    console->numberUpdate = true;
    if (maximum < 10) {
        console->numberDigit = 1;
        console->numberWidth = IocsGetTextWidth(kIocsFontJapanese, "9");
    } else if (maximum < 100) {
        console->numberDigit = 2;
        console->numberWidth = IocsGetTextWidth(kIocsFontJapanese, "99");
    } else if (maximum < 1000) {
        console->numberDigit = 3;
        console->numberWidth = IocsGetTextWidth(kIocsFontJapanese, "999");
    } else {
        console->numberDigit = 4;
        console->numberWidth = IocsGetTextWidth(kIocsFontJapanese, "9999");
    }
    console->numberCrankInterval = (float)(360 / (maximum - minimum));
    if (console->numberCrankInterval < 1.0f) {
        console->numberCrankInterval = 1.0f;
    } else if (console->numberCrankInterval > 10.0f) {
        console->numberCrankInterval = 10.0f;
    }
}

// 数値入力がされているかどうかを判定する
//
bool ConsoleIsInputNumber(void)
{
    struct Console *console = (struct Console *)ActorFindWithTag(kGameTagConsole);
    return console != NULL && console->numberInput >= 0 ? true : false;
}

// 入力された数値を取得する
//
int ConsoleGetInputedNumber(void)
{
    struct Console *console = (struct Console *)ActorFindWithTag(kGameTagConsole);
    return console != NULL ? console->numberDone : -1;
}

// 角度入力を開始する
//
void ConsoleInputAngle(void)
{
    // コンソールの取得
    struct Console *console = (struct Console *)ActorFindWithTag(kGameTagConsole);
    if (console == NULL) {
        return;
    }

    // 角度入力の設定
    console->angleInput = IocsGetCrankAngle();
    console->angleDone = -1.0f;
    console->angleUpdate = true;
}

// 角度入力がされているかどうかを判定する
//
bool ConsoleIsInputAngle(void)
{
    struct Console *console = (struct Console *)ActorFindWithTag(kGameTagConsole);
    return console != NULL && console->angleInput >= 0.0f ? true : false;
}

// 入力された角度を取得する
//
int ConsoleGetInputedAngle(void)
{
    struct Console *console = (struct Console *)ActorFindWithTag(kGameTagConsole);
    return console != NULL ? (int)console->angleDone : -1;
}

