// Aseprite.c - Aseprite ライブラリ
//

// 参照ファイル
//
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include "pd_api.h"
#include "Iocs.h"
#include "Aseprite.h"


// 内部関数
//
static void AsepriteFreeSprite(struct AsepriteSprite *sprite);
static struct AsepriteSprite *AsepriteFindSprite(const char *name);
static void AsepriteSpriteJsonDecodeError(struct json_decoder *decoder, const char *error, int linenum);
static void AsepriteSpriteJsonWillDecodeSublist(struct json_decoder *decoder, const char *name, json_value_type type);
static int AsepriteSpriteJsonShouldDecodeTableValueForKey(struct json_decoder *decoder, const char *key);
static void AsepriteSpriteJsonDidDecodeTableValue(struct json_decoder *decoder, const char *key, json_value value);
static int AsepriteSpriteJsonShouldDecodeArrayValueAtIndex(struct json_decoder *decoder, int pos);
static void AsepriteSpriteJsonDidDecodeArrayValue(struct json_decoder *decoder, int pos, json_value value);
static void *AsepriteSpriteJsonDidDecodeSublist(struct json_decoder *decoder, const char *name, json_value_type type);
static const char *AsepriteGetJsonValueName(json_value value);
static const char *AsepriteGetJsonTypeName(json_value_type type);
static bool AsepriteLoadJson(struct AsepriteJson *json, const char *path);
static void AsepriteUnloadJson(struct AsepriteJson *json);
static int AsepriteReadJson(void *userdata, uint8_t *buffer, int size);

// 内部変数
//
static struct AsepriteController *asepriteController = NULL;
static const char *asepriteSpriteJsonSublistNameFrames = "frames[";
static const char *asepriteSpriteJsonSublistNameFramesFrame = "frame";
static const char *asepriteSpriteJsonSublistNameFramesSpriteSourceSize = "spriteSourceSize";
static const char *asepriteSpriteJsonSublistNameFramesSourceSize = "sourceSize";
static const char *asepriteSpriteJsonSublistNameTags = "frameTags[";


// Aseprite を初期化する
//
void AsepriteInitialize(const char *spritePath)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // Aseprite コントローラの作成
    asepriteController = playdate->system->realloc(NULL, sizeof (struct AsepriteController));
    if (asepriteController == NULL) {
        playdate->system->error("%s: %d: aseprite controller instance is not created.", __FILE__, __LINE__);
        return;
    }
    memset(asepriteController, 0, sizeof (struct AsepriteController));

    // スプライトの初期化
    strcpy(asepriteController->spritePath, spritePath);
}

