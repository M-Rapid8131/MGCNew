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

		if (controller.rotate_fail)
		{
			controller.rotate_fail_timer += elapsed_time;
			if (controller.rotate_fail_timer > FLIP_LIMIT)
			{
				controller.rotate_fail = false;
				controller.rotate_fail_timer = 0.0f;
			}
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

void GamesystemInput::RotationFail(UINT id, EnumBlockRotation old_rotation)
{
	controllers[id].block_rotation = old_rotation;
	controllers[id].rotate_fail = true;
}

// ブロックを180度回転
bool GamesystemInput::BlockFlip(UINT id)
{
	GameController& controller = controllers[id];
	if(controller.rotate_fail)
	{
		controller.rotate_fail			= false;
		controller.rotate_fail_timer	= 0.0f;
		return true;
	}
	return false;
}

// ブロック回転
bool GamesystemInput::BlockRotate(UINT id, GamePadButton input)
{
	GameController&		controller	= controllers[id];
	EnumBlockRotation&	rotation	= controller.block_rotation;

	bool flip_succeed = false;
	switch (rotation)
	{
	case EnumBlockRotation::RIGHT:
		if (input & BTN_A)
			rotation = EnumBlockRotation::TOP;
		else if (input & BTN_B)
			rotation = EnumBlockRotation::UNDER;
		break;

	case EnumBlockRotation::UNDER:
		if(BlockFlip(id))
		{
			rotation = EnumBlockRotation::TOP;
			flip_succeed = true;
		}
		else if (input & BTN_A)
			rotation = EnumBlockRotation::RIGHT;
		else if (input & BTN_B)
			rotation = EnumBlockRotation::LEFT;
		break;

	case EnumBlockRotation::LEFT:
		if (input & BTN_A)
			rotation = EnumBlockRotation::UNDER;
		else if (input & BTN_B)
			rotation = EnumBlockRotation::TOP;
		break;

	case EnumBlockRotation::TOP:
		if (BlockFlip(id))
		{
			rotation = EnumBlockRotation::UNDER;
			flip_succeed = true;
		}
		else if (input & BTN_A)
			rotation = EnumBlockRotation::LEFT;
		else if (input & BTN_B)
			rotation = EnumBlockRotation::RIGHT;
		break;

	default:
		break;
	}

	return flip_succeed;
}