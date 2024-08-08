#ifndef __SCENE_RESULT_H__
#define __SCENE_RESULT_H__

// <>�C���N���[�h
#include <map>
#include <string>

// ""�C���N���[�h
// LightBlueEngine
#include "2D/sprite.h"
#include "graphics/bloom_effect.h"
#include "scene/scene.h"

// �Q�[���\�[�X�R�[�h
#include "object_board.h"

struct GameData;

// �萔
static const std::map<std::string, float> appear_time = {
	{"result", 0.0f},
	{"level", 1.0f},
	{"count", 1.5f},
	{"score", 2.0f},
};

// class >> [SceneResult] �p���FScene
class SceneResult : public Scene
{
private:
	static constexpr float BLOCK_SIZE = 10.0f;
	static constexpr float ROT_SPEED = 0.2f;

public:
	// public:�R���X�g���N�^�E�f�X�g���N�^
	SceneResult() {};
	~SceneResult() override {};

	// public:�ʏ�֐�
	void		Initialize() override;
	void		Update(float) override;
	void		DebugGUI() override;
	void		Render() override;

private:
	// private:�ϐ�
	std::unique_ptr<Sprite>				result_spr;
	std::unique_ptr<Sprite>				cleared_spr;
	std::unique_ptr<Sprite>				game_over_spr;
	std::unique_ptr<Sprite>				level_spr;
	std::unique_ptr<Sprite>				block_count_spr;
	std::unique_ptr<Sprite>				score_spr;
	std::unique_ptr<Sprite>				menu_key_spr;
};

#endif __SCENE_RESULT_H__