#ifndef __SCENE_MANAGER_H__
#define __SCENE_MANAGER_H__

// <>�C���N���[�h
#include <memory>

// ""�C���N���[�h
// LightBlueEngine
#include "2D/sprite.h"
#include "audio/audio.h"
#include "singleton.h"
#include "scene.h"

// class >> [SceneManager]
// �Q�[���V�[�����Ǘ�����N���X�B
class SceneManager
{
public:
	// public:�R���X�g���N�^�E�f�X�g���N�^
	SceneManager() {};
	~SceneManager() = default;

	//public:�R�s�[�R���X�g���N�^�E���[�u�R���X�g���N�^
	SceneManager(const SceneManager&) = delete;
	SceneManager& operator = (const SceneManager&) = delete;
	SceneManager(SceneManager&&) noexcept = delete;
	SceneManager& operator= (SceneManager&&) noexcept = delete;

	// public:�ʏ�֐�
	void		Update(float);
	void		DebugGUI();
	void		Render();
	void		Clear();
	void		ChangeScene(Scene*);

	// public:�Q�b�^�[�֐�
	bool	IsTransition() { return current_scene->IsTransition(); }
	Scene* GetCurrentScene() { return current_scene.get(); }

private:
	// private:�ϐ�
	std::unique_ptr<Scene>	current_scene;
	std::unique_ptr<Scene>	next_scene;
	std::unique_ptr<Sprite> font_sprite;
};

#endif // __SCENE_MANAGER_H__