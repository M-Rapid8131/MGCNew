#ifndef __INPUT_H__
#define __INPUT_H__

// <>�C���N���[�h
#include <array>
#include <memory>

// ""�C���N���[�h
// LightBlueEngine
#include "mouse.h"
#include "game_pad.h"

// class >> [Input]
// �Q�[���ł̑�����Ǘ�����N���X�B
class Input
{
public:
	// public:�萔
	static const UINT MAX_PLAYER = 4;

	// public:�R���X�g���N�^�E�f�X�g���N�^
	Input()	{}

	~Input() {}

	// public:�ʏ�֐�
	virtual void	Initialize(HWND, UINT);
	virtual void	Update(float);
	virtual void	DebugGUI();
	void			AddGamePad(UINT);
	void			DeleteGamePad(UINT);
	Mouse&			GetMouse()			{ return mouse; }
	GamePad*		GetGamePad(UINT id = 0)	{ return game_pad[id].get(); }

protected:
	// private:�ϐ�
	UINT	players;
	Mouse	mouse;

	std::array<std::unique_ptr<GamePad>, MAX_PLAYER> game_pad;
};

#endif // __INPUT_H__