// スプライトを読み込む
//
void AsepriteLoadSprite(const char *spriteName)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // スプライトの登録
    struct AsepriteSprite *sprite = NULL;
    for (int i = 0; i < kAsepriteSpriteEntry; i++) {
        if (asepriteController->sprites[i].name[0] == '\0') {
            sprite = &asepriteController->sprites[i];
            break;
        }
    }
    if (sprite == NULL) {
        playdate->system->error("%s: %d: sprite is not entry.", __FILE__, __LINE__);
        return;
    }
    memset(sprite, 0, sizeof (struct AsepriteSprite));

    // .json の読み込み
    {
        // パスの取得
        char path[kAsepritePathSize];
        strcpy(path, asepriteController->spritePath);
        strcat(path, spriteName);
        strcat(path, ".json");

        // .json の読み込み
        if (!AsepriteLoadJson(&sprite->json, path)) {
            playdate->system->error("%s: %d: json is not loaded: %s", __FILE__, __LINE__, path);
            return;
        }

        // デコーダの定義
        struct json_decoder decoder = {
            .decodeError = AsepriteSpriteJsonDecodeError,
            .willDecodeSublist = AsepriteSpriteJsonWillDecodeSublist,
            .shouldDecodeTableValueForKey = AsepriteSpriteJsonShouldDecodeTableValueForKey,
            .didDecodeTableValue = AsepriteSpriteJsonDidDecodeTableValue,
            .shouldDecodeArrayValueAtIndex = AsepriteSpriteJsonShouldDecodeArrayValueAtIndex,
            .didDecodeArrayValue = AsepriteSpriteJsonDidDecodeArrayValue,
            .didDecodeSublist = AsepriteSpriteJsonDidDecodeSublist, 
            .userdata = sprite, 
        };

        // リーダの定義
        json_reader reader = {
            .read = (int (*)(void *, uint8_t *, int))AsepriteReadJson, 
            .userdata = &sprite->json, 
        };

        // .json のデコード: 1 pass
        {
            json_value value;
            sprite->json.ptr = 0;
            sprite->pass = 1;
            sprite->sublist = kAsepriteSpriteJsonSublistNull;
            playdate->json->decode(&decoder, reader, &value);
        }

        // フレームの作成
        if (sprite->frameSize > 0) {
            sprite->frames = playdate->system->realloc(NULL, sprite->frameSize * sizeof (struct AsepriteSpriteFrame));
        }

        // タグの作成
        if (sprite->tagSize > 0) {
            sprite->tags = playdate->system->realloc(NULL, sprite->tagSize * sizeof (struct AsepriteSpriteTag));
        }

        // .json のデコード: 2 pass
        {
            json_value value;
            sprite->json.ptr = 0;
            sprite->pass = 2;
            sprite->sublist = kAsepriteSpriteJsonSublistNull;
            playdate->json->decode(&decoder, reader, &value);
        }
    }

    // ビットマップの読み込み
    {
        // パスの取得
        char path[kAsepritePathSize];
        strcpy(path, asepriteController->spritePath);
        strcat(path, spriteName);
        strcat(path, ".png");

        // ビットマップ配列の作成
        sprite->bitmaps = playdate->system->realloc(NULL, sprite->frameSize * sizeof (struct AsepriteSpriteBitmap));
        if (sprite->bitmaps == NULL) {
            playdate->system->error("%s: %d: bitmap array is not allocated.", __FILE__, __LINE__);
            return;
        }

        // ビットマップの作成
        sprite->bitmapSize = 0;
        for (int i = 0; i < sprite->frameSize; i++) {
            int j = 0;
            while (j < sprite->bitmapSize) {
                if (
                    sprite->bitmaps[j].frame.x == sprite->frames[i].frame.x && 
                    sprite->bitmaps[j].frame.y == sprite->frames[i].frame.y && 
                    sprite->bitmaps[j].frame.w == sprite->frames[i].frame.w && 
                    sprite->bitmaps[j].frame.h == sprite->frames[i].frame.h
                ) {
                    break;
                }
                ++j;
            }
            if (j >= sprite->bitmapSize) {
                sprite->bitmaps[j].frame = sprite->frames[i].frame;
                sprite->bitmaps[j].bitmap = playdate->graphics->newBitmap(sprite->bitmaps[j].frame.w, sprite->bitmaps[j].frame.h, kColorClear);
                if (sprite->bitmaps[j].bitmap == NULL) {
                    playdate->system->error("%s: %d: bitmap is not created.", __FILE__, __LINE__);
                    return;
                }
                ++sprite->bitmapSize;
            }
            sprite->frames[i].bitmap = j;
        }

        // ビットマップの読み込み
        LCDBitmap *bitmap = NULL;
        const char *error;
        bitmap = playdate->graphics->loadBitmap(path, &error);
        if (bitmap == NULL) {
            playdate->system->error("%s: %d: bitmap is not loaded: %s: %s", __FILE__, __LINE__, path, error);
            return;
        }

        // ビットマップの複写
        for (int i = 0; i < sprite->bitmapSize; i++) {
            playdate->graphics->pushContext(sprite->bitmaps[i].bitmap);
            playdate->graphics->setDrawMode(kDrawModeCopy);
            playdate->graphics->drawBitmap(bitmap, -sprite->bitmaps[i].frame.x, -sprite->bitmaps[i].frame.y, kBitmapUnflipped);
            playdate->graphics->popContext();
        }

        // ビットマップの解放
        playdate->graphics->freeBitmap(bitmap);
    }

    // 名前の設定
    strcpy(sprite->name, spriteName);
}
void AsepriteLoadSpriteList(const char *spriteNames[], int entry)
{
    for (int i = 0; i < entry; i++) {
        AsepriteLoadSprite(spriteNames[i]);
    }
}

