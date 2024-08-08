#ifndef __GAME_PAD_H__
#define __GAME_PAD_H__

// <>インクルード
#include <Windows.h>

// ""インクルード
// LightBlueEngine
#include "engine_utility.h"

// using
using GamePadButton = unsigned int;

// enum class
enum class EnumGamePadValue
{
	CURRENT,
	OLD
};

enum class EnumInputDevice
{
	XBOX,
	PS,
	KEYBOARD,

	UNDEFINE
};

// namespace
inline namespace Button
{
	const GamePadButton BTN_UP				= (1 << 0);
	const GamePadButton BTN_RIGHT			= (1 << 1);
	const GamePadButton BTN_DOWN			= (1 << 2);
	const GamePadButton BTN_LEFT			= (1 << 3);
	const GamePadButton BTN_A				= (1 << 4);
	const GamePadButton BTN_B				= (1 << 5);
	const GamePadButton BTN_X				= (1 << 6);
	const GamePadButton BTN_Y				= (1 << 7);
	const GamePadButton BTN_START			= (1 << 8);
	const GamePadButton BTN_BACK			= (1 << 9);
	const GamePadButton BTN_LEFT_THUMB		= (1 << 10);
	const GamePadButton BTN_RIGHT_THUMB		= (1 << 11);
	const GamePadButton BTN_LEFT_SHOULDER	= (1 << 12);
	const GamePadButton BTN_RIGHT_SHOULDER	= (1 << 13);
	const GamePadButton BTN_LEFT_TRIGGER	= (1 << 14);
	const GamePadButton BTN_RIGHT_TRIGGER	= (1 << 15);
};

// class >> [GamePad]
// ゲームパッド操作を管理するクラス。
class GamePad
{
public:
	// public:コンストラクタ・デストラクタ
	GamePad()	{}
	~GamePad()	{}

	// public:通常関数
	void Update();
	void DebugGUI();

	// public:ゲッター関数
	float			GetAxisLX() const			{ return axis_lx; }
	float			GetAxisLY() const			{ return axis_ly; }
	float			GetAxisRX() const			{ return axis_rx; }
	float			GetAxisRY() const			{ return axis_ry; }
	float			GetTriggerL() const			{ return trigger_l; }
	float			GetTriggerR() const			{ return trigger_r; }
	GamePadButton	GetButtonDown() const		{ return button_down; }
	GamePadButton	GetButtonUp() const			{ return button_up; }
	GamePadButton	GetButton() const			{ return button_state[SCast(size_t,EnumGamePadValue::CURRENT)]; }
	EnumInputDevice GetInputDevice()			{ return input_device; }

	// public:セッター関数
	void SetSlot(UINT i_slot)			{ slot = i_slot; }

private:
	// private:変数
	float			axis_lx			= 0.0f;
	float			axis_ly			= 0.0f;
	float			axis_rx			= 0.0f;
	float			axis_ry			= 0.0f;
	float			trigger_l		= 0.0f;
	float			trigger_r		= 0.0f;
	float			left_vibration	= 0;
	float			right_vibration	= 0;
	UINT			slot			= 0;
	GamePadButton	button_down		= 0;
	GamePadButton	button_up		= 0;
	EnumInputDevice input_device	= EnumInputDevice::UNDEFINE;

	GamePadButton	button_state[2] = { 0,0 };
};

#endif // __GAME_PAD_H__