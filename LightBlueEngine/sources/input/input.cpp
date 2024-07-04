// ""インクルード
// LightBlueEngine
#include "input/input.h"

//-------------------------------------------
// GamePad メンバ関数
//-------------------------------------------

// 初期化処理
void Input::Initialize(HWND hwnd, UINT/*継承先で使用*/)
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

// 更新処理
void Input::Update(float/*継承先で使用*/)
{
	mouse.Update();

	for (UINT id = 0; id < players; id++)
		game_pad[id]->Update();
}

// ImGui表示
void Input::DebugGUI()
{
	mouse.DebugGUI();

	for (UINT id = 0; id < players; id++)
		game_pad[id]->DebugGUI();
}