// <>�C���N���[�h
#include <imgui.h>

// ""�C���N���[�h
// LightBlueEngine
#include "game_system/gamesystem_director.h"
#include "game_system/gamesystem_input.h"
#include "UI/value_UI.h"
#include "json_editor.h"
#include "xmfloat_calclation.h"

// �Q�[���\�[�X�t�@�C��
#include "object_board.h"

// enum class
enum class EnumUISpriteIndex
{
	LEFT,
	RIGHT,
};

enum class EnumValueUIIndex
{
	TIME,
	SCORE,
	SPEED_LEVEL,
	CHAIN,
	DELETED_BLOCKS,

	VALUE_UI_INDEX_NUM
};
using enum EnumValueUIIndex;

enum class EnumSpriteUIIndex
{
	NEXT_BLOCK,
	GAME_MODE,

	SPRITE_UI_INDEX_NUM
};
using enum EnumSpriteUIIndex;

// �萔
const UINT				TEXT_SIZE				= 256;
const float				UI_SHAKE_FACTOR			= 10.0f;
const float				CONVERT_COLOR_FACTOR	= 255.0f;
const DirectX::XMFLOAT2 TEXT_RIGHT_ALIGN_POS	= { 200.0f, 50.0f };
const DirectX::XMFLOAT2 DEFAULT_UI_SCALE		= { 0.1f,	0.1f };
const DirectX::XMFLOAT2 SPRITE_UI_SCALE			= { 0.2f,	0.2f };
const DirectX::XMFLOAT2 UI_SPRITE_CENTER_POS	= { 70.0f,	60.0f };
const DirectX::XMFLOAT2 UI_SPRITE_SIZE			= { 60.0f,	60.0f };
const DirectX::XMFLOAT2 UI_GAME_MODE_SIZE		= { 250.0f, 50.0f };

const float UI_LEFT_X		= SCast(float, Graphics::GetInstance()->GetScreenWidth()) * 0.19f;
const float UI_RIGHT_X		= SCast(float, Graphics::GetInstance()->GetScreenWidth()) * 0.7f;
const float UI_NEXT_BLOCK_Y = SCast(float, Graphics::GetInstance()->GetScreenHeight()) * 0.05f;
const float UI_TOP_Y		= SCast(float, Graphics::GetInstance()->GetScreenHeight()) * 0.5f;
const float UI_CENTER_Y		= SCast(float, Graphics::GetInstance()->GetScreenHeight()) * 0.65f;
const float UI_UNDER_Y		= SCast(float, Graphics::GetInstance()->GetScreenHeight()) * 0.8f;

//---------------------------------
// ObjectBoard �����o�֐�
//---------------------------------
// ���l�FUI�֌W�͍���ʃN���X�Ɉړ�����\��
const DirectX::XMFLOAT3 ObjectBoard::INIT_POSITION =
{
	(ObjectBlock::BLOCK_SIZE * MAX_ROW * 0.5f) + 1.0f,
	(ObjectBlock::BLOCK_SIZE * MAX_COLUMN * -0.5f) - 1.5f
		,0.5f
};

// �u���b�N����J�n
void ObjectBoard::NextBlock::Start()
{
	moving_block = std::move(next_block.front());
	next_block.pop_front();
	EnumBlockColor left_color = next_block.front().LEFT_BLOCK->GetBlockColor();
	EnumBlockColor right_color = next_block.front().RIGHT_BLOCK->GetBlockColor();
	obj->sprite_ui.at(SCast(size_t, NEXT_BLOCK))->SetIndexValue(SCast(size_t, EnumUISpriteIndex::LEFT), SCast(size_t, left_color));
	obj->sprite_ui.at(SCast(size_t, NEXT_BLOCK))->SetIndexValue(SCast(size_t, EnumUISpriteIndex::RIGHT), SCast(size_t, right_color));

	moving_block.LEFT_BLOCK->GetBlockState().TransitionMoveState(EnumBlockPlace::LEFT);
	moving_block.RIGHT_BLOCK->GetBlockState().TransitionMoveState(EnumBlockPlace::RIGHT);
}

// �X�V����
void ObjectBoard::NextBlock::Update(float elapsed_time)
{
	// �v���C���[ID���g���ăR���g���[���[���擾
	const UINT			ID					= obj->player_id;
	GamesystemInput*	gamesystem_input	= GamesystemInput::GetInstance();
	GamePad*			pad					= gamesystem_input->GetGamePad(ID);

	if (obj->board_state.state == EnumBoardState::MOVING)
	{
		GamePadButton input = pad->GetButtonDown();
		GamePadButton hold = pad->GetButton();
		GamePadButton barrage = gamesystem_input->GetGamePadButtonBarrage(ID);
		if ((input & BTN_A) || (input & BTN_B))
		{
			gamesystem_input->BlockRotate(ID, input);
			moving_block.RIGHT_BLOCK->RotateBlock(moving_block.LEFT_BLOCK.get(), gamesystem_input->GetBlockRotation(ID));
		}
		else
		{
			if (gamesystem_input->ActivateMoveContinuous(ID))
				obj->SignalMove(barrage);
			else
				obj->SignalMove(input);
		}

		if (hold & BTN_DOWN)
			obj->current_speed += 0.75f;
	}

	if (moving_block.LEFT_BLOCK)
		moving_block.LEFT_BLOCK->Update(elapsed_time);
	else
		stand_switch.first = false;

	if (moving_block.RIGHT_BLOCK)
	{
		moving_block.RIGHT_BLOCK->Update(elapsed_time);
		moving_block.RIGHT_BLOCK->FollowRootBlock(gamesystem_input->GetBlockRotation(ID),
			moving_block.LEFT_BLOCK->GetBlockCell());
	}
	else
		stand_switch.second = false;
}

// �u���b�N�`�揈��
void ObjectBoard::NextBlock::Render()
{
	if (moving_block.LEFT_BLOCK)
		moving_block.LEFT_BLOCK->Render();

	if (moving_block.RIGHT_BLOCK)
		moving_block.RIGHT_BLOCK->Render();
}

// �G�~�b�V�u������`��
void ObjectBoard::NextBlock::EmissiveRender()
{
	if (moving_block.LEFT_BLOCK)
		moving_block.LEFT_BLOCK->EmissiveRender();

	if (moving_block.RIGHT_BLOCK)
		moving_block.RIGHT_BLOCK->EmissiveRender();
}

