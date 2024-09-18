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
	static constexpr float BLOCK_SIZE	= 10.0f;
	static constexpr float ROT_SPEED	= 0.2f;
	static constexpr float MOVE_FACTOR	= 1920.0f;

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
	bool								viewing_rule			= false;
	int									selecting_menu			= 0;
	int									rule_page				= 0;
	float								sprite_move				= 0.0f;
	float								sprite_move_time		= 0.0f;
	float								move_easing_rate		= 0.0f;
	DirectX::XMFLOAT3					hsv_color = { 0.0f, 1.0f, 1.0f };
	std::unique_ptr<Sprite>				logo;
	std::unique_ptr<ParticleSystem>		background_particle;
	std::unique_ptr<GameModel>			model;
	std::unique_ptr<BloomEffect>		main_bloom;
	std::unique_ptr<BloomEffect>		emissive_bloom;

	UPtrVector<Sprite>					rule;
	UPtrVector<Sprite>					menu_key;
	UPtrVector<Sprite>					rule_key;
	UPtrVector<Sprite>					rule_close_key;
};

#endif // __SCENE_SAMPLE_H__