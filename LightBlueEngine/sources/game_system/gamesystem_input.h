#ifndef __GAMESYSTEM_INPUT_H__
#define __GAMESYSTEM_INPUT_H__

// <>�C���N���[�h
#include <memory>
#include <magic_enum.hpp>

// ""�C���N���[�h
// LightBlueEngine
#include "graphics/camera.h"
#include "input/input.h"
#include "singleton.h"

//	MEMO:
//	���FRIGHT�̎������̃u���b�N ���FRIGHT�̎��E���̃u���b�N	
//						   �s�n�o							
//							 ��								
//				�k�d�e�s  �� �� ���@�q�h�f�g�s				
//							 ��								
//						 �t�m�c�d�q							

// enum
enum class EnumBlockRotation
{
	RIGHT,
	UNDER,
	LEFT,
	TOP,
};

// class >> [GamesystemInput] �p���FInput�ASingleton<GamesystemInput>
// Input���g�������A�Q�[���p�̓��̓N���X�B
class GamesystemInput : public Input, public Singleton<GamesystemInput>
{
private:
	// private:�萔
	static constexpr float HOLD_TIME_MOVE		= 0.2f;
	static constexpr float BARRAGE_TIME_MOVE	= 0.02f;
	static constexpr float HOLD_TIME_CANCEL		= 1.0f;
	static constexpr float FLIP_LIMIT			= 0.5f;

	// private:�\����
	// struct >> GamesystemInput >> [GameController]
	struct GameController
	{
		bool						rotate_fail			= false;
		float						rotate_fail_timer	= 0.0f;
		float						hold_timer			= 0.0f;
		float						barrage_timer		= 0.0f;
		EnumBlockRotation			block_rotation;
		std::vector<GamePadButton>	command_buffer;
	};

public:
	// public:�R���X�g���N�^�E�f�X�g���N�^
	GamesystemInput() {};
	~GamesystemInput() {};

	// public:�ʏ�֐�
	void Initialize(HWND, UINT = MAX_PLAYER) override;
	void Update(float) override;
	void DebugGUI();
	void ResetRotate(UINT id)			{ controllers[id].block_rotation = EnumBlockRotation::RIGHT; }
	void RotationFail(UINT, EnumBlockRotation);
	bool BlockRotate(UINT, GamePadButton);
	bool BlockFlip(UINT);
	bool ActivateMoveContinuous(UINT id){ return controllers[id].hold_timer > HOLD_TIME_MOVE; }
	bool MachDrop(UINT id)				{ return game_pad[id]->GetButtonDown() & BTN_UP; }

	// public:�Q�b�^�[�֐�
	GamePadButton		GetGamePadButton(UINT id)		{ return game_pad[id]->GetButton(); }
	GamePadButton		GetGamePadButtonUp(UINT id)		{ return game_pad[id]->GetButtonUp(); }
	GamePadButton		GetGamePadButtonDown(UINT id)	{ return game_pad[id]->GetButtonDown(); }
	EnumBlockRotation	GetBlockRotation(UINT id)		{ return controllers[id].block_rotation; }

	GamePadButton		GetGamePadButtonBarrage(UINT id)
	{ 
		if(controllers[id].barrage_timer > BARRAGE_TIME_MOVE)
		{
			controllers[id].barrage_timer -= BARRAGE_TIME_MOVE;
			return game_pad[id]->GetButton();
		}
		else
		{
			return (GamePadButton)0;
		}
	}

private:
	// private:�ϐ�
	std::array<GameController, MAX_PLAYER> controllers;
};

#endif // __GAMESYSTEM_INPUT_H__