// NEXT�u���b�N�𓊓�
void ObjectBoard::NextBlock::SetNewBlock()
{
	auto left	= std::make_unique<ObjectBlock>(false, obj);
	auto right	= std::make_unique<ObjectBlock>(false, obj);
	next_block.push_back(std::make_pair(std::move(left), std::move(right)));
}

//---------------------------------------------------------------------------------------
//		BoardState Transition�n
//---------------------------------------------------------------------------------------

// �Q�[���J�n�O
void ObjectBoard::BoardState::TransitionStandbyState()
{
	// ��ԕύX�㏈��
	state = EnumBoardState::STANDBY;
	obj->count_down_time = 4.0f - FLT_EPSILON;

	float& disolve_factor = obj->board_model->GetDisolveFactor();
	disolve_factor = 1.0f;

	obj->state_update = &ObjectBoard::UpdateStandbyState;
}

// �Q�[���J�n
void ObjectBoard::BoardState::TransitionStartState(int game_mode_id)
{
	AudioManager*	audio_manager	= GamesystemDirector::GetInstance()->GetAudioManager();
	JSONEditor*		json_editor		= JSONEditor::GetInstance();

	// ��ԕύX�㏈��
	state = EnumBoardState::START;
	obj->state_update = &ObjectBoard::UpdateStartState;

	std::string mode_name(magic_enum::enum_name(SCast(EnumGameMode, game_mode_id)).data());

	Parameters	mode_params;
	ParamPtr	mode_setting_params;

	std::filesystem::path json_path("resources/json_data/mode_data.json");
	json_editor->ImportJSON(json_path, &mode_params);
	mode_setting_params = GET_PARAMETER_IN_PARAMPTR(mode_name, Parameters, &mode_params);

	obj->init_level				= SCast(UINT, *(GET_PARAMETER_IN_PARAMPTR("InitLevel", int, mode_setting_params)));
	obj->max_level				= SCast(UINT, *(GET_PARAMETER_IN_PARAMPTR("MaxLevel", int, mode_setting_params)));
	obj->speed_increase_factor	= *(GET_PARAMETER_IN_PARAMPTR("SpeedIncrease", float, mode_setting_params));
	obj->stand_decrease_factor	= *(GET_PARAMETER_IN_PARAMPTR("StandDecrease", float, mode_setting_params));
	
	audio_manager->StopBGM();
	audio_manager->PlayVoice(EnumVoiceBank::COUNT_DOWN);

	obj->value_ui.at(SCast(size_t, SPEED_LEVEL))->SetIntValue(0, obj->init_level, 0.0f);
	obj->flag_system.SetFlag(EnumBoardFlags::PLAYING, true);
	obj->sprite_ui.at(SCast(size_t, GAME_MODE))->SetIndexValue(0, game_mode_id);

	obj->speed_level = obj->init_level;
}

// �u���b�N�ړ��J�n
void ObjectBoard::BoardState::TransitionDropStartState()
{
	GamesystemInput* gamesystem_input = GamesystemInput::GetInstance();

	// ��ԕύX�㏈��
	state = EnumBoardState::DROP_START;

	obj->ClearDeleteBlockList();

	obj->state_update	= nullptr;
	obj->standing_time	= 0.0f;
	obj->chain			= 0;
	obj->value_ui.at(SCast(size_t, CHAIN))->SetIntValue(0, obj->chain, 0.0f);

	for (UINT row = 0; row < MAX_ROW; row++)
	{
		UINT column = MAX_COLUMN - 1;
		while (obj->existing_matrix[row][column])
		{
			column--;
		}
		obj->stand_collision_heignt[row] = column;
	}

	gamesystem_input->ResetRotate(obj->player_id);
	obj->flag_system.SetFlag(EnumBoardFlags::BLOCK_REGISTERED, false);

	obj->next_block.Start();
	obj->next_block.SetNewBlock();

	TransitionMovingState();
}

// �u���b�N�ړ��J�n
void ObjectBoard::BoardState::TransitionMovingState()
{
	// ��ԕύX�㏈��
	state = EnumBoardState::MOVING;
	obj->state_update = &ObjectBoard::UpdateMovingState;
	obj->flag_system.SetFlag(EnumBoardFlags::CAN_MOVE, true);
}

// �u���b�N�����J�n
void ObjectBoard::BoardState::TransitionDroppingState()
{
	// ��ԕύX�㏈��
	state = EnumBoardState::DROPPING;
	obj->state_update = &ObjectBoard::UpdateDroppingState;
	obj->standing_time = 0.0f;

	obj->current_speed = 0.0f;
}

// ���n
void ObjectBoard::BoardState::TransitionLandingState()
{
	// ��ԕύX�㏈��
	state = EnumBoardState::LANDING;
	obj->state_update = &ObjectBoard::UpdateLandingState;
	obj->standing_time = 0.0f;

	for (UINT row = 1; row <= MAX_ROW; row++)
	{
		obj->VerticalLineCheck(row);
	}

	for (UINT column = 1; column <= MAX_COLUMN; column++)
	{
		obj->HorizontalLineCheck(column);
	}

	obj->flag_system.SetFlag(EnumBoardFlags::RESULT, obj->MoveToDeletedBlockList());

	auto call_back = [&](ID3D11PixelShader* accumulate_ps) {
		for (const auto& delete_block_list : obj->delete_block_sorter.color_block)
		{
			for (const auto& deleted_block : delete_block_list)
			{
				deleted_block->AccumulateBlockParticle(accumulate_ps);
			}
		}
		};
	obj->erase_block_particle->AccumulateParticles(call_back);

}

// �Q�[���I�[�o�[
void ObjectBoard::BoardState::TransitionGameOverState(bool cleared)
{
	state = EnumBoardState::GAME_OVER;
	obj->state_update = &ObjectBoard::UpdateGameOverState;

	for (const auto& block : obj->block_list)
	{
		block->GetBlockState().TransitionEraseState();
	}

	obj->MoveToDeletedBlockList();

	for (int row = 0; row < MAX_ROW; row++)
	{
		for (int column = MAX_COLUMN - 1; column >= 0; column--)
		{
			obj->existing_matrix[row][column] = false;
		}
	}

	obj->AccumulateBoardParticle();

	obj->flag_system.SetFlag(EnumBoardFlags::PLAYING, false);
}

//---------------------------------------------------------------------------------------
//		State��Update�n
//---------------------------------------------------------------------------------------

