// ""インクルード
// LightBlueEngine
#include "graphics/graphics.h"
#include "game_system/gamesystem_director.h"
#include "scene_loading.h"

//-------------------------------------------
// SceneLoading メンバ関数
//-------------------------------------------

// デストラクタ
SceneLoading::~SceneLoading()
{
	if (thread.get())
	{
		thread->join();
		thread.reset(nullptr);
	}
}

// 初期化処理
void SceneLoading::Initialize()
{
	// threadの作成
	thread = std::make_unique<std::jthread>(LoadingThread, this);

	loading_sprite = std::make_unique<Sprite>(L"./resources/sprite/loading.png");
}

// 更新処理
void SceneLoading::Update(float elapsed_time)
{
	SceneManager* scene_manager = GamesystemDirector::GetInstance()->GetSceneManager();
	if (scene && scene->IsReady())
	{
		scene_manager->ChangeScene(scene.release());
	}

	scene_time = elapsed_time;
}

// 描画処理
void SceneLoading::Render()
{
	// ここにローディング画面のスプライトとかなんかを入れる
	loading_sprite->Render({ 0.0f,0.0f },
		{ SCast(float, Graphics::GetInstance()->GetScreenWidth()), SCast(float, Graphics::GetInstance()->GetScreenHeight()) }
	);
}

// マルチスレッドでこの次のシーンを読み込む
void SceneLoading::LoadingThread(SceneLoading* scene)
{
	if (!scene->scene.get())	return;

	CoInitialize(nullptr);

	scene->scene->Initialize();

	CoUninitialize();

	scene->scene->SetReady();
}