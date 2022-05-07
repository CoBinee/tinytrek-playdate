// main.c - プログラムのエントリポイント
//

// 参照ファイル
//
#include "pd_api.h"
#include "Iocs.h"
#include "Aseprite.h"
#include "Scene.h"
#include "Actor.h"
#include "Application.h"

// 内部関数
//
static int updateCallback(void* userdata);

// 内部変数
//


// イベントハンドラ
//
#ifdef _WINDLL
__declspec(dllexport)
#endif
int eventHandler(PlaydateAPI *playdate, PDSystemEvent event, uint32_t arg)
{
	// kEventInit: 初期化
	if (event == kEventInit) {
		playdate->system->logToConsole("%s: %d: kEventInit.", __FILE__, __LINE__);

		// IOCS の初期化
		IocsInitialize(playdate);

		// Aseprite の初期化
		AsepriteInitialize("images/");

		// シーンの初期化
		SceneInitialize();

		// アクタの初期化
		ActorInitialize();

		// アプリケーションの初期化
		ApplicationInitialize();

		// コールバック関数の設定
		playdate->system->setUpdateCallback(updateCallback, playdate);

	// それ以外のイベント
	} else {

		// イベントの処理
		IocsEventHandler(event, arg);
	}
	
	// 終了
	return 0;
}

// 更新のコールバック関数
//
static int updateCallback(void *userdata)
{
	// IOCS の更新の開始
	IocsUpdateBegin();

	// シーンの更新の開始
	SceneUpdateBegin();

	// アクタの更新
	ActorUpdate();

	// 画面のクリア
	IocsClearScreen();

	// アクタの描画
	ActorDraw();

	// シーンの更新の完了
	SceneUpdateEnd();

	// IOCS の更新の完了
	IocsUpdateEnd();

	// 終了
	return 1;
}

