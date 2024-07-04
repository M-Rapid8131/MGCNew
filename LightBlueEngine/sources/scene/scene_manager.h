#ifndef __SCENE_MANAGER_H__
#define __SCENE_MANAGER_H__

// <>インクルード
#include <memory>

// ""インクルード
// LightBlueEngine
#include "2D/sprite.h"
#include "audio/audio.h"
#include "singleton.h"
#include "scene.h"

// class >> [SceneManager]
// ゲームシーンを管理するクラス。
class SceneManager
{
public:
	// public:コンストラクタ・デストラクタ
	SceneManager() {};
	~SceneManager() = default;

	//public:コピーコンストラクタ・ムーブコンストラクタ
	SceneManager(const SceneManager&) = delete;
	SceneManager& operator = (const SceneManager&) = delete;
	SceneManager(SceneManager&&) noexcept = delete;
	SceneManager& operator= (SceneManager&&) noexcept = delete;

	// public:通常関数
	void		Update(float);
	void		DebugGUI();
	void		Render();
	void		Clear();
	void		ChangeScene(Scene*);

	// public:ゲッター関数
	bool	IsTransition() { return current_scene->IsTransition(); }
	Scene* GetCurrentScene() { return current_scene.get(); }

private:
	// private:変数
	std::unique_ptr<Scene>	current_scene;
	std::unique_ptr<Scene>	next_scene;
	std::unique_ptr<Sprite> font_sprite;
};

#endif // __SCENE_MANAGER_H__