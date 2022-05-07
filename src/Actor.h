// Actor.h - アクタ
//
#pragma once

// 外部参照
//
#include <stdbool.h>
#include "pd_api.h"


// アクタ関数
//
typedef void (*ActorFunction)(void *);

// プライオリティ
//
enum {
    kActorPriorityHigh = 0, 
    kActorPriorityLow = 7, 
    kActorPrioritySize = kActorPriorityLow + 1, 
};

// 描画順
//
enum {
    kActorOrderBack = 0, 
    kActorOrderSprite = 1, 
    kActorOrderFront = 511, 
    kActorOrderSize = kActorOrderFront + 1, 
};

// タグ
//
enum {
    kActorTagNull = 0, 
    kActorTagSize = 16, 
};

// アクタ
//
enum {
    kActorEntry = 32, 
};
struct Actor {

    // プライオリティ
    struct Actor *priorityPrevious;
    struct Actor *priorityNext;
    int priority;

    // 描画順
    struct Actor *orderPrevious;
    struct Actor *orderNext;
    int order;

    // タグ
    struct Actor *tagPrevious;
    struct Actor *tagNext;
    int tag;

    // 更新処理
    ActorFunction update;

    // 解放処理
    ActorFunction unload;

    // 描画処理
    ActorFunction draw;

    // 状態
    int state;

};

// アクタブロック
//
enum {
    kActorBlockSize = 1024, 
};

// アクタコントローラ
//
struct ActorController {

    // 解放されたアクタのリンク
    struct Actor *free;

    // プライオリティ別のアクタのリンク
    struct Actor *prioritys[kActorPrioritySize];

    // 描画順別のアクタのリンク
    struct Actor *orders[kActorOrderSize];

    // タグ別のアクタのリンク
    struct Actor *tags[kActorTagSize];

    // アクタブロック
    uint8_t blocks[kActorEntry][kActorBlockSize];
    
};


// 外部参照関数
//
extern void ActorInitialize(void);
extern void ActorUpdate(void);
extern void ActorDraw(void);
extern struct Actor *ActorLoad(ActorFunction update, int priority);
extern void ActorUnload(struct Actor *actor);
extern void ActorUnloadAll(void);
extern void ActorUnloadWithTag(int tag);
extern void ActorTransition(struct Actor *actor, ActorFunction update);
extern void ActorSetUnload(struct Actor *actor, ActorFunction unload);
extern void ActorSetDraw(struct Actor *actor, ActorFunction draw, int order);
extern void ActorUnsetDraw(struct Actor *actor);
extern void ActorSetTag(struct Actor *actor, int tag);
extern void ActorUnsetTag(struct Actor *actor);
extern struct Actor *ActorFindWithTag(int tag);
extern struct Actor *ActorNextWithTag(struct Actor *actor);