// スプライトを解放する
//
static void AsepriteFreeSprite(struct AsepriteSprite *sprite)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // スプライトの解放
    if (sprite != NULL && sprite->name[0] != '\0') {

        // .json の解放
        AsepriteUnloadJson(&sprite->json);

        // frames の解放
        if (sprite->frames != NULL) {
            playdate->system->realloc(sprite->frames, 0);
        }

        // frameTags の解放
        if (sprite->tags != NULL) {
            playdate->system->realloc(sprite->tags, 0);
        }

        // ビットマップの解放
        if (sprite->bitmaps != NULL) {
            for (int i = 0; i < sprite->bitmapSize; i++) {
                if (sprite->bitmaps[i].bitmap != NULL) {
                    playdate->graphics->freeBitmap(sprite->bitmaps[i].bitmap);
                }
            }
            playdate->system->realloc(sprite->bitmaps, 0);
        }

        // スプライトの登録の解除
        sprite->name[0] = '\0';
    }
}
void AsepriteUnloadSprite(const char *spriteName)
{
    AsepriteFreeSprite(AsepriteFindSprite(spriteName));
}
void AsepriteUnloadAllSprites(void)
{
    for (int i = 0; i < kAsepriteSpriteEntry; i++) {
        AsepriteFreeSprite(&asepriteController->sprites[i]);
    }
}

// スプライトを取得する
//
static struct AsepriteSprite *AsepriteFindSprite(const char *name)
{
    struct AsepriteSprite *sprite = NULL;
    for (int i = 0; i < kAsepriteSpriteEntry; i++) {
        if (strcmp(asepriteController->sprites[i].name, name) == 0) {
            sprite = &asepriteController->sprites[i];
            break;
        }
    }
    return sprite;
}

// スプライトの .json を読み込む
//
void AsepriteLoadSpriteJson(struct AsepriteSprite *sprite, const char *path)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // スプライトの初期化
    memset(sprite, 0, sizeof (struct AsepriteSprite));

    // .json の読み込み
    {
        // .json の読み込み
        if (!AsepriteLoadJson(&sprite->json, path)) {
            playdate->system->error("%s: %d: json is not loaded: %s", __FILE__, __LINE__, path);
            return;
        }

        // デコーダの定義
        struct json_decoder decoder = {
            .decodeError = AsepriteSpriteJsonDecodeError,
            .willDecodeSublist = AsepriteSpriteJsonWillDecodeSublist,
            .shouldDecodeTableValueForKey = AsepriteSpriteJsonShouldDecodeTableValueForKey,
            .didDecodeTableValue = AsepriteSpriteJsonDidDecodeTableValue,
            .shouldDecodeArrayValueAtIndex = AsepriteSpriteJsonShouldDecodeArrayValueAtIndex,
            .didDecodeArrayValue = AsepriteSpriteJsonDidDecodeArrayValue,
            .didDecodeSublist = AsepriteSpriteJsonDidDecodeSublist, 
            .userdata = sprite, 
        };

        // リーダの定義
        json_reader reader = {
            .read = (int (*)(void *, uint8_t *, int))AsepriteReadJson, 
            .userdata = &sprite->json, 
        };

        // .json のデコード: 1 pass
        {
            json_value value;
            sprite->json.ptr = 0;
            sprite->pass = 1;
            sprite->sublist = kAsepriteSpriteJsonSublistNull;
            playdate->json->decode(&decoder, reader, &value);
        }

        // フレームの作成
        if (sprite->frameSize > 0) {
            sprite->frames = playdate->system->realloc(NULL, sprite->frameSize * sizeof (struct AsepriteSpriteFrame));
        }

        // タグの作成
        if (sprite->tagSize > 0) {
            sprite->tags = playdate->system->realloc(NULL, sprite->tagSize * sizeof (struct AsepriteSpriteTag));
        }

        // .json のデコード: 2 pass
        {
            json_value value;
            sprite->json.ptr = 0;
            sprite->pass = 2;
            sprite->sublist = kAsepriteSpriteJsonSublistNull;
            playdate->json->decode(&decoder, reader, &value);
        }
    }
}

