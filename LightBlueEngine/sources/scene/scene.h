#ifndef __SCENE_H__
#define __SCENE_H__

// <>インクルード
#include <d3d11.h>
#include <memory>
#include <atomic>

// ""インクルード
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
// ゲームシーンの原型になるクラス。
class Scene
{
public:
	// public:コンストラクタ・デストラクタ
	Scene() {};
	virtual ~Scene() = default;

	// public:通常関数
	virtual void		Initialize() = 0;
	virtual void		Update(float) = 0;
	virtual void		DebugGUI() = 0;
	virtual void		Render() = 0;

	// public:ゲッター関数
	bool			IsReady() const				{ return ready; }
	bool			IsTransition() const		{ return transition; }
	int				GetSceneStateNum() const	{ return SCast(int, scene_state); };
	float			GetSceneTime() const		{ return scene_time; };
	EnumSceneState	GetSceneState() const		{ return scene_state; };

	// public:セッター関数
	void			SetReady() { ready = true; }

protected:
	// protected:変数
	bool						ready			= false;
	bool						pause			= false;
	bool						transition		= false;
	float						scene_time		= 0.0f;
	float						transition_time = 1.0f;			//トランジションの時間
	EnumSceneState				scene_state		= EnumSceneState::SCENE_SETTING;
};

#endif // __SCENE_H__