// �Q�[���J�n�O
void ObjectBoard::UpdateStandbyState(float elapsed_time)
{
	GamesystemInput* gamesystem_input = GamesystemInput::GetInstance();

	if (gamesystem_input->GetGamePadButtonDown(player_id) & BTN_START)
	{
		if (gamesystem_input->GetGamePadButton(player_id) & BTN_Y)
			GameStart(SCast(int, EnumGameMode::IMPACT));
		else if (gamesystem_input->GetGamePadButton(player_id) & BTN_X)
			GameStart(SCast(int, EnumGameMode::ULTIMATE));
		else if (gamesystem_input->GetGamePadButton(player_id) & BTN_A)
			GameStart(SCast(int, EnumGameMode::ENDLESS));
		else
			GameStart(SCast(int, EnumGameMode::NORMAL));
	}
}

void ObjectBoard::UpdateStartState(float elapsed_time)
{
	AudioManager* audio_manarger = GamesystemDirector::GetInstance()->GetAudioManager();
	count_down_time -= elapsed_time;
	count_down_se_time += elapsed_time;
	if (count_down_time > 0.0f)
	{
		if (count_down_se_time > count_down_se_span)
		{
			if (count_down_time > 1.0f)
				audio_manarger->PlaySE(EnumSEBank::COUNT_DOWN);
			else
				audio_manarger->PlaySE(EnumSEBank::GAME_START);

			count_down_se_time -= count_down_se_span;
		}
	}
	else if (count_down_time < 0.0f)
	{
		audio_manarger->PlayBGM(SCast(EnumBGMBank, init_level / 10));
		board_state.TransitionDropStartState();
	}
}

// �u���b�N�ړ����̏���
void ObjectBoard::UpdateMovingState(float elapsed_time)
{
	GamesystemInput* gamesystem_input = GamesystemInput::GetInstance();

	CalcBlockSpeed(elapsed_time);

	object_time += elapsed_time;

	// ���E�ǂ��炩�����Î~��Ԃł���Ƃ�
	if (next_block.moving_block.LEFT_BLOCK->GetBlockState().state == EnumBlockState::STAND
		|| next_block.moving_block.RIGHT_BLOCK->GetBlockState().state == EnumBlockState::STAND)
	{
		// ������Î~��Ԃɂ��āA�Î~���ԃ^�C�}�[���쓮������
		next_block.moving_block.LEFT_BLOCK->SetStanding();
		next_block.moving_block.RIGHT_BLOCK->SetStanding();
		standing_time += elapsed_time;
		if (standing_time > standing_time_limit || (gamesystem_input->GetGamePadButton(player_id) & BTN_DOWN))
		{
			board_state.TransitionDroppingState();
			return;
		}

		BlockCell under_cell_l = BlockCell(next_block.moving_block.LEFT_BLOCK->GetBlockCell().row, next_block.moving_block.LEFT_BLOCK->GetBlockCell().column + 1);
		BlockCell under_cell_r = BlockCell(next_block.moving_block.RIGHT_BLOCK->GetBlockCell().row, next_block.moving_block.RIGHT_BLOCK->GetBlockCell().column + 1);

		bool result = false;

		// ��]���łȂ��u���b�N�Ƃ̈ʒu�֌W�ŁA�����Ă��邩�̔���Ɏg���u���b�N��ς��Ă���
		switch (gamesystem_input->GetBlockRotation(player_id))
		{
		case EnumBlockRotation::RIGHT:
		case EnumBlockRotation::LEFT:
			result = !(IsExistingBlockFromCell(under_cell_l) || IsExistingBlockFromCell(under_cell_r));
			break;

		case EnumBlockRotation::UNDER:
			result = !IsExistingBlockFromCell(under_cell_r);
			break;

		case EnumBlockRotation::TOP:
			result = !IsExistingBlockFromCell(under_cell_l);
			break;
		}

		if (result)
		{
			next_block.moving_block.LEFT_BLOCK->ResetStanding();
			next_block.moving_block.RIGHT_BLOCK->ResetStanding();
		}
	}
}

// �u���b�N�������̏���
void ObjectBoard::UpdateDroppingState(float elapsed_time)
{
	const float MAX_SPEED = 0.3f;

	if (current_speed < MAX_SPEED)
		current_speed += elapsed_time;
	else
		current_speed = MAX_SPEED;

	object_time += elapsed_time;

	if (!flag_system.GetFlag(EnumBoardFlags::BLOCK_REGISTERED))
	{
		RegisterBlock();
		flag_system.SetFlag(EnumBoardFlags::BLOCK_REGISTERED, true);
	}

	for (const auto& block : block_list)
	{
		if (block->GetBlockState().state != EnumBlockState::PUT)
			return;
	}
	board_state.TransitionLandingState();
}

// �u���b�N�����ׂĒ��n�������̏���
void ObjectBoard::UpdateLandingState(float elapsed_time)
{
	object_time += elapsed_time;
	current_speed = 0.0f;

	float chain_wait_time = waiting_time_limit * (1.0f - SCast(float, chain) * 0.2f);

	if (waiting_time > chain_wait_time)
	{
		waiting_time = 0.0f;
	}
	else
	{
		waiting_time += elapsed_time;
		return;
	}

	if (flag_system.GetFlag(EnumBoardFlags::RESULT))
	{
		board_state.TransitionDroppingState();
	}
	else
	{
		if (CheckGameOver())
		{
			board_state.TransitionGameOverState(false);
			return;
		}

		for (auto& block : block_list)
		{
			block->GetBlockState().TransitionPutState();
		}

		board_state.TransitionDropStartState();
	}
}

// �Q�[���I�[�o�[����
void ObjectBoard::UpdateGameOverState(float elapsed_time)
{
	AudioManager* audio_manager = GamesystemDirector::GetInstance()->GetAudioManager();

	audio_manager->StopBGM();

	game_over_time += elapsed_time;

	float& disolve_factor = board_model->GetDisolveFactor();
	disolve_factor -= 0.02f;

	if (game_over_time > 5.0f)
	{
		game_over_time = 0.0f;
		board_state.TransitionStandbyState();
	}
}

