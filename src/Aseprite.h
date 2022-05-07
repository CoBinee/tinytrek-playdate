// Aseprite.h - Aseprite ライブラリ
//
#pragma once

// 参照ファイル
// 
#include <stdbool.h>
#include "pd_api.h"


// .json ファイル
//
struct AsepriteJson {
    uint8_t *base;
    int ptr;
    int size;
};

// スプライト
//
typedef enum {
    kAsepriteSpriteJsonSublistNull = 0, 
    kAsepriteSpriteJsonSublistFrames, 
    kAsepriteSpriteJsonSublistFramesFrame, 
    kAsepriteSpriteJsonSublistFramesSpriteSourceSize, 
    kAsepriteSpriteJsonSublistFramesSourceSize, 
    kAsepriteSpriteJsonSublistTags, 
} AsepriteSpriteJsonSublist;
struct AsepriteSpriteSize {
    int w;
    int h;
};
struct AsepriteSpriteRect {
    int x;
    int y;
    int w;
    int h;
};
struct AsepriteSpriteFrame {
    struct AsepriteSpriteRect frame;
    struct AsepriteSpriteRect spriteSourceSize;
    struct AsepriteSpriteSize sourceSize;
    int duration;
};
enum {
    kAsepriteSpriteTagNameSize = 16, 
};
struct AsepriteSpriteTag {
    char name[kAsepriteSpriteTagNameSize];
    int from;
    int to;
};
enum {
    kAsepriteSpriteNameSize = 32, 
};
struct AsepriteSprite {

    // 名前
    char name[kAsepriteSpriteNameSize];

    // .json
    struct AsepriteJson json;

    // "frames"
    struct AsepriteSpriteFrame *frames;
    int frameIndex;
    int frameSize;

    // "frameTags"
    struct AsepriteSpriteTag *tags;
    int tagIndex;
    int tagSize;

    // サブリスト
    AsepriteSpriteJsonSublist sublist;

    // パス
    int pass;

    // バグ対応
    int indexLast;

    // ビットマップ
    LCDBitmap **bitmaps;
};

// スプライトアニメーション
//
struct AsepriteSpriteAnimation {
    struct AsepriteSprite *sprite;
    int play;
    int from;
    int to;
    int millisecond;
    bool loop;
};

// Aseprite コントローラ
//
enum {
    kAsepritePathSize = 32, 
};
enum {
    kAsepriteSpriteEntry = 32, 
};
struct AsepriteController {

    // スプライト
    struct AsepriteSprite sprites[kAsepriteSpriteEntry];
    char spritePath[kAsepritePathSize];

};

// 外部関数
//
extern void AsepriteInitialize(const char *spritePath);
extern void AsepriteLoadSprite(const char *spriteName);
extern void AsepriteLoadSpriteList(const char *spriteNames[], int entry);
extern void AsepriteUnloadSprite(const char *spriteName);
extern void AsepriteUnloadAllSprites(void);
extern void AsepriteStartSpriteAnimation(struct AsepriteSpriteAnimation *animation, const char *spriteName, const char *animationName, bool loop);
extern void AsepriteUpdateSpriteAnimation(struct AsepriteSpriteAnimation *animation);
extern bool AsepriteIsSpriteAnimationDone(struct AsepriteSpriteAnimation *animation);
extern void AsepriteDrawSpriteAnimation(struct AsepriteSpriteAnimation *animation, int x, int y, LCDBitmapDrawMode mode, LCDBitmapFlip flip);
extern void AsepriteDrawRotatedSpriteAnimation(struct AsepriteSpriteAnimation *animation, int x, int y, float degrees, float centerx, float centery, float xscale, float yscale, LCDBitmapDrawMode mode);

