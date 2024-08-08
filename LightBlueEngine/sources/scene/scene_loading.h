#ifndef __SCENE_LOADER_H__
#define __SCENE_LOADER_H__

// <>インクルード
#include <thread>

// ""インクルード
// LightBlueEngine
#include "2D/sprite.h"
#include "scene.h"

// class >> [SceneLoading]
// シーン読み込み中にローディング画面を表示させるクラス。
class SceneLoading : public Scene
{
private:
	// private:定数
	static constexpr float EASING_LOOP_TIME = 2.0f;

	// private:構造体

public:
	// public:コンストラクタ・デストラクタ
	SceneLoading(Scene* next_scene) : scene(next_scene) {}
	~SceneLoading() override;

	// public:通常関数
	void		Initialize() override;
	void		Update(float) override;
	void		DebugGUI() override {};
	void		Render() override;

	// public:static関数
	static void LoadingThread(SceneLoading* scene);

	// public:ゲッター関数

	// public:セッター関数

private:
	// private:変数
	float							easing_loop		= 0.0f;
	std::unique_ptr<std::jthread>	thread			= nullptr;
	std::unique_ptr<Scene>			scene			= nullptr;
	std::unique_ptr<Sprite>			loading_sprite	= nullptr;
	std::unique_ptr<Sprite>			loading_block	= nullptr;
};

#endif // __SCENE_LOADER_H__