#ifndef __SCENE_GAME_H__
#define __SCENE_GAME_H__

// ""インクルード
// LightBlueEngine
#include "2D/sprite.h"
#include "graphics/bloom_effect.h"
#include "scene/scene.h"

// ゲームソースコード
#include "object_board.h"

// class >> [SceneGame] 継承：Scene
class SceneGame : public Scene
{
public:
	// public:コンストラクタ・デストラクタ
	SceneGame() {};
	~SceneGame() override;

	// public:通常関数
	void		Initialize() override;
	void		Update(float) override;
	void		DebugGUI() override;
	void		Render() override;

private:
	// private:変数
	float								roll_angle			= 0.0f;
	std::unique_ptr<ParticleSystem>		background_particle;
	std::unique_ptr<Sprite>				config_move_pad;
	std::unique_ptr<Sprite>				config_rotate_pad;
	std::unique_ptr<Sprite>				config_move_keyboard;
	std::unique_ptr<Sprite>				config_rotate_keyboard;
	std::unique_ptr<GameModel>			model;
	std::unique_ptr<BloomEffect>		main_bloom;
	std::unique_ptr<BloomEffect>		emissive_bloom;

	UPtrVector<ObjectBoard>				game_board;
};

#endif // __SCENE_SAMPLE_H__