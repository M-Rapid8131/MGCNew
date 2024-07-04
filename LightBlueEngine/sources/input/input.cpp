// ""�C���N���[�h
// LightBlueEngine
#include "input/input.h"

//-------------------------------------------
// GamePad �����o�֐�
//-------------------------------------------

// ����������
void Input::Initialize(HWND hwnd, UINT/*�p����Ŏg�p*/)
{
	mouse.Initialize(hwnd);
}

void Input::AddGamePad(UINT player_id)
{
	game_pad[player_id] = std::make_unique<GamePad>();
	game_pad[player_id]->SetSlot(player_id);

	players++;
}

void Input::DeleteGamePad(UINT player_id)
{
	auto pad_ptr = game_pad[player_id].release();

	if (pad_ptr)
		delete pad_ptr;

	game_pad[player_id].reset();

	players--;
}

// �X�V����
void Input::Update(float/*�p����Ŏg�p*/)
{
	mouse.Update();

	for (UINT id = 0; id < players; id++)
		game_pad[id]->Update();
}

// ImGui�\��
void Input::DebugGUI()
{
	mouse.DebugGUI();

	for (UINT id = 0; id < players; id++)
		game_pad[id]->DebugGUI();
}