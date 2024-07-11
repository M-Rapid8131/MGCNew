#ifndef __SCENE_MAIN_MANU_H__
#define __SCENE_MAIN_MANU_H__

// ""インクルード
// LightBlueEngine
#include "2D/sprite.h"
#include "graphics/bloom_effect.h"
#include "scene/scene.h"

// ゲームソースコード
#include "object_board.h"

// class >> [SceneMainManu] 継承：Scene
class SceneMainManu : public Scene
{
private:
	static constexpr float BLOCK_SIZE = 10.0f;

public:
	// public:コンストラクタ・デストラクタ
	SceneMainManu() {};
	~SceneMainManu() override;

	// public:通常関数
	void		Initialize() override;
	void		Update(float) override;
	void		DebugGUI() override;
	void		Render() override;

private:
	// private:変数
	DirectX::XMFLOAT3					hsv_color = { 0.0f, 1.0f, 1.0f };
	std::unique_ptr<Sprite>				logo;
	std::unique_ptr<Sprite>				menu_key;
	std::unique_ptr<ParticleSystem>		background_particle;
	std::unique_ptr<GameModel>			model;
	std::unique_ptr<BloomEffect>		main_bloom;
	std::unique_ptr<BloomEffect>		emissive_bloom;
};

#endif // __SCENE_SAMPLE_H__