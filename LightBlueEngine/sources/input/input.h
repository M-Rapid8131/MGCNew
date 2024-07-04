#ifndef __INPUT_H__
#define __INPUT_H__

// <>インクルード
#include <array>
#include <memory>

// ""インクルード
// LightBlueEngine
#include "mouse.h"
#include "game_pad.h"

// class >> [Input]
// ゲームでの操作を管理するクラス。
class Input
{
public:
	// public:定数
	static const UINT MAX_PLAYER = 4;

	// public:コンストラクタ・デストラクタ
	Input()	{}

	~Input() {}

	// public:通常関数
	virtual void	Initialize(HWND, UINT);
	virtual void	Update(float);
	virtual void	DebugGUI();
	void			AddGamePad(UINT);
	void			DeleteGamePad(UINT);
	Mouse&			GetMouse()			{ return mouse; }
	GamePad*		GetGamePad(UINT id = 0)	{ return game_pad[id].get(); }

protected:
	// private:変数
	UINT	players;
	Mouse	mouse;

	std::array<std::unique_ptr<GamePad>, MAX_PLAYER> game_pad;
};

#endif // __INPUT_H__