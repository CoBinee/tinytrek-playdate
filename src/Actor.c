// Actor.c - アクタ
//

// 外部参照
//
#include <string.h>
#include "pd_api.h"
#include "Iocs.h"
#include "Actor.h"

// 内部関数
//

// 内部変数
//
static struct ActorController *actorController = NULL;


// アクタを初期化する
//
void ActorInitialize(void)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // アクタコントローラの作成
    actorController = playdate->system->realloc(NULL, sizeof (struct ActorController));
    if (actorController == NULL) {
        playdate->system->error("%s: %d: actor controller instance is not created.", __FILE__, __LINE__);
        return;
    }
    memset(actorController, 0, sizeof (struct ActorController));

    // アクタコントローラの初期化
    {
        for (int i = 0; i < kActorEntry - 1; i++) {
            struct Actor *here = (struct Actor *)actorController->blocks[i];
            struct Actor *next = (struct Actor *)actorController->blocks[i + 1];
            here->priorityNext = next;
            next->priorityPrevious = here;
        }
        actorController->free = (struct Actor *)actorController->blocks[0];
    }
}

// アクタを更新する
//
void ActorUpdate(void)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // 描画順のクリア
    for (int i = 0; i < kActorOrderSize; i++) {
        actorController->orders[i] = NULL;
    }

    // アクタの更新
    for (int i = 0; i < kActorPrioritySize; i++) {
        struct Actor *actor = actorController->prioritys[i];
        while (actor != NULL) {
            struct Actor *next = actor->priorityNext;
            if (actor->update != NULL) {
                (*actor->update)(actor);
            }
            actor = next;
        }
    }
}

// アクタを描画する
//
void ActorDraw(void)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // アクタの描画
    for (int i = 0; i < kActorOrderSize; i++) {
        struct Actor *actor = actorController->orders[i];
        while (actor != NULL) {
            struct Actor *next = actor->orderNext;
            if (actor->draw != NULL) {
                (*actor->draw)(actor);
            }
            actor = next;
        }
    }
}

// アクタを読み込む
//
struct Actor *ActorLoad(ActorFunction update, int priority)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return NULL;
    }

    // アクタの取得
    struct Actor *actor = actorController->free;
    if (actor != NULL) {

        // リンクの更新
        actorController->free = actor->priorityNext;
        if (actor->priorityNext != NULL) {
            actor->priorityNext->priorityPrevious = NULL;
        }

        // プライオリティの設定
        if (priority < 0) {
            priority = 0;
        } else if (priority >= kActorPrioritySize) {
            priority = kActorPrioritySize - 1;
        }
        {
            struct Actor *head = actorController->prioritys[priority];
            actor->priorityPrevious = NULL;
            actor->priorityNext = head;
            if (head != NULL) {
                head->priorityPrevious = actor;
            }
            actor->priority = priority;
            actor->update = update;
            actorController->prioritys[priority] = actor;
        }

        // アクタの初期化
        actor->orderPrevious = NULL;
        actor->orderNext = NULL;
        actor->order = 0;
        actor->tagPrevious = NULL;
        actor->tagNext = NULL;
        actor->tag = 0;
        actor->unload = NULL;
        actor->draw = NULL;
        actor->state = 0;
    }

    // 終了
    return actor;
}

// アクタを解放する
//
void ActorUnload(struct Actor *actor)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // 解放処理
    if (actor->unload != NULL) {
        (*actor->unload)(actor);
        actor->unload = NULL;
    }

    // タグの解除
    ActorUnsetTag(actor);

    // 描画順の解除
    ActorUnsetDraw(actor);

    // プライオリティの解除
    struct Actor *previous = actor->priorityPrevious;
    struct Actor *next = actor->priorityNext;
    if (previous != NULL) {
        previous->priorityNext = next;
    } else {
        actorController->prioritys[actor->priority] = next;
    }
    if (next != NULL) {
        next->priorityPrevious = previous;
    }

    // 解放の完了
    actor->priorityPrevious = NULL;
    actor->priorityNext = actorController->free;
    actorController->free = actor;
}

// すべてのアクタを解放する
//
void ActorUnloadAll(void)
{
    for (int i = 0; i < kActorPrioritySize; i++) {
        struct Actor *actor = actorController->prioritys[i];
        while (actor != NULL) {
            struct Actor *next = actor->priorityNext;
            ActorUnload(actor);
            actor = next;
        }
    }
}

// 指定されたタグのアクタを解放する
//
void ActorUnloadWithTag(int tag)
{
    struct Actor *actor = ActorFindWithTag(tag);
    while (actor != NULL) {
        struct Actor *next = ActorNextWithTag(actor);
        ActorUnload(actor);
        actor = next;
    }
}

// アクタの更新処理を遷移する
//
void ActorTransition(struct Actor *actor, ActorFunction update)
{
    actor->update = update;
    actor->state = 0;
}

// アクタの解放処理を設定する
//
void ActorSetUnload(struct Actor *actor, ActorFunction unload)
{
    actor->unload = unload;
}

// アクタの描画処理を設定する
//
void ActorSetDraw(struct Actor *actor, ActorFunction draw, int order)
{
    if (order < 0) {
        order = 0;
    } else if (order >= kActorOrderSize) {
        order = kActorOrderSize - 1;
    }
    {
        struct Actor *head = actorController->orders[order];
        actor->orderPrevious = NULL;
        actor->orderNext = head;
        if (head != NULL) {
            head->orderPrevious = actor;
        }
        actor->order = order;
        actor->draw = draw;
        actorController->orders[order] = actor;
    }
}

// アクタの描画処理を解放する
//
void ActorUnsetDraw(struct Actor *actor)
{
    struct Actor *previous = actor->orderPrevious;
    struct Actor *next = actor->orderNext;
    if (previous != NULL) {
        previous->orderNext = next;
    } else {
        actorController->orders[actor->order] = next;
    }
    if (next != NULL) {
        next->orderPrevious = previous;
    }
}

// アクタのタグを設定する
//
void ActorSetTag(struct Actor *actor, int tag)
{
    if (tag < 0) {
        tag = 0;
    } else if (tag >= kActorTagSize) {
        tag = kActorOrderSize - 1;
    }
    {
        struct Actor *head = actorController->tags[tag];
        actor->tagPrevious = NULL;
        actor->tagNext = head;
        if (head != NULL) {
            head->tagPrevious = actor;
        }
        actor->tag = tag;
        actorController->tags[tag] = actor;

    }
}

// アクタのタグを解除する
//
void ActorUnsetTag(struct Actor *actor)
{
    struct Actor *previous = actor->tagPrevious;
    struct Actor *next = actor->tagNext;
    if (previous != NULL) {
        previous->tagNext = next;
    } else {
        actorController->tags[actor->tag] = next;
    }
    if (next != NULL) {
        next->tagPrevious = previous;
    }
}

// タグでアクタを検索する
//
struct Actor *ActorFindWithTag(int tag)
{
    return actorController->tags[tag];
}
struct Actor *ActorNextWithTag(struct Actor *actor)
{
    return actor->tagNext;
}
