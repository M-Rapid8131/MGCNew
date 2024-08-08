#ifndef __SCENE_H__
#define __SCENE_H__

// <>�C���N���[�h
#include <d3d11.h>
#include <memory>
#include <atomic>

// ""�C���N���[�h
// LightBlueEngine
#include "engine_utility.h"

// enum class
enum class EnumSceneState
{
	SCENE_SETTING,
	TRANSITION_IN_SETTING,
	TRANSITION_IN,
	SCENE_MAIN,
	TRANSITION_OUT_SETTING,
	TRANSITION_OUT,
	SCENE_CHANGE,
};

enum class EnumSceneChangeDirection
{
	NONE,
	SCENE_MAINMENU,
	SCENE_GAME,
	SCENE_RESULT,
};

// class >> [Scene]
// �Q�[���V�[���̌��^�ɂȂ�N���X�B
class Scene
{
public:
	// public:�R���X�g���N�^�E�f�X�g���N�^
	Scene() {};
	virtual ~Scene() = default;

	// public:�ʏ�֐�
	virtual void		Initialize() = 0;
	virtual void		Update(float) = 0;
	virtual void		DebugGUI() = 0;
	virtual void		Render() = 0;

	// public:�Q�b�^�[�֐�
	bool			IsReady() const				{ return ready; }
	bool			IsTransition() const		{ return transition; }
	int				GetSceneStateNum() const	{ return SCast(int, scene_state); };
	float			GetSceneTime() const		{ return scene_time; };
	EnumSceneState	GetSceneState() const		{ return scene_state; };

	// public:�Z�b�^�[�֐�
	void			SetReady() { ready = true; }

protected:
	// protected:�ϐ�
	bool						ready			= false;
	bool						pause			= false;
	bool						transition		= false;
	float						scene_time		= 0.0f;
	float						transition_time = 1.0f;			//�g�����W�V�����̎���
	EnumSceneState				scene_state		= EnumSceneState::SCENE_SETTING;
};

#endif // __SCENE_H__