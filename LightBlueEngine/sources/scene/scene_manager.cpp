// <>インクルード
#include <imgui.h>

// ""インクルード
// LightBlueEngine
#include "scene_manager.h"

//-------------------------------------------
// SceneManager メンバ関数
//-------------------------------------------

// 更新処理
void SceneManager::Update(float elapsed_time)
{
	// シーンチェンジ処理
	if (next_scene)
	{
		// 古いシーンを終了
		if (current_scene)
			Clear();

		// 新しいシーンを設定
		current_scene = std::move(next_scene);

		// シーン初期化処理
		if (!current_scene->IsReady())
		{
			current_scene->Initialize();
		}
	}

	// 現在のシーンの更新処理
	if (current_scene)
	{
		current_scene->Update(elapsed_time);
	}
}

// ImGui表示
void SceneManager::DebugGUI()
{
	if (current_scene)
	{
		if (ImGui::CollapsingHeader("Scene"))
		{
			current_scene->DebugGUI();
		}
	}
}

// 描画処理
void SceneManager::Render()
{
	if (current_scene)
	{
		current_scene->Render();
	}
}

// シーンクリア
void SceneManager::Clear()
{
	current_scene.reset();
}

// シーン切り替え
void SceneManager::ChangeScene(Scene* new_scene)
{
	// 新しいシーンを設定
	if (!new_scene)		return;
	next_scene.reset(std::move(new_scene));
}