// スプライトの .json を解放する
//
void AsepriteUnloadSpriteJson(struct AsepriteSprite *sprite)
{
    AsepriteFreeSprite(sprite);
}

// スプライトの .json をデコードする
//
static void AsepriteSpriteJsonDecodeError(struct json_decoder *decoder, const char *error, int linenum)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // エラー
    playdate->system->error("%s: %d: .json decode error: %d: %s", __FILE__, __LINE__, linenum, error);
}
static void AsepriteSpriteJsonWillDecodeSublist(struct json_decoder *decoder, const char *name, json_value_type type)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // スプライトの取得
    struct AsepriteSprite *sprite = (struct AsepriteSprite *)decoder->userdata;

    // 1 pass
    if (sprite->pass == 1) {
        if (strncmp(name, asepriteSpriteJsonSublistNameFrames, strlen(asepriteSpriteJsonSublistNameFrames)) == 0) {
            sprite->frameSize = sprite->indexLast; // バグ対応 atoi(&name[7]);
        } else if (strncmp(name, asepriteSpriteJsonSublistNameTags, strlen(asepriteSpriteJsonSublistNameTags)) == 0) {
            sprite->tagSize = sprite->indexLast; // バグ対応 atoi(&name[10]);
        }

    // 2 pass
    } else if (sprite->pass == 2) {
        if (sprite->sublist == kAsepriteSpriteJsonSublistNull) {
            if (strncmp(name, asepriteSpriteJsonSublistNameFrames, strlen(asepriteSpriteJsonSublistNameFrames)) == 0) {
                sprite->sublist = kAsepriteSpriteJsonSublistFrames;
                sprite->frameIndex = sprite->indexLast - 1; // バグ対応 atoi(&name[7]);
            } else if (strncmp(name, asepriteSpriteJsonSublistNameTags, strlen(asepriteSpriteJsonSublistNameTags)) == 0) {
                sprite->sublist = kAsepriteSpriteJsonSublistTags;
                sprite->tagIndex = sprite->indexLast - 1; // バグ対応 atoi(&name[10]);
            }
        } else if (sprite->sublist == kAsepriteSpriteJsonSublistFrames) {
            if (strcmp(name, asepriteSpriteJsonSublistNameFramesFrame) == 0) {
                sprite->sublist = kAsepriteSpriteJsonSublistFramesFrame;
            } else if (strcmp(name, asepriteSpriteJsonSublistNameFramesSpriteSourceSize) == 0) {
                sprite->sublist = kAsepriteSpriteJsonSublistFramesSpriteSourceSize;
            } else if (strcmp(name, asepriteSpriteJsonSublistNameFramesSourceSize) == 0) {
                sprite->sublist = kAsepriteSpriteJsonSublistFramesSourceSize;
            }
        }
    }

    // ログ
    // playdate->system->logToConsole("AsepriteSpriteJsonWillDecodeSublist: %s: %s", name, AsepriteGetJsonTypeName(type));
}
static int AsepriteSpriteJsonShouldDecodeTableValueForKey(struct json_decoder *decoder, const char *key)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return 0;
    }

    // スプライトの取得
    struct AsepriteSprite *sprite = (struct AsepriteSprite *)decoder->userdata;

    // ログ
    // playdate->system->logToConsole("AsepriteSpriteJsonShouldDecodeTableValueForKey: %s", key);

    // 終了
    return 1;
}
static void AsepriteSpriteJsonDidDecodeTableValue(struct json_decoder *decoder, const char *key, json_value value)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // スプライトの取得
    struct AsepriteSprite *sprite = (struct AsepriteSprite *)decoder->userdata;

    // 1 pass
    if (sprite->pass == 1) {

    // 2 pass
    } else if (sprite->pass == 2) {

        // frames
        if (sprite->sublist == kAsepriteSpriteJsonSublistFrames) {
            if (strcmp(key, "duration") == 0) {
                sprite->frames[sprite->frameIndex].duration = value.data.intval;
            }
        } else if (sprite->sublist == kAsepriteSpriteJsonSublistFramesFrame) {
            if (strcmp(key, "x") == 0) {
                sprite->frames[sprite->frameIndex].frame.x = value.data.intval;
            } else if (strcmp(key, "y") == 0) {
                sprite->frames[sprite->frameIndex].frame.y = value.data.intval;
            } else if (strcmp(key, "w") == 0) {
                sprite->frames[sprite->frameIndex].frame.w = value.data.intval;
            } else if (strcmp(key, "h") == 0) {
                sprite->frames[sprite->frameIndex].frame.h = value.data.intval;
            }
        } else if (sprite->sublist == kAsepriteSpriteJsonSublistFramesSpriteSourceSize) {
            if (strcmp(key, "x") == 0) {
                sprite->frames[sprite->frameIndex].spriteSourceSize.x = value.data.intval;
            } else if (strcmp(key, "y") == 0) {
                sprite->frames[sprite->frameIndex].spriteSourceSize.y = value.data.intval;
            } else if (strcmp(key, "w") == 0) {
                sprite->frames[sprite->frameIndex].spriteSourceSize.w = value.data.intval;
            } else if (strcmp(key, "h") == 0) {
                sprite->frames[sprite->frameIndex].spriteSourceSize.h = value.data.intval;
            }
        } else if (sprite->sublist == kAsepriteSpriteJsonSublistFramesSourceSize) {
            if (strcmp(key, "w") == 0) {
                sprite->frames[sprite->frameIndex].sourceSize.w = value.data.intval;
            } else if (strcmp(key, "h") == 0) {
                sprite->frames[sprite->frameIndex].sourceSize.h = value.data.intval;
            }

        // frameTags
        } else if (sprite->sublist == kAsepriteSpriteJsonSublistTags) {
            if (strcmp(key, "name") == 0) {
                strcpy(sprite->tags[sprite->tagIndex].name, value.data.stringval);
            } else if (strcmp(key, "from") == 0) {
                sprite->tags[sprite->tagIndex].from = value.data.intval;
            } else if (strcmp(key, "to") == 0) {
                sprite->tags[sprite->tagIndex].to = value.data.intval;
            }
        }
    }

    // ログ
    // playdate->system->logToConsole("AsepriteSpriteJsonDidDecodeTableValue: %s: %s", key, AsepriteGetJsonValueName(value));
}
static int AsepriteSpriteJsonShouldDecodeArrayValueAtIndex(struct json_decoder *decoder, int pos)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return 0;
    }

    // スプライトの取得
    struct AsepriteSprite *sprite = (struct AsepriteSprite *)decoder->userdata;

    // バグ対応として pos を保存する
    sprite->indexLast = pos;

    // ログ
    // playdate->system->logToConsole("AsepriteSpriteJsonShouldDecodeArrayValueAtIndex: %d", pos);

    // 終了
    return 1;
}
static void AsepriteSpriteJsonDidDecodeArrayValue(struct json_decoder *decoder, int pos, json_value value)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // スプライトの取得
    struct AsepriteSprite *sprite = (struct AsepriteSprite *)decoder->userdata;

    // ログ
    // playdate->system->logToConsole("AsepriteSpriteJsonDidDecodeArrayValue: %d: %s", pos, AsepriteGetJsonValueName(value));
}
static void *AsepriteSpriteJsonDidDecodeSublist(struct json_decoder *decoder, const char *name, json_value_type type)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return NULL;
    }

    // スプライトの取得
    struct AsepriteSprite *sprite = (struct AsepriteSprite *)decoder->userdata;

    // 1 pass
    if (sprite->pass == 1) {

    // 2 pass
    } else if (sprite->pass == 2) {
        if (sprite->sublist == kAsepriteSpriteJsonSublistFrames) {
            if (strncmp(name, asepriteSpriteJsonSublistNameFrames, strlen(asepriteSpriteJsonSublistNameFrames)) == 0) {
                sprite->sublist = kAsepriteSpriteJsonSublistNull;
            }
        } else if (sprite->sublist == kAsepriteSpriteJsonSublistTags) {
            if (strncmp(name, asepriteSpriteJsonSublistNameTags, strlen(asepriteSpriteJsonSublistNameTags)) == 0) {
                sprite->sublist = kAsepriteSpriteJsonSublistNull;
            }
        } else if (sprite->sublist == kAsepriteSpriteJsonSublistFramesFrame) {
            if (strcmp(name, asepriteSpriteJsonSublistNameFramesFrame) == 0) {
                sprite->sublist = kAsepriteSpriteJsonSublistFrames;
            }
        } else if (sprite->sublist == kAsepriteSpriteJsonSublistFramesSpriteSourceSize) {
            if (strcmp(name, asepriteSpriteJsonSublistNameFramesSpriteSourceSize) == 0) {
                sprite->sublist = kAsepriteSpriteJsonSublistFrames;
            }
        } else if (sprite->sublist == kAsepriteSpriteJsonSublistFramesSourceSize) {
            if (strcmp(name, asepriteSpriteJsonSublistNameFramesSourceSize) == 0) {
                sprite->sublist = kAsepriteSpriteJsonSublistFrames;
            }
        }
    }

    // ログ
    // playdate->system->logToConsole("AsepriteSpriteJsonDidDecodeSublist: %s: %s", name, AsepriteGetJsonTypeName(type));

    // 終了
    return NULL;
}

