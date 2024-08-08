#ifndef __SCENE_RESULT_H__
#define __SCENE_RESULT_H__

// <>インクルード
#include <map>
#include <string>

// ""インクルード
// LightBlueEngine
#include "2D/sprite.h"
#include "graphics/bloom_effect.h"
#include "scene/scene.h"

// ゲームソースコード
#include "object_board.h"

struct GameData;

// 定数
static const std::map<std::string, float> appear_time = {
	{"result", 0.0f},
	{"level", 1.0f},
	{"count", 1.5f},
	{"score", 2.0f},
};

// class >> [SceneResult] 継承：Scene
class SceneResult : public Scene
{
private:
	static constexpr float BLOCK_SIZE = 10.0f;
	static constexpr float ROT_SPEED = 0.2f;

public:
	// public:コンストラクタ・デストラクタ
	SceneResult() {};
	~SceneResult() override {};

	// public:通常関数
	void		Initialize() override;
	void		Update(float) override;
	void		DebugGUI() override;
	void		Render() override;

private:
	// private:変数
	std::unique_ptr<Sprite>				result_spr;
	std::unique_ptr<Sprite>				cleared_spr;
	std::unique_ptr<Sprite>				game_over_spr;
	std::unique_ptr<Sprite>				level_spr;
	std::unique_ptr<Sprite>				block_count_spr;
	std::unique_ptr<Sprite>				score_spr;
	std::unique_ptr<Sprite>				menu_key_spr;
};

#endif __SCENE_RESULT_H__