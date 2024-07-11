#ifndef __SCENE_MAIN_MANU_H__
#define __SCENE_MAIN_MANU_H__

// ""�C���N���[�h
// LightBlueEngine
#include "2D/sprite.h"
#include "graphics/bloom_effect.h"
#include "scene/scene.h"

// �Q�[���\�[�X�R�[�h
#include "object_board.h"

// class >> [SceneMainManu] �p���FScene
class SceneMainManu : public Scene
{
private:
	static constexpr float BLOCK_SIZE = 10.0f;

public:
	// public:�R���X�g���N�^�E�f�X�g���N�^
	SceneMainManu() {};
	~SceneMainManu() override;

	// public:�ʏ�֐�
	void		Initialize() override;
	void		Update(float) override;
	void		DebugGUI() override;
	void		Render() override;

private:
	// private:�ϐ�
	DirectX::XMFLOAT3					hsv_color = { 0.0f, 1.0f, 1.0f };
	std::unique_ptr<Sprite>				logo;
	std::unique_ptr<Sprite>				menu_key;
	std::unique_ptr<ParticleSystem>		background_particle;
	std::unique_ptr<GameModel>			model;
	std::unique_ptr<BloomEffect>		main_bloom;
	std::unique_ptr<BloomEffect>		emissive_bloom;
};

#endif // __SCENE_SAMPLE_H__