// スプライトフレームを取得する
//
struct AsepriteSpriteFrame *AsepriteGetSpriteFrame(struct AsepriteSprite *sprite, int index)
{
    return &sprite->frames[index];
}

// スプライトアニメーションを開始する
//
void AsepriteStartSpriteAnimation(struct AsepriteSpriteAnimation *animation, const char *spriteName, const char *animationName, bool loop)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // スプライトの取得
    struct AsepriteSprite *sprite = AsepriteFindSprite(spriteName);
    if (sprite == NULL) {
        playdate->system->error("%s: %d: sprite is not entry: %s", __FILE__, __LINE__, spriteName);
        return;
    }

    // タグの検索
    struct AsepriteSpriteTag *tag = NULL;
    for (int i = 0; i < sprite->tagSize; i++) {
        if (strcmp(animationName, sprite->tags[i].name) == 0) {
            tag = &sprite->tags[i];
            break;
        }
    }
    if (tag == NULL) {
        playdate->system->error("%s: %d: animation is not entry: %s", __FILE__, __LINE__, animationName);
        return;
    }

    // アニメーションの設定
    animation->sprite = sprite;
    animation->play = tag->from;
    animation->from = tag->from;
    animation->to = tag->to;
    animation->millisecond = -IocsGetFrameMillisecond();
    animation->loop = loop;
}

