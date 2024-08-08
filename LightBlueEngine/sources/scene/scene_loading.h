#ifndef __SCENE_LOADER_H__
#define __SCENE_LOADER_H__

// <>�C���N���[�h
#include <thread>

// ""�C���N���[�h
// LightBlueEngine
#include "2D/sprite.h"
#include "scene.h"

// class >> [SceneLoading]
// �V�[���ǂݍ��ݒ��Ƀ��[�f�B���O��ʂ�\��������N���X�B
class SceneLoading : public Scene
{
private:
	// private:�萔
	static constexpr float EASING_LOOP_TIME = 2.0f;

	// private:�\����

public:
	// public:�R���X�g���N�^�E�f�X�g���N�^
	SceneLoading(Scene* next_scene) : scene(next_scene) {}
	~SceneLoading() override;

	// public:�ʏ�֐�
	void		Initialize() override;
	void		Update(float) override;
	void		DebugGUI() override {};
	void		Render() override;

	// public:static�֐�
	static void LoadingThread(SceneLoading* scene);

	// public:�Q�b�^�[�֐�

	// public:�Z�b�^�[�֐�

private:
	// private:�ϐ�
	float							easing_loop		= 0.0f;
	std::unique_ptr<std::jthread>	thread			= nullptr;
	std::unique_ptr<Scene>			scene			= nullptr;
	std::unique_ptr<Sprite>			loading_sprite	= nullptr;
	std::unique_ptr<Sprite>			loading_block	= nullptr;
};

#endif // __SCENE_LOADER_H__