// �u���b�N���ړ�������֐�
void ObjectBoard::SignalMove(GamePadButton input)
{
	GamesystemInput* gamesystem_input = GamesystemInput::GetInstance();

	bool		result = false;
	BlockCell	left_cell_l = BlockCell(next_block.moving_block.LEFT_BLOCK->GetBlockCell().row - 1, next_block.moving_block.LEFT_BLOCK->GetBlockCell().column);
	BlockCell	left_cell_r = BlockCell(next_block.moving_block.RIGHT_BLOCK->GetBlockCell().row - 1, next_block.moving_block.RIGHT_BLOCK->GetBlockCell().column);
	BlockCell	right_cell_l = BlockCell(next_block.moving_block.LEFT_BLOCK->GetBlockCell().row + 1, next_block.moving_block.LEFT_BLOCK->GetBlockCell().column);
	BlockCell	right_cell_r = BlockCell(next_block.moving_block.RIGHT_BLOCK->GetBlockCell().row + 1, next_block.moving_block.RIGHT_BLOCK->GetBlockCell().column);
	if (input & BTN_LEFT)
	{
		switch (gamesystem_input->GetBlockRotation(player_id))
		{
		case EnumBlockRotation::RIGHT:
		case EnumBlockRotation::TOP:
			if (!IsExistingBlockFromCell(left_cell_l))
				result = true;
			break;
		case EnumBlockRotation::UNDER:
		case EnumBlockRotation::LEFT:
			if (!IsExistingBlockFromCell(left_cell_r))
				result = true;
			break;
		}
	}
	else if (input & BTN_RIGHT)
	{
		switch (gamesystem_input->GetBlockRotation(player_id))
		{
		case EnumBlockRotation::RIGHT:
		case EnumBlockRotation::UNDER:
			if (!IsExistingBlockFromCell(right_cell_r))
				result = true;
			break;
		case EnumBlockRotation::LEFT:
		case EnumBlockRotation::TOP:
			if (!IsExistingBlockFromCell(right_cell_l))
				result = true;
			break;
		}
	}
	if (result)
	{
		next_block.moving_block.LEFT_BLOCK->MoveBlock(input);
		next_block.moving_block.RIGHT_BLOCK->MoveBlock(input);
	}
}

// �R���X�g���N�^
ObjectBoard::ObjectBoard(UINT player_id)
	: player_id(player_id)
{
	translation = { 0.0f,0.0f,0.0f };
	rotation.y = DirectX::XM_PIDIV2;
	board_model = std::make_unique<GameModel>("resources/model/board/board.gltf");
	board_model->SetMaskTexture(L"resources/sprite/mask_texture.png");

	ParticleSystem::CbParticleEmitter	cb_emitter;
	cb_emitter.emit_amounts = 1000;

	board_particle = std::make_unique<ParticleSystem>(cb_emitter, true, "accumulate_particle_ps.cso");
	
	cb_emitter.emit_amounts = 100000;
	cb_emitter.life_time = 0.3f;
	cb_emitter.emit_size = 0.2f;
	erase_block_particle = std::make_unique<ParticleSystem>(cb_emitter, true, "accumulate_particle_ps.cso");

	existing_matrix.resize(MAX_ROW);
	for (int x = 0; x < MAX_ROW; x++)
	{
		existing_matrix.at(x).resize(MAX_COLUMN);
	}

	level_up_block = LV_UP_BLOCK_COUNT;

	board_state.obj = this;

	next_block.obj = this;

	auto& ui_next_block = sprite_ui.emplace_back(std::make_unique<SpriteUI>());
	ui_next_block->Initialize(L"resources/sprite/UI/UI_base_next.png", L"NEXT_BLOCK", { UI_LEFT_X, UI_NEXT_BLOCK_Y });
	ui_next_block->PushIndexValue(0);
	ui_next_block->PushIndexValue(0);

	const wchar_t* w_next_block_filename[] = {
		L"resources/sprite/next_block/red.png",
		L"resources/sprite/next_block/blue.png",
		L"resources/sprite/next_block/green.png",
		L"resources/sprite/next_block/yellow.png",
		L"resources/sprite/next_block/purple.png",
	};

	for (const wchar_t* w_filename : w_next_block_filename)
	{
		ui_next_block->PushSpriteImage(w_filename, UI_SPRITE_SIZE);
	}

	ui_next_block->PushImagePosition(XMFloatCalclation::XMFloat2Subtract(UI_SPRITE_CENTER_POS, { UI_SPRITE_SIZE.x * 0.5f, 0.0f }));
	ui_next_block->PushImagePosition(XMFloatCalclation::XMFloat2Add(UI_SPRITE_CENTER_POS, { UI_SPRITE_SIZE.x * 0.5f, 0.0f }));
	ui_next_block->SetUIScale(SPRITE_UI_SCALE);

	next_block.SetNewBlock();
	next_block.SetNewBlock();
	next_block.SetNewBlock();

	auto& ui_game_mode = sprite_ui.emplace_back(std::make_unique<SpriteUI>());
	ui_game_mode->Initialize(L"", L"GAME_MODE", { UI_RIGHT_X, UI_TOP_Y });
	ui_game_mode->PushIndexValue(0);

	const wchar_t* w_game_mode_filename[] = {
		L"resources/sprite/mode/normal.png",
		L"resources/sprite/mode/impact.png",
		L"resources/sprite/mode/endless.png",
		L"resources/sprite/mode/mission.png",
		L"resources/sprite/mode/ultimate.png",
	};

	for (const wchar_t* w_filename : w_game_mode_filename)
	{
		ui_game_mode->PushSpriteImage(w_filename, UI_GAME_MODE_SIZE);
	}

	ui_game_mode->PushImagePosition({ 0.0f, UI_GAME_MODE_SIZE.y * 0.5f });
	ui_game_mode->SetUIScale({ 1.0f, 1.0f });

	auto& ui_time = value_ui.emplace_back(std::make_unique<ValueUI>());
	ui_time->Initialize(L"resources/sprite/UI/UI_time.png", L"TIME", { UI_RIGHT_X, UI_CENTER_Y });
	ui_time->PushIntValue(SCast(int, object_time), TEXT_RIGHT_ALIGN_POS);
	ui_time->SetUIScale(DEFAULT_UI_SCALE);

	auto& ui_score = value_ui.emplace_back(std::make_unique<ValueUI>());
	ui_score->Initialize(L"resources/sprite/UI/UI_score.png", L"SCORE", { UI_RIGHT_X, UI_UNDER_Y });
	ui_score->PushIntValue(score, TEXT_RIGHT_ALIGN_POS);
	ui_score->SetUIScale(DEFAULT_UI_SCALE);

	auto& ui_speed_level = value_ui.emplace_back(std::make_unique<ValueUI>());
	ui_speed_level->Initialize(L"resources/sprite/UI/UI_speed_lv.png", L"SPEED_LEVEL", { UI_LEFT_X, UI_CENTER_Y });
	ui_speed_level->PushIntValue(speed_level, TEXT_RIGHT_ALIGN_POS);
	ui_speed_level->SetUIScale(DEFAULT_UI_SCALE);

	auto& ui_chain = value_ui.emplace_back(std::make_unique<ValueUI>());
	ui_chain->Initialize(L"resources/sprite/UI/UI_chain.png", L"CHAIN", { UI_LEFT_X, UI_TOP_Y });
	ui_chain->PushIntValue(chain, TEXT_RIGHT_ALIGN_POS);
	ui_chain->SetUIScale(DEFAULT_UI_SCALE);

	auto& ui_deleted_blocks = value_ui.emplace_back(std::make_unique<ValueUI>());
	ui_deleted_blocks->Initialize(L"resources/sprite/UI/UI_deleted_blocks.png", L"DELETED_BLOCKS", { UI_LEFT_X, UI_UNDER_Y });
	ui_deleted_blocks->PushIntValue(deleted_block_count, TEXT_RIGHT_ALIGN_POS);
	ui_deleted_blocks->SetUIScale(DEFAULT_UI_SCALE);

	delete_block_sorter.color_block.resize(6);

	board_state.TransitionStandbyState();
}

// �f�X�g���N�^
ObjectBoard::~ObjectBoard()
{
}

// �X�V����
void ObjectBoard::Update(float elapsed_time)
{
	GamesystemInput* gamesystem_input = GamesystemInput::GetInstance();
	const GamePad* game_pad = gamesystem_input->GetGamePad(player_id);
	AudioManager* audio_manager = GamesystemDirector::GetInstance()->GetAudioManager();

	// state�ʃA�b�v�f�[�g
	if (state_update)
		(this->*state_update)(elapsed_time);

	// �J�����ړ�
	if (game_pad->GetAxisLY() <= -0.01f || game_pad->GetAxisLY() >= +0.01f)
		CameraZoom(elapsed_time);

	if ((game_pad->GetAxisRX() <= -0.01f || game_pad->GetAxisRX() >= +0.01f) ||
		(game_pad->GetAxisRY() <= -0.01f || game_pad->GetAxisRY() >= +0.01f))
		CameraMove(elapsed_time);

	// �h�炵����
	DirectX::XMFLOAT3 root_translation = translation;
	shake_position = XMFloatCalclation::XMFloat3Add(shake_position, shake_speed);
	translation = XMFloatCalclation::XMFloat3Subtract(translation, shake_position);

	// transform�X�V
	UpdateTransform();

	// translation�����ɖ߂�
	translation = root_translation;
	shake_speed = XMFloatCalclation::XMFloat3Lerp(shake_speed, { 0.0f,0.0f,0.0f }, 0.3f);
	shake_position = XMFloatCalclation::XMFloat3Lerp(shake_position, { 0.0f,0.0f,0.0f }, 0.03f);

	// block�̍X�V����
	for (const auto& block : block_list)
	{
		block->Update(elapsed_time);
	}

	// �������o�p�̍X�V����
	for (const auto& delete_block_list : delete_block_sorter.color_block)
	{
		for (const auto& deleted_block : delete_block_list)
		{
			deleted_block->ErasingUpdate(elapsed_time);
		}
	}

	// �p�[�e�B�N���̍X�V����
	for (const auto& particle : block_particle)
	{
		particle->Update(elapsed_time);
	}

	next_block.Update(elapsed_time);

	// ���x���A�b�v����
	if (deleted_block_count >= level_up_block)
	{
		audio_manager->PlayVoice(EnumVoiceBank::LEVEL_UP);
		audio_manager->PlaySE(EnumSEBank::LEVEL_UP);
		if (speed_level == max_level)
			board_state.TransitionGameOverState(true);
		else
		{
			speed_level++;
			value_ui.at(SCast(size_t, SPEED_LEVEL))->SetIntValue(0, speed_level);
			level_up_block += LV_UP_BLOCK_COUNT;
		}
		if (speed_level % 10 == 1 && speed_level < 70)	// 11,21,31,41,51,61��BGM�ω�
		{
			audio_manager->StopBGM();
			audio_manager->PlayBGM(SCast(EnumBGMBank, speed_level / 10));
		}

		standing_time_limit -= stand_decrease_factor;
	}

	int game_second = SCast(int, object_time) % 60;
	int game_minute = SCast(int, object_time) / 60;
	value_ui.at(SCast(size_t, EnumValueUIIndex::TIME))->SetIntValue(0, game_second + game_minute * 100, 0.0f);

	UIUpdate(elapsed_time);

	board_model->InstanceUpdate();

	erase_block_particle->Update(elapsed_time, Graphics::GetInstance()->GetComputeShader(EnumComputeShader::BLOCK).Get());

	if (gamesystem_input->GetGamePadButtonDown(player_id) & BTN_BACK)
		board_state.TransitionGameOverState(false);
}

// UI�̍X�V����
void ObjectBoard::UIUpdate(float elapsed_time)
{
	DirectX::XMFLOAT2 shake_2d;
	shake_2d.x = shake_position.x * -UI_SHAKE_FACTOR;
	shake_2d.y = shake_position.y * UI_SHAKE_FACTOR;

	for (auto& ui : value_ui)
	{
		ui->SetShakePosition(shake_2d);
		ui->Update(elapsed_time);
	}

	for (auto& ui : sprite_ui)
	{
		ui->SetShakePosition(shake_2d);
	}
}

// ImGui�\��
void ObjectBoard::DebugGUI()
{
	if (ImGui::CollapsingHeader("ObjectBoard"))
	{
		ImGui::InputFloat("Current Speed", &current_speed);
		ImGui::InputFloat("Rigidity Time", &standing_time);
		ImGui::InputFloat("Rigidity Time Limit", &standing_time_limit);

		void* vp_level = &speed_level;
		if (ImGui::InputScalar("Speed Level", ImGuiDataType_U16, vp_level))
		{
			if (speed_level < init_level)			speed_level = init_level;
			else if (speed_level > max_level)		speed_level = max_level;
			else									speed_level = speed_level;
		}

		int id = SCast(int, player_id);
		ImGui::InputInt("Player Id", &id);

		for (int i = 0; i < delete_block_sorter.color_block.size(); i++)
		{
			int size = delete_block_sorter.color_block[i].size();
			std::string str_id = "Sorter" + std::to_string(i);
			ImGui::InputInt(str_id.c_str(), &id);
		}

		int erased = SCast(int, deleted_block_count);
		ImGui::InputInt("deleted_block_count", &erased);

		char block_state_str[TEXT_SIZE] = "Board State : ";
		auto state = magic_enum::enum_name(board_state.state);
		strcat_s(block_state_str, sizeof(block_state_str), state.data());
		ImGui::Text(block_state_str);
	}
}

// �`�揈��
void ObjectBoard::Render()
{
	Graphics* graphics = Graphics::GetInstance();

	graphics->SetDepthStencilState(EnumDepthState::ZT_ON_ZW_ON);
	graphics->SetRasterizerState(EnumRasterizerState::SOLID);
	graphics->SetBlendState(EnumBlendState::ALPHA, nullptr, 0xFFFFFFFF);

	board_model->Render(false, transform);

	for (UPtrVector<ObjectBlock>::const_reference block : block_list)
		block->Render();

	for (const auto& delete_block_list : delete_block_sorter.color_block)
	{
		for (const auto& deleted_block : delete_block_list)
		{
			deleted_block->Render();
		}
	}

	erase_block_particle->Render();

	next_block.Render();
}

// �G�~�b�V�u������`��
void ObjectBoard::EmissiveRender()
{
	Graphics* graphics = Graphics::GetInstance();

	board_model->Render(false, transform, graphics->GetPixelShader(EnumPixelShader::EXTRACT_EMISSIVE).Get());

	for (UPtrVector<ObjectBlock>::const_reference block : block_list)
		block->EmissiveRender();

	for (const auto& delete_block_list : delete_block_sorter.color_block)
	{
		for (const auto& deleted_block : delete_block_list)
		{
			deleted_block->Render();
		}
	}

	next_block.EmissiveRender();
}

// UI��`��
void ObjectBoard::UIRender()
{
	float width		= Graphics::GetInstance()->GetScreenWidth() * 0.5f;
	float height	= Graphics::GetInstance()->GetScreenHeight() * 0.5f;

	for (auto const& ui : value_ui)
		ui->Render();

	for (auto const& ui : sprite_ui)
		ui->Render();

	if (board_state.state == EnumBoardState::START)
	{
		NumberRenderer* number_renderer = GamesystemDirector::GetInstance()->GetNumberRenderer();
		number_renderer->RenderInt(SCast(int, count_down_time), { width,height }, { 0.5f,0.5f }, EnumNumberAlignment::CENTER_ALIGNMENT);
	}
}

// ���삵�Ă����u���b�N��Ֆʂɓo�^
void ObjectBoard::RegisterBlock()
{
	next_block.left_cell = next_block.moving_block.LEFT_BLOCK->GetBlockCell();
	next_block.moving_block.LEFT_BLOCK->GetBlockState().TransitionDropState();
	block_list.push_back(std::move(next_block.moving_block.LEFT_BLOCK));
	SetExistingBlockFromCell(next_block.left_cell, true);

	next_block.right_cell = next_block.moving_block.RIGHT_BLOCK->GetBlockCell();
	next_block.moving_block.RIGHT_BLOCK->GetBlockState().TransitionDropState();
	block_list.push_back(std::move(next_block.moving_block.RIGHT_BLOCK));
	SetExistingBlockFromCell(next_block.right_cell, true);
}

// �������X�g��������
void ObjectBoard::ClearDeleteBlockList()
{
	// �u���b�N������
	for (auto& list : delete_block_sorter.color_block)
	{
		int pop_count = 0;
		for (auto& erase_block : list)
		{
			if (erase_block->GetBlockState().state != EnumBlockState::DISAPPEARED)
				continue;

			// ���������u���b�N��reset
			erase_block.reset();

			pop_count++;
		}
		for (int i = 0; i < pop_count; i++)
		{
			list.pop_back();
		}
		list.shrink_to_fit();
	}
}

// �u���b�N���������X�g�ɑ���֐�
bool ObjectBoard::MoveToDeletedBlockList()
{
	int		pop_count = 0; // �u���b�N�폜�z��ɑ�������

	// �����\�t���O�̃��Z�b�g
	delete_block_sorter.AllOff();

	// �폜���X�g�Ɉړ����鏈��
	for (auto& block : block_list)
	{
		size_t color_num = magic_enum::enum_index<EnumBlockColor>(block->GetBlockColor()).value();
		if (block->GetBlockState().state == EnumBlockState::ERASE)
		{
			delete_block_sorter.color_block.at(color_num).push_back(std::move(block));
			block.reset();
			pop_count++;
			if (!delete_block_sorter.erase_flag.at(color_num))
				delete_block_sorter.erase_flag.at(color_num).flip();
		}
	}

	// �ǂ̐F���폜�ł���K�萔�ɖ����Ȃ�������I��
	if (delete_block_sorter.IsAllOff() && board_state.state != EnumBoardState::GAME_OVER)
	{
		for (auto& delete_list : delete_block_sorter.color_block)
		{
			delete_list.clear();
		}
		return false;
	}

	// block_list���̗]�����ł�����茸�炵�Ă�������
	SortInList(block_list);
	for (int i = 0; i < pop_count; i++)
	{
		block_list.pop_back();
	}
	block_list.shrink_to_fit();

	int	num = 0;

	// �X�R�A�v�Z�Ɏg�p
	UINT	poped_block = 0;

	for (auto& list : delete_block_sorter.color_block)
	{
		// ���J�E���g���Z�b�g
		pop_count = 0;

		// �u���b�N����������K�萔�ɖ����Ȃ������Ƃ��̓X�L�b�v
		// (�Q�[���I�[�o�[��������)
		if (board_state.state != EnumBoardState::GAME_OVER)
		{
			if (!(delete_block_sorter.erase_flag.at(num)))
			{
				num++;
				list.clear();
				continue;
			}
		}

		// ��������u���b�N�̌��ƃu���b�N����
		size_t	erase_count = list.size();
		deleted_block_count += board_state.state != EnumBoardState::GAME_OVER ? SCast(UINT, erase_count) : 0;
		poped_block += SCast(UINT, erase_count);
		num++;
	}

	// �u���b�N���������Ƃ��ł����̂ŁA�A�����A�X�R�A�A�������u���b�N�̐������Z����
	// (�Q�[���I�[�o�[��������)
	if (board_state.state != EnumBoardState::GAME_OVER)
	{
		chain++;
		score += CalcScore(poped_block, chain);
		value_ui.at(SCast(size_t, SCORE))->SetIntValue(0, score);
		value_ui.at(SCast(size_t, CHAIN))->SetIntValue(0, chain);
		value_ui.at(SCast(size_t, DELETED_BLOCKS))->SetIntValue(0, deleted_block_count);
	}

	return true;
}

void ObjectBoard::CameraZoom(float elapsed_time)
{
	GamePad* game_pad = GamesystemInput::GetInstance()->GetGamePad();
	Camera* camera = GamesystemDirector::GetInstance()->GetCamera();
	Camera::TPVData* tpv = camera->GetTPVCamera(1);

	if (!tpv)	return;

	tpv->tpv_distance -= game_pad->GetAxisLY();

	if (tpv->tpv_distance < ZOOM_MIN)
		tpv->tpv_distance = ZOOM_MIN;
	else if (tpv->tpv_distance > ZOOM_MAX)
		tpv->tpv_distance = ZOOM_MAX;
}

void ObjectBoard::CameraMove(float elapsed_time)
{
	Camera*				camera		= GamesystemDirector::GetInstance()->GetCamera();
	Camera::TPVData*	tpv			= camera->GetTPVCamera(1);
	GamePad*			game_pad	= GamesystemInput::GetInstance()->GetGamePad();

	if (!tpv)	return;

	if (game_pad->GetButton() & BTN_RIGHT_SHOULDER)
	{
		tpv->tpv_target.x += game_pad->GetAxisRX() * 0.5f;
		tpv->tpv_target.y += game_pad->GetAxisRY() * 0.5f;
		return;
	}

	camera_rot = {
		game_pad->GetAxisRY(),
		-game_pad->GetAxisRX(),
		0.0f
	};

	DirectX::XMVECTOR	v_camera_rot	= DirectX::XMVectorScale(DirectX::XMLoadFloat3(&camera_rot), elapsed_time);
	float				angle			= DirectX::XMVectorGetX(DirectX::XMVector3Length(v_camera_rot));
	DirectX::XMVECTOR	nv_camera_rot	= DirectX::XMVector3Normalize(v_camera_rot);

	DirectX::XMFLOAT3 n_camera_rot;
	DirectX::XMStoreFloat3(&n_camera_rot, nv_camera_rot);

	DirectX::XMVECTOR q_rotation = DirectX::XMVectorSet(
		n_camera_rot.x * sin(angle * 0.5f),
		n_camera_rot.y * sin(angle * 0.5f),
		n_camera_rot.z * sin(angle * 0.5f),
		cos(angle * 0.5f)
	);

	q_rotation = DirectX::XMQuaternionNormalize(q_rotation);
	DirectX::XMVECTOR v_direction = DirectX::XMLoadFloat4(&tpv->tpv_direction);
	v_direction = DirectX::XMVector3Rotate(v_direction, q_rotation);
	DirectX::XMStoreFloat4(&tpv->tpv_direction, v_direction);
}

// �������x���v�Z
void ObjectBoard::CalcBlockSpeed(float elapsed_time)
{
	UINT modify_level	= speed_level - init_level;
	float speed_base	= (SPEED_FACTOR * elapsed_time) + (SCast(float, init_level) - 1) * 0.05f;
	float add_speed		= (SPEED_FACTOR * elapsed_time) * (SCast(float, modify_level) * speed_increase_factor);

	current_speed = speed_base + add_speed;
}

// �Ֆʂ�h�炷����
void ObjectBoard::BoardShake(DirectX::XMFLOAT3 shake)
{
	shake_speed = XMFloatCalclation::XMFloat3Add(shake_speed, shake);
}

// �c�����`�F�b�N
void ObjectBoard::VerticalLineCheck(const UINT row_line)
{
	// �u���b�N�̃C�e���[�^���i�[����z��
	std::vector<UPtrVector<ObjectBlock>::iterator> itr_array;

	// �ꉞ�ŏ��s���A�ő�s���𒴂��Ă��Ȃ����`�F�b�N
	if (row_line < MIN_CELL || row_line > MAX_ROW)
	{
		return;
	}

	UINT			continuous_color = 1;					// �F���A�����Ă�������L�^
	UINT			last_column = 0;						// �`�F�b�N���I��������
	EnumBlockColor	last_color = EnumBlockColor::UNDEFINE;	// ���O�̃u���b�N�̐F

	// ����J�n�A������J�n
	for (UINT check_column = MAX_COLUMN; check_column >= MIN_CELL; check_column--)
	{
		// �`�F�b�N����u���b�N�̃C�e���[�^���擾
		UPtrVector<ObjectBlock>::iterator	block_itr;
		block_itr = GetBlockFromCell(BlockCell(row_line, check_column));

		// �u���b�N�����݂��Ă��Ȃ���΁A���������̃u���b�N�����݂��Ă��Ȃ��̂ŏI��
		if (block_itr == block_list.end())
		{
			last_column = check_column + 1;
			break;
		}

		// �u���b�N�����݂��Ă������A�F������Ă����ꍇ
		else if (last_color != (*block_itr)->GetBlockColor())
		{
			last_color = (*block_itr)->GetBlockColor();
			UINT push_count = continuous_color;

			continuous_color = 1;

			// �K�萔�ȏ�q�����Ă��Ȃ���������̃Z����
			if (push_count < ERASE_CONDITION)
				continue;

			// �Ȃ����Ă����u���b�N�����C�e���[�^�z���
			for (UINT count = 1; count <= push_count; count++)
			{
				UPtrVector<ObjectBlock>::iterator	move_to_array;
				move_to_array = GetBlockFromCell(BlockCell(row_line, check_column + count));
				itr_array.push_back(move_to_array);
			}
		}

		// �O��̃u���b�N�Ɠ����F��������
		else
		{
			continuous_color++;
		}
	}

	// �Ō�̗�܂œ����F��������Ō�ɃC�e���[�^�z��ɑ}������
	if (continuous_color >= 3)
	{
		UINT push_count = continuous_color;
		for (UINT count = 0; count < push_count; count++)
		{
			UPtrVector<ObjectBlock>::iterator	move_to_array;
			move_to_array = GetBlockFromCell(BlockCell(row_line, last_column + count));
			itr_array.push_back(move_to_array);
		}
	}

	// �F�������Ă����u���b�N��������Ԃɂ��Ă���
	for (const auto& block_itr : itr_array)
	{
		(*block_itr)->GetBlockState().TransitionEraseState();
	}
}

// �������`�F�b�N
void ObjectBoard::HorizontalLineCheck(const UINT column_line)
{
	// �u���b�N�̃C�e���[�^���i�[����z��
	std::vector<UPtrVector<ObjectBlock>::iterator> itr_array;

	// �ꉞ�ŏ��s���A�ő�s���𒴂��Ă��Ȃ����`�F�b�N
	if (column_line < MIN_CELL || column_line > MAX_COLUMN)
	{
		return;
	}

	UINT			continuous_color = 1;						// �F���A�����Ă�������L�^
	EnumBlockColor	last_color = EnumBlockColor::UNDEFINE;	// ���O�̃u���b�N�̐F

	// ����J�n�A������J�n
	for (UINT check_row = MIN_CELL; check_row <= MAX_ROW; check_row++)
	{
		// �`�F�b�N����u���b�N�̃C�e���[�^���擾
		UPtrVector<ObjectBlock>::iterator	block_itr;
		block_itr = GetBlockFromCell(BlockCell(check_row, column_line));

		// �u���b�N�����݂��Ă��āA�O��̃u���b�N�Ɠ����F��������
		if (block_itr != block_list.end() && last_color == (*block_itr)->GetBlockColor())
		{
			continuous_color++;
		}

		// �u���b�N�����݂��Ă��Ȃ������ꍇ�A�܂��͐F������Ă����ꍇ
		else
		{
			last_color = (block_itr == block_list.end()) ? EnumBlockColor::UNDEFINE : (*block_itr)->GetBlockColor();
			UINT push_count = continuous_color;

			continuous_color = 1;

			// �K�萔�ȏ�q�����Ă��Ȃ���������̃Z����
			if (push_count < ERASE_CONDITION)
				continue;

			for (UINT count = 1; count <= push_count; count++)
			{
				UPtrVector<ObjectBlock>::iterator	move_to_array;
				move_to_array = GetBlockFromCell(BlockCell(check_row - count, column_line));
				itr_array.push_back(move_to_array);
			}
		}
	}

	// �Ō�̍s�܂œ����F��������Ō�ɃC�e���[�^�z��ɑ}������
	if (continuous_color >= 3)
	{
		UINT push_count = continuous_color;
		for (UINT count = 0; count < push_count; count++)
		{
			UPtrVector<ObjectBlock>::iterator	move_to_array;
			move_to_array = GetBlockFromCell(BlockCell(MAX_ROW - count, column_line));
			itr_array.push_back(move_to_array);
		}
	}

	// �F�������Ă����u���b�N��������Ԃɂ��Ă���
	for (const auto& block_itr : itr_array)
	{
		(*block_itr)->GetBlockState().TransitionEraseState();
	}
}

// �u���b�N�̔z��̊Ԋu�𖄂߂�
void ObjectBoard::SortInList(UPtrVector<ObjectBlock>& list)
{
	auto begin = list.begin();
	auto end = list.end();

	size_t pop_count = 0;

	for (auto to_itr = begin; to_itr != end; to_itr++)
	{
		if ((*to_itr).get())		continue;
		for (auto from_itr = to_itr + 1; from_itr != end; from_itr++)
		{
			if (!(*from_itr).get())	continue;
			(*to_itr).swap(*from_itr);
			pop_count++;
			break;
		}
	}
}

// �Q�[���J�n
void ObjectBoard::GameStart(int game_mode_id)
{
	GamesystemDirector::GetInstance()->GetAudioManager()->StopBGM();
	deleted_block_count = 0;
	level_up_block = LV_UP_BLOCK_COUNT;
	speed_level = init_level;
	deleted_block_count = 0;
	score = 0;
	value_ui.at(SCast(size_t, EnumValueUIIndex::SPEED_LEVEL))->SetIntValue(0, 1, 0.0f);
	value_ui.at(SCast(size_t, EnumValueUIIndex::DELETED_BLOCKS))->SetIntValue(0, 0, 0.0f);
	value_ui.at(SCast(size_t, EnumValueUIIndex::SCORE))->SetIntValue(0, 0, 0.0f);
	count_down_time = 4.0f;
	board_state.TransitionStartState(game_mode_id);
}

void ObjectBoard::AccumulateBoardParticle()
{
	Graphics* graphics = Graphics::GetInstance();
	ID3D11DeviceContext* device_context = graphics->GetDeviceContext().Get();

	graphics->SetDepthStencilState(EnumDepthState::ZT_ON_ZW_OFF);
	graphics->SetRasterizerState(EnumRasterizerState::CULL_NONE);
	graphics->SetBlendState(EnumBlendState::ALPHA, nullptr, 0xFFFFFFFF);

	DirectX::XMFLOAT4X4 particle_transform;
	DirectX::XMStoreFloat4x4(&particle_transform,
		DirectX::XMLoadFloat4x4(&transform) * DirectX::XMMatrixTranslation(+1, 0, 0));

	auto call_back = [&](ID3D11PixelShader* accumulate_ps)
		{
			board_model->Render(false, particle_transform, accumulate_ps);
		};
	board_particle->AccumulateParticles(call_back);
}

// �V��ɓ��B���������`�F�b�N����֐�
bool ObjectBoard::CheckGameOver()
{
	bool game_over = false;
	for (UINT row = 1; row <= MAX_ROW; row++)		// block_cell�ԍ���1����n�܂邽�߂���for����1�ŊJ�n
	{
		game_over = IsExistingBlockFromCell(BlockCell(row, 1));
		if (game_over)
		{
			break;
		}
	}
	return game_over;
}

// �X�R�A���v�Z
UINT ObjectBoard::CalcScore(UINT block_count, UINT chain)
{
	return (SCORE_BASE + (speed_level - 1)) * block_count * (chain * chain);
}

// �w�肵���Z���Ƀu���b�N�����݂��邩���擾(��ɒ��n����)
bool ObjectBoard::IsExistingBlockFromCell(BlockCell cell)
{
	if (cell.row < 1 || cell.row > MAX_ROW)			return true;
	if (cell.column < 1 || cell.column > MAX_COLUMN)		return true;
	return existing_matrix[cell.row - 1][cell.column - 1];
}

// �w�肵���Z���̃C�e���[�^�������łȂ����̔���
bool ObjectBoard::IsInvalidBlockFromCell(BlockCell cell)
{
	auto itr = GetBlockFromCell(cell);
	bool invalid = (itr == block_list.end());
	return invalid;
}

// �C�e���[�^�������łȂ����̔���
bool ObjectBoard::IsInvalidBlockFromIterator(UPtrVector<ObjectBlock>::iterator itr)
{
	bool invalid = itr == block_list.end();
	return invalid;
}

// �w�肵���Z������u���b�N�̃C�e���[�^���擾
UPtrVector<ObjectBlock>::iterator ObjectBoard::GetBlockFromCell(const BlockCell cell)
{
	if (cell.row > MAX_ROW) return block_list.end();
	if (cell.column > MAX_COLUMN) return block_list.end();
	auto begin = block_list.begin();
	auto end = block_list.end();
	for (auto itr = begin; itr != end; itr++)
	{
		if (!(*itr).get())	continue;
		if ((*itr)->GetBlockCell() == cell)
			return itr;
	}
	return end;
}