// スプライトアニメーションを更新する
//
void AsepriteUpdateSpriteAnimation(struct AsepriteSpriteAnimation *animation)
{
    
    animation->millisecond += IocsGetFrameMillisecond();
    while (animation->millisecond >= animation->sprite->frames[animation->play].duration) {
        animation->millisecond -= animation->sprite->frames[animation->play].duration;
        if (++animation->play > animation->to) {
            if (animation->loop) {
                animation->play = animation->from;
            } else {
                animation->play = animation->to;
                animation->millisecond = animation->sprite->frames[animation->play].duration;
                break;
            }
        }
    }
}

// スプライトアニメーションが完了したかどうかを判定する
//
bool AsepriteIsSpriteAnimationDone(struct AsepriteSpriteAnimation *animation)
{
    return (
        !animation->loop && 
        animation->play == animation->to && 
        animation->millisecond == animation->sprite->frames[animation->play].duration
    ) ? true : false;
}

// スプライトアニメーションを描画する
//
void AsepriteDrawSpriteAnimation(struct AsepriteSpriteAnimation *animation, int x, int y, LCDBitmapDrawMode mode, LCDBitmapFlip flip)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // ビットマップの描画
    {
        struct AsepriteSpriteFrame *frame = &animation->sprite->frames[animation->play];
        struct AsepriteSpriteBitmap *bitmap = &animation->sprite->bitmaps[frame->bitmap];
        playdate->graphics->setDrawMode(mode);
        // playdate->graphics->drawBitmap(bitmap->bitmap, x, y, flip);
        {
            if (flip == kBitmapFlippedX || flip == kBitmapFlippedXY) {
                x = x + (frame->sourceSize.w - (frame->spriteSourceSize.x + frame->spriteSourceSize.w));
            }
            if (flip == kBitmapFlippedY || flip == kBitmapFlippedXY) {
                y = y + (frame->sourceSize.h - (frame->spriteSourceSize.y + frame->spriteSourceSize.h));
            }
            playdate->graphics->drawBitmap(bitmap->bitmap, x, y, flip);
        }
    }
}
void AsepriteDrawRotatedSpriteAnimation(struct AsepriteSpriteAnimation *animation, int x, int y, float degrees, float centerx, float centery, float xscale, float yscale, LCDBitmapDrawMode mode)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // ビットマップの描画
    {
        struct AsepriteSpriteFrame *frame = &animation->sprite->frames[animation->play];
        struct AsepriteSpriteBitmap *bitmap = &animation->sprite->bitmaps[frame->bitmap];
        playdate->graphics->setDrawMode(mode);
        // playdate->graphics->drawRotatedBitmap(bitmap->bitmap, x, y, degrees, centerx, centery, xscale, yscale);
        {
            float fx = ((float)frame->spriteSourceSize.x - (float)frame->sourceSize.w * centerx) * xscale;
            float fy = ((float)frame->spriteSourceSize.y - (float)frame->sourceSize.h * centery) * yscale;
            float radian = (float)M_PI * degrees / 180.0f;
            float sf = sinf(radian);
            float cf = cosf(radian);
            playdate->graphics->drawRotatedBitmap(bitmap->bitmap, x + (int)(fx * cf - fy * sf), y + (int)(fx * sf + fy * cf), degrees, 0.0f, 0.0f, xscale, yscale);
        }
    }
}

// スプライトアニメーションの現在のフレームインデックスを取得する
//
int AsepriteGetSpriteAnimationPlayFrameIndex(struct AsepriteSpriteAnimation *animation)
{
    return animation->play;
}

// .json の値の名前を取得する
//
static const char *AsepriteGetJsonValueName(json_value value)
{
    return AsepriteGetJsonTypeName(value.type);
}
static const char *AsepriteGetJsonTypeName(json_value_type type)
{
    const char *s = NULL;
	if (type == kJSONNull) {
        s = "null";
    } else if (type == kJSONTrue) {
        s = "true";
    } else if (type == kJSONFalse) {
        s = "false";
    } else if (type == kJSONInteger) {
		s = "integer";
    } else if (type == kJSONFloat) {
		s = "float";
    } else if (type == kJSONString) {
		s = "string";
    } else if (type == kJSONArray) {
		s = "array";
    } else if (type == kJSONTable) {
		s = "table";
    } else {
		s = "unknown";
	}
    return s;
}

// .json ファイルを読み込む
//
static bool AsepriteLoadJson(struct AsepriteJson *json, const char *path)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return false;
    }

    // ファイルの読み込み
    FileStat stat;
    if (playdate->file->stat(path, &stat) == 0) {
        json->base = playdate->system->realloc(NULL, stat.size);
        if (json->base != NULL) {
            SDFile *file = playdate->file->open(path, kFileRead);
            if (file != NULL) {
                playdate->file->read(file, json->base, stat.size);
                playdate->file->close(file);
                json->ptr = 0;
                json->size = stat.size;
            }
        }
    }

    // 終了
    return true;
}

// .json を破棄する
//
static void AsepriteUnloadJson(struct AsepriteJson *json)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // .json の解放
    if (json->base != NULL) {
        playdate->system->realloc(json->base, 0);
        json->base = NULL;
    }
}

// .json の読み込み関数
//
static int AsepriteReadJson(void *userdata, uint8_t *buffer, int size)
{
    struct AsepriteJson *json = (struct AsepriteJson *)userdata;
    uint8_t *p = &json->base[json->ptr];
    int read = 0;
    while (read < size && json->ptr < json->size) {
        *buffer++ = *p++;
        ++json->ptr;
        ++read;
    }
    return read;
}

