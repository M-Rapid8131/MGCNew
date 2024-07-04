// <>インクルード
#include <imgui.h>

// ""インクルード
// LightBlueEngine
#include "audio/audio_manager.h"
#include "gamesystem_director.h"
#include "gamesystem_input.h"

//--------------------------------------
// GamesystemInput メンバ関数
//--------------------------------------

// コンストラクタ
void GamesystemInput::Initialize(HWND, UINT set_players)
{
	players = set_players;
	for (UINT id = 0; id < set_players; id++)
	{
		AddGamePad(id);
		controllers[id].block_rotation = EnumBlockRotation::RIGHT;
	}
}

// 更新処理
void GamesystemInput::Update(float elapsed_time)
{
	mouse.Update();

	for (UINT id = 0; id < MAX_PLAYER; id++)
	{
		GameController& controller = controllers[id];

		const GamePadButton& BUTTON		= GetGamePadButton(id);
		const GamePadButton& BUTTON_UP	= GetGamePadButtonUp(id);

		game_pad[id]->Update();

		if ((BUTTON & BTN_LEFT) || (BUTTON & BTN_RIGHT))
		{
			controller.hold_timer += elapsed_time;
			if (ActivateMoveContinuous(id))
				controller.barrage_timer += elapsed_time;
		}

		if ((BUTTON_UP & BTN_LEFT) || (BUTTON_UP & BTN_RIGHT))
		{
			controller.hold_timer		= 0.0f;
			controller.barrage_timer	= 0.0f;
		}
	}
}

// ImGui表示
void GamesystemInput::DebugGUI()
{
	for (UINT id = 0; id < MAX_PLAYER; id++)
	{
		GameController& controller = controllers[id];

		std::string block_rotation_str_a = { "BlockRotation : " };
		auto block_rotation_str_b = magic_enum::enum_name(controller.block_rotation);
		block_rotation_str_a.append(block_rotation_str_b.data());

		std::string header_title = { "GamesystemInput Player" + std::to_string(id)};

		if (ImGui::CollapsingHeader(header_title.c_str()))
		{
			ImGui::Text(block_rotation_str_a.c_str());
			ImGui::InputFloat("Hold Timer", &controller.hold_timer);

			game_pad[id]->DebugGUI();
		}
	}
}

// ブロックを180度回転
bool GamesystemInput::BlockFlip()
{
	return false;
}

// ブロック回転
void GamesystemInput::BlockRotate(UINT id, GamePadButton input)
{
	EnumBlockRotation& rotation = controllers[id].block_rotation;
	switch (rotation)
	{
	case EnumBlockRotation::RIGHT:
		if (input & BTN_A)
			rotation = EnumBlockRotation::UNDER;
		else if (input & BTN_B)
			rotation = EnumBlockRotation::TOP;
		break;

	case EnumBlockRotation::UNDER:
		if (input & BTN_A)
			rotation = EnumBlockRotation::LEFT;
		else if(BlockFlip())
			rotation = EnumBlockRotation::TOP;
		else if (input & BTN_B)
			rotation = EnumBlockRotation::RIGHT;
		break;

	case EnumBlockRotation::LEFT:
		if (input & BTN_A)
			rotation = EnumBlockRotation::TOP;
		else if (input & BTN_B)
			rotation = EnumBlockRotation::UNDER;
		break;

	case EnumBlockRotation::TOP:
		if (input & BTN_A)
			rotation = EnumBlockRotation::RIGHT;
		else if (BlockFlip())
			rotation = EnumBlockRotation::UNDER;
		else if (input & BTN_B)
			rotation = EnumBlockRotation::LEFT;
		break;

	default:
		break;
	}
}