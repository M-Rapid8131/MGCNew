#ifndef __SCENE_GAME_H__
#define __SCENE_GAME_H__

// ""�C���N���[�h
// LightBlueEngine
#include "2D/sprite.h"
#include "graphics/bloom_effect.h"
#include "scene/scene.h"

// �Q�[���\�[�X�R�[�h
#include "object_board.h"

// class >> [SceneGame] �p���FScene
class SceneGame : public Scene
{
private:
	static constexpr float MOVE_FACTOR		= 50.0f;
	static constexpr float MENU_DISTANCE	= 100.0f;
	
	static const std::vector<EnumGameMode>	game_mode_list;

public:
	// public:�R���X�g���N�^�E�f�X�g���N�^
	SceneGame() {};
	~SceneGame() override;

	// public:�ʏ�֐�
	void		Initialize() override;
	void		Update(float) override;
	void		DebugGUI() override;
	void		Render() override;

private:
	// private:�ϐ�
	bool								playing_game				= false;
	bool								pausing_game				= false;
	int									selecting_mode_num			= 0;
	int									selecting_pause_menu_num	= 0;
	float								sprite_move					= 0.0f;
	float								sprite_move_time			= 0.0f;
	float								move_easing_rate			= 0.0f;
	std::unique_ptr<ParticleSystem>		background_particle;
	std::unique_ptr<GameModel>			model;
	std::unique_ptr<BloomEffect>		main_bloom;
	std::unique_ptr<BloomEffect>		emissive_bloom;
	std::unique_ptr<Sprite>				mode_select;
	std::unique_ptr<Sprite>				pause_sprite;

	UPtrVector<Sprite>					game_mode_sprite;
	UPtrVector<Sprite>					game_mode_detail;
	UPtrVector<Sprite>					pause_menu;
	UPtrVector<ObjectBoard>				game_board;
};

#endif // __SCENE_SAMPLE_H__