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