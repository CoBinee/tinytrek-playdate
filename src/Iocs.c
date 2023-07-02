// Iocs.c - 入出力制御システム
//

// 参照ファイル
//
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "pd_api.h"
#include "Iocs.h"


// 内部関数
//
static void IocsInitializeFont(void);
static void IocsInitializeScreen(void);
static void IocsInitializeButton(void);
static void IocsUpdateButton(void);
static void IocsPrintButton(int x, int y, PDButtons button);
static void IocsInitializeCrank(void);
static void IocsUpdateCrank(void);
static void IocsPrintCrank(int x, int y, float crank);
static void IocsInitializeAudio(void);

// 内部変数
//
static struct Iocs *iocs = NULL;
static const char *fontPaths[] = {
    "fonts/font-0", 
    "fonts/misaki_gothic", 
    "fonts/font-game", 
    "fonts/font-mini", 
};
static const char *audioPaths[] = {
    "sounds/null", 
    "sounds/pipo", 
    "sounds/pi", 
    "sounds/po", 
};


// 入出力制御システムを初期化する
//
void IocsInitialize(PlaydateAPI *playdate)
{
    // 入出力制御システムの作成
    iocs = playdate->system->realloc(NULL, sizeof (struct Iocs));
    if (iocs == NULL) {
        playdate->system->error("%s:%i: iocs instance is not created.", __FILE__, __LINE__);
        return;
    }

    // PlaydateAPI の設定
    iocs->playdate = playdate;

    // フレームレートの設定
    playdate->display->setRefreshRate(kIocsFrameRate);

    // フォントの初期化
    IocsInitializeFont();

    // 画面の初期化
    IocsInitializeScreen();

    // ボタンの初期化
    IocsInitializeButton();

    // クランクの初期化
    IocsInitializeCrank();

    // オーディオの初期化
    IocsInitializeAudio();

    // 乱数の初期化
    srand(playdate->system->getSecondsSinceEpoch(NULL));

}

// 入出力制御システムインスタンスを取得する
//
struct Iocs *IocsGetInstance(void)
{
    return iocs;
}

// Playdate インスタンスを取得する
//
PlaydateAPI *IocsGetPlaydate(void)
{
    return iocs != NULL ? iocs->playdate : NULL;
}

// イベントを処理する
//
void IocsEventHandler(PDSystemEvent event, uint32_t arg)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // 	kEventInitLua: Lua の初期化
    if (event == kEventInitLua) {
		playdate->system->logToConsole("%s: %d: kEventInitLua.\n", __FILE__, __LINE__);

    // kEventLock: ロック
    } else if (event == kEventLock) {
		playdate->system->logToConsole("%s: %d: kEventLock.\n", __FILE__, __LINE__);

    // kEventUnlock: アンロック
    } else if (event == kEventUnlock) {
		playdate->system->logToConsole("%s: %d: kEventUnlock.", __FILE__, __LINE__);

    // kEventPause: 一時停止
    } else if (event == kEventPause) {
		playdate->system->logToConsole("%s: %d: kEventPause.", __FILE__, __LINE__);

    // kEventResume: 一時停止からの復帰
    } else if (event == kEventResume) {
		playdate->system->logToConsole("%s: %d: kEventResume.", __FILE__, __LINE__);

    // kEventTerminate: 停止
    } else if (event == kEventTerminate) {
		playdate->system->logToConsole("%s: %d: kEventTerminate.", __FILE__, __LINE__);

    // kEventKeyPressed: キーが押される
    } else if (event == kEventKeyPressed) {
		playdate->system->logToConsole("%s: %d: kEventKeyPressed: %02x", __FILE__, __LINE__, arg);

    // kEventKeyReleased: キーが離される
    } else if (event == kEventKeyReleased) {
		playdate->system->logToConsole("%s: %d: kEventKeyReleased: %02x", __FILE__, __LINE__, arg);

    // kEventLowPower: 電力低下
    } else if (event == kEventLowPower) {
		playdate->system->logToConsole("%s: %d: kEventLowPowern.", __FILE__, __LINE__);

    }
}

// 入出力制御システムの更新を開始する
//
void IocsUpdateBegin(void)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // ボタンの更新
    IocsUpdateButton();

    // クランクの更新
    IocsUpdateCrank();
}

// 入出力制御システムの更新を終了する
//
void IocsUpdateEnd(void)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // デバッグ
    /*
    IocsPrintButton(  1, 1, iocs->buttonPush);
    IocsPrintButton( 97, 1, iocs->buttonEdge);
    IocsPrintButton(193, 1, iocs->buttonRepeat);
    */
    /*
    IocsPrintCrank( 1, 1, iocs->crankAngle);
    IocsPrintCrank(81, 1, iocs->crankChange);
    */
}

// フレームレートを取得する
//
int IocsGetFrameRate(void)
{
    return kIocsFrameRate;
}
int IocsGetFrameMillisecond(void)
{
    return kIocsFrameMillisecond;
}

// フォントを初期化する
//
static void IocsInitializeFont(void)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // フォントの読み込み
    const char *error;
    for (int i = 0; i < kIocsFontSize; i++) {
        iocs->fonts[i] = playdate->graphics->loadFont(fontPaths[i], &error);
        if (iocs->fonts[i] == NULL) {
            playdate->system->error("%s: %d: font is not loaded:  %s: %s", __FILE__, __LINE__, fontPaths[i], error);
            return;
        }
    }
}

// フォントを設定する
//
void IocsSetFont(IocsFont font)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // フォントの設定
    playdate->graphics->setFont(iocs->fonts[font]);
}

// フォントを高さを取得する
//
int IocsGetFontHeight(IocsFont font)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return 0;
    }

    // フォントの高さの取得
    return playdate->graphics->getFontHeight(iocs->fonts[font]);
}

// テキストの幅を取得する
//
int IocsGetTextWidth(IocsFont font, const char *text)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return 0;
    }

    // テキストの長さの取得
    return playdate->graphics->getTextWidth(iocs->fonts[font], text, strlen(text), kUTF8Encoding, 0);
}


// 画面を初期化する
//
static void IocsInitializeScreen(void)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // 画面の設定
    playdate->display->setInverted(false);
    playdate->display->setScale(1);
    playdate->display->setOffset(0, 0);
    playdate->graphics->setBackgroundColor(kColorWhite);

    // 画面の初期化
    iocs->screenColor = kColorWhite;
}

// 画面の色を設定する
//
void IocsSetScreenColor(LCDColor color)
{
    // スクリーンの初期化
    iocs->screenColor = color;
}

// 画面をクリアする
//
void IocsClearScreen(void)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // 画面のクリア
    playdate->graphics->setDrawMode(kDrawModeCopy);
    playdate->graphics->setDrawOffset(0, 0);
    playdate->graphics->clearClipRect();
    if (iocs->screenColor != kColorClear) {
        playdate->graphics->clear(iocs->screenColor);
    }
}

// ボタンを初期化する
//
static void IocsInitializeButton(void)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // ボタンの初期化
    iocs->buttonPush = 0;
    iocs->buttonEdge = 0;
    iocs->buttonRepeat = 0;
    for (int i = 0; i < kIocsButtonSize; i++) {
        iocs->buttonRepeatCounts[i] = 0;
    }
    iocs->buttonRepeatCountDelay = 15;
    iocs->buttonRepeatCountInterval = 1;
    
}

// ボタンを更新する
//
static void IocsUpdateButton(void)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // ボタンの取得
    PDButtons current, pushed;
    playdate->system->getButtonState(&current, &pushed, NULL);
    iocs->buttonPush = current;
    iocs->buttonEdge = pushed;
    iocs->buttonRepeat = 0;

    // リピートの更新
    for (int i = 0; i < kIocsButtonSize; i++) {
        if ((iocs->buttonPush & (1 << i)) != 0) {
            ++iocs->buttonRepeatCounts[i];
            if (iocs->buttonRepeatCounts[i] > iocs->buttonRepeatCountDelay + iocs->buttonRepeatCountInterval) {
                iocs->buttonRepeatCounts[i] = iocs->buttonRepeatCountDelay;
            }
            if (iocs->buttonRepeatCounts[i] == 1 || iocs->buttonRepeatCounts[i] == iocs->buttonRepeatCountDelay) {
                iocs->buttonRepeat |= (1 << i);
            }
        } else {
            iocs->buttonRepeatCounts[i] = 0;
        }
    }
}

// ボタンを取得する
//
bool IocsIsButtonPush(PDButtons button)
{
    return (iocs->buttonPush & button) != 0 ? true : false;
}
bool IocsIsButtonEdge(PDButtons button)
{
    return (iocs->buttonEdge & button) != 0 ? true : false;
}
bool IocsIsButtonRepeat(PDButtons button)
{
    return (iocs->buttonRepeat & button) != 0 ? true : false;
}

// ボタンの状態を表示する
//
static void IocsPrintButton(int x, int y, PDButtons button)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // ボタンの表示
    char text[7] = "      ";
    if ((button & kButtonLeft) != 0) {
        text[0] = 'L';
    }
    if ((button & kButtonRight) != 0) {
        text[1] = 'R';
    }
    if ((button & kButtonUp) != 0) {
        text[2] = 'U';
    }
    if ((button & kButtonDown) != 0) {
        text[3] = 'D';
    }
    if ((button & kButtonB) != 0) {
        text[4] = 'B';
    }
    if ((button & kButtonA) != 0) {
        text[5] = 'A';
    }
    playdate->graphics->setFont(iocs->fonts[kIocsFontSystem]);
    playdate->graphics->setDrawMode(kDrawModeXOR);
    playdate->graphics->drawText(text, strlen(text), kUTF8Encoding, x, y);
}

// クランクを初期化する
//
static void IocsInitializeCrank(void)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // クランクの初期化
    iocs->crankAngle = playdate->system->getCrankAngle();
    iocs->crankChange = playdate->system->getCrankChange();    
}

// クランクを更新する
//
static void IocsUpdateCrank(void)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // クランクの取得
    iocs->crankAngle = playdate->system->getCrankAngle();
    iocs->crankChange = playdate->system->getCrankChange();    
}

// クランクを取得する
//
float IocsGetCrankAngle(void)
{
    return iocs->crankAngle;
}
float IocsGetCrankChange(void)
{
    return iocs->crankChange;
}

//　クランクの状態を表示する
//
static void IocsPrintCrank(int x, int y, float crank)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // ボタンの表示
    {
        char *text;
        playdate->system->formatString(&text, "% 3d", (int)crank);
        playdate->graphics->setFont(iocs->fonts[kIocsFontSystem]);
        playdate->graphics->setDrawMode(kDrawModeXOR);
        playdate->graphics->drawText(text, strlen(text), kUTF8Encoding, x, y);
        playdate->system->realloc(text, 0);
    }
}

// オーディオを初期化する
//
static void IocsInitializeAudio(void)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // システムオーディオの作成
    {
        for (int i = 0; i < kIocsAudioSystemSampleSize; i++) {
            iocs->audioSystemSamples[i] = playdate->sound->sample->load(audioPaths[i]);
            if (iocs->audioSystemSamples[i] == NULL) {
                playdate->system->error("%s: %d: system audio sample is not loaded: %s", __FILE__, __LINE__, audioPaths[i]);
                return;
            }
            {
                uint8_t *data;
                SoundFormat format;
                uint32_t samplerate;
                uint32_t bytelength;
                playdate->sound->sample->getData(iocs->audioSystemSamples[i], &data, &format, &samplerate, &bytelength);
                iocs->audioSystemFrames[i] = bytelength / SoundFormat_bytesPerFrame(format);
                playdate->system->logToConsole(
                    "%s: %d: %s: %d, %d, %d, %d, %f", 
                    __FILE__, 
                    __LINE__, 
                    audioPaths[i], 
                    format, 
                    samplerate, 
                    bytelength, 
                    iocs->audioSystemFrames[i], 
                    (double)playdate->sound->sample->getLength(iocs->audioSystemSamples[i])
                );
            }
        }
        iocs->audioSystemPlayer = playdate->sound->sampleplayer->newPlayer();
        if (iocs->audioSystemPlayer == NULL) {
            playdate->system->error("%s: %d: system sample player is not created.", __FILE__, __LINE__);
            return;
        }
    }

    // エフェクトオーディオの作成
    {
        for (int i = 0; i < kIocsAudioEffectSampleSize; i++) {
            iocs->audioEffectSamples[i] = NULL;
        }
        for (int i = 0; i < kIocsAudioEffectPlayerSize; i++) {
            iocs->audioEffectPlayers[i] = playdate->sound->sampleplayer->newPlayer();
            if (iocs->audioEffectPlayers[i] == NULL) {
                playdate->system->error("%s: %d: effect sample player is not created.", __FILE__, __LINE__);
                return;
            }
        }
    }

    // ミュージックオーディオの作成
    {
        iocs->audioMusicPlayer = playdate->sound->fileplayer->newPlayer();
        if (iocs->audioMusicPlayer == NULL) {
            playdate->system->error("%s: %d: music file player is not created.", __FILE__, __LINE__);
            return;
        }
    }
}

// システムオーディオを再生する
//
void IocsPlayAudioSystem(IocsAudioSystemSample sample, int repeat)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // オーディオの再生
    if (iocs->audioSystemPlayer != NULL) {
        playdate->sound->sampleplayer->setSample(iocs->audioSystemPlayer, iocs->audioSystemSamples[sample]);
        playdate->sound->sampleplayer->setVolume(iocs->audioSystemPlayer, 1.0f, 1.0f);
        // playdate->sound->sampleplayer->setPlayRange(iocs->audioSystemPlayer, 0, iocs->audioSystemFrames[sample]);
        playdate->sound->sampleplayer->play(iocs->audioSystemPlayer, repeat, 1.0f);
    }
}

// システムオーディオを停止する
//
void IocsStopAudioSystem(void)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // オーディオの再生
    if (iocs->audioSystemPlayer != NULL) {
        if (playdate->sound->sampleplayer->isPlaying(iocs->audioSystemPlayer) != 0) {
            playdate->sound->sampleplayer->stop(iocs->audioSystemPlayer);
        }
    }
}

// エフェクトオーディオを読み込む
//
void IocsLoadAudioEffects(const char *paths[], int size)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // オーディオの読み込み
    if (size > kIocsAudioEffectSampleSize) {
        playdate->system->error("%s: %d: effect audio entry is over.", __FILE__, __LINE__);
        return;
    }
    for (int i = 0; i < size; i++) {
        iocs->audioEffectSamples[i] = playdate->sound->sample->load(paths[i]);
        if (iocs->audioEffectSamples[i] == NULL) {
            playdate->system->error("%s: %d: effect audio sample is not loaded: %s", __FILE__, __LINE__, paths[i]);
            return;
        }
        {
            uint8_t *data;
            SoundFormat format;
            uint32_t samplerate;
            uint32_t bytelength;
            playdate->sound->sample->getData(iocs->audioEffectSamples[i], &data, &format, &samplerate, &bytelength);
            iocs->audioEffectFrames[i] = bytelength / SoundFormat_bytesPerFrame(format);
            playdate->system->logToConsole(
                "%s: %d: %s: %d, %d, %d, %d, %f", 
                __FILE__, 
                __LINE__, 
                paths[i], 
                format, 
                samplerate, 
                bytelength, 
                iocs->audioEffectFrames[i], 
                (double)playdate->sound->sample->getLength(iocs->audioEffectSamples[i])
            );
        }
    }
}

// エフェクトオーディオを解放する
//
void IocsUnloadAllAudioEffects(void)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // オーディオの解放
    for (int i = 0; i < kIocsAudioEffectSampleSize; i++) {
        if (iocs->audioEffectSamples[i] != NULL) {
            playdate->sound->sample->freeSample(iocs->audioEffectSamples[i]);
            iocs->audioEffectSamples[i] = NULL;
        }
    }
}

// エフェクトオーディオを再生する
//
int IocsPlayAudioEffect(int sample, int repeat)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return -1;
    }

    // オーディオの再生
    int player = 0;
    while (player < kIocsAudioEffectPlayerSize) {
        if (playdate->sound->sampleplayer->isPlaying(iocs->audioEffectPlayers[player]) == 0) {
            playdate->sound->sampleplayer->setSample(iocs->audioEffectPlayers[player], iocs->audioEffectSamples[sample]);
            playdate->sound->sampleplayer->setVolume(iocs->audioEffectPlayers[player], 1.0f, 1.0f);
            // playdate->sound->sampleplayer->setPlayRange(iocs->audioEffectPlayers[player], 0, iocs->audioEffectFrames[sample]);
            playdate->sound->sampleplayer->play(iocs->audioEffectPlayers[player], repeat, 1.0f);
            break;
        }
        ++player;
    }
    return player < kIocsAudioEffectPlayerSize ? player : -1;
}

// エフェクトオーディオを停止する
//
void IocsStopAudioEffect(int player)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // オーディオの停止
    if (0 <= player && player < kIocsAudioEffectPlayerSize) {
        if (playdate->sound->sampleplayer->isPlaying(iocs->audioEffectPlayers[player]) != 0) {
            playdate->sound->sampleplayer->stop(iocs->audioEffectPlayers[player]);
        }
    }
}
void IocsStopAllAudioEffects(void)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // オーディオの停止
    for (int i = 0; i < kIocsAudioEffectPlayerSize; i++) {
        if (playdate->sound->sampleplayer->isPlaying(iocs->audioEffectPlayers[i]) != 0) {
            playdate->sound->sampleplayer->stop(iocs->audioEffectPlayers[i]);
        }
    }
}

// ミュージックオーディオを再生する
//
void IocsPlayMusicAudio(const char *path, int repeat)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // オーディオの再生
    playdate->sound->fileplayer->loadIntoPlayer(iocs->audioMusicPlayer, path);
    playdate->sound->fileplayer->setVolume(iocs->audioMusicPlayer, 1.0f, 1.0f);
    playdate->sound->fileplayer->play(iocs->audioMusicPlayer, repeat);
}

// ミュージックオーディオを停止する
//
void IocsStopMusicAudio(void)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // オーディオの再生
    playdate->sound->fileplayer->stop(iocs->audioMusicPlayer);
}

// UTF-8 の一文字のバイト数を取得する
//
int IocsGetCharByte(char c)
{
    int byte = 0;
    if ((c & 0x80) == 0x00) {
        byte = 1;
    } else if ((c & 0xe0) == 0xc0) {
        byte = 2;
    } else if ((c & 0xf0) == 0xe0) {
        byte = 3;
    } else if ((c & 0xf8) == 0xf0) {
        byte = 2;
    } else if ((c & 0xfc) == 0xf8) {
        byte = 2;
    } else {
        byte = 6;
    }
    return byte;
}

// UTF-8 の文字列長を取得する
//
int IocsGetTextLength(const char *text)
{
    int length = 0;
    while (*text != '\0') {
        text += IocsGetCharByte(*text);
        ++length;
    }
    return length;
}

// 乱数を取得する
//
int IocsGetRandom(void)
{
    return rand();
}
