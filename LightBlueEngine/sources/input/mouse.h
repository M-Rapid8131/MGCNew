#ifndef __MOUSE_H__
#define __MOUSE_H__

// <>インクルード
#include <Windows.h>

// ""インクルード
// LightBlueEngine
#include "engine_utility.h"

// using
using MouseButton = unsigned int;

// enum
enum class EnumMouseValue
{
	CURRENT,
	OLD,
	DELTA
};

// class >> [Mouse]
// マウス操作を管理するクラス。
class Mouse
{
public:
	// public:static定数
	static const MouseButton BTN_LEFT	= (1 << 0);
	static const MouseButton BTN_MIDDLE = (1 << 1);
	static const MouseButton BTN_RIGHT	= (1 << 2);

private:
	// private:構造体

	// struct >> Mouse >> [PointInfo]
	struct PointInfo
	{
		POINT	point;
		POINT	old_point;
		POINT	delta_point;
		RECT	rc;
	};

public:
	// public:コンストラクタ・デストラクタ
	Mouse()		{};
	~Mouse()	{};

	// public:通常関数
	void Initialize(HWND hwnd);
	void Update();
	void DebugGUI();

	// public:ゲッター関数
	int			GetWheel() const			{ return wheel[SCast(size_t, EnumMouseValue::OLD)]; }
	int			GetScreenWidth()			{ return screen_width; }
	int			GetScreenHeight()			{ return screen_height; }
	MouseButton GetButton() const			{ return button_state[SCast(size_t, EnumMouseValue::CURRENT)]; };
	MouseButton GetButtonDown() const		{ return button_down; }
	MouseButton GetButtonUp() const			{ return button_up; }

	int			GetPositionX(EnumMouseValue value) const
		{ return position_x[SCast(size_t, value)]; }
	
	int			GetPositionY(EnumMouseValue value) const
		{ return position_y[SCast(size_t, value)]; }

	// public:セッター関数
	void SetScreenWidth(int width)		{ screen_width = width; }
	void SetScreenHeight(int height)	{ screen_height = height; }
	void AddWheel(int i_wheel)			{ wheel[SCast(size_t, EnumMouseValue::CURRENT)] += i_wheel; }

private:
	// private:変数
	int			screen_width		= 0;
	int			screen_height	= 0;
	HWND		hwnd			= nullptr;
	MouseButton button_down		= 0;
	MouseButton button_up		= 0;
	PointInfo	point_info		= {};

	int			wheel[2]		= { 0,0 };
	int			position_x[3]	= { 0,0,0 };
	int			position_y[3]	= { 0,0,0 };
	MouseButton button_state[2] = { 0,0 };
};

#endif // __MOUSE_H__