// <>インクルード
#include <imgui.h>

// ""インクルード
// LightBlueEngine
#include "game_system/gamesystem_director.h"
#include "game_system/gamesystem_input.h"
#include "UI/value_UI.h"
#include "easing.h"
#include "json_editor.h"
#include "xmfloat_calclation.h"

// ゲームソースファイル
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

// 定数
const UINT				TEXT_SIZE				= 256;
const float				UI_SHAKE_FACTOR			= 10.0f;
const float				CONVERT_COLOR_FACTOR	= 255.0f;
const DirectX::XMFLOAT2 TEXT_LEFT_ALIGN_POS		= { 0.0f, 50.0f };
const DirectX::XMFLOAT2 TEXT_RIGHT_ALIGN_POS	= { 200.0f, 50.0f };
const DirectX::XMFLOAT2 DEFAULT_UI_SCALE		= { 0.1f,	0.1f };
const DirectX::XMFLOAT2 SPRITE_UI_SCALE			= { 0.2f,	0.2f };
const DirectX::XMFLOAT2 UI_SPRITE_CENTER_POS	= { 70.0f,	60.0f };
const DirectX::XMFLOAT2 UI_SPRITE_SIZE			= { 60.0f,	60.0f };
const DirectX::XMFLOAT2 UI_GAME_MODE_SIZE		= { 250.0f, 50.0f };

const float UI_LEFT_X		= SCast(float, Graphics::GetInstance()->GetScreenWidth()) * 0.22f;
const float UI_RIGHT_X		= SCast(float, Graphics::GetInstance()->GetScreenWidth()) * 0.64f;
const float UI_NEXT_BLOCK_Y = SCast(float, Graphics::GetInstance()->GetScreenHeight()) * 0.05f;
const float UI_TOP_Y		= SCast(float, Graphics::GetInstance()->GetScreenHeight()) * 0.5f;
const float UI_CENTER_Y		= SCast(float, Graphics::GetInstance()->GetScreenHeight()) * 0.65f;
const float UI_UNDER_Y		= SCast(float, Graphics::GetInstance()->GetScreenHeight()) * 0.8f;

//---------------------------------
// ObjectBoard メンバ関数
//---------------------------------
const DirectX::XMFLOAT3 ObjectBoard::INIT_POSITION =
{
	(ObjectBlock::BLOCK_SIZE * MAX_ROW * 0.5f) + 1.0f,
	(ObjectBlock::BLOCK_SIZE * MAX_COLUMN * -0.5f) - 1.5f
		,0.5f
};

// ブロック操作開始
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

// 更新処理
void ObjectBoard::NextBlock::Update(float elapsed_time)
{
	// プレイヤーIDを使ってコントローラーを取得
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
			EnumBlockRotation old_rotation = gamesystem_input->GetBlockRotation(ID);
			bool flip = gamesystem_input->BlockRotate(ID, input);
			bool succeed = RotateBlock(flip);
			if (!succeed)
				gamesystem_input->RotationFail(ID, old_rotation);
		}
		else
		{
			if (gamesystem_input->ActivateMoveContinuous(ID))
				obj->SignalMove(barrage);
			else
				obj->SignalMove(input);
		}

		if (hold & BTN_DOWN)
		{
			obj->current_speed = obj->level_speed > ACCEL_SPEED ? obj->level_speed : ACCEL_SPEED;
		}

		if (input & BTN_UP)
			SuperDrop();
	}

	MoveBlock(elapsed_time);

	if (moving_block.LEFT_BLOCK)
	{
		moving_block.LEFT_BLOCK->Update(elapsed_time);
		obj->root_block_column = moving_block.LEFT_BLOCK->GetBlockCell().column;
	}

	if (moving_block.RIGHT_BLOCK)
	{
		moving_block.RIGHT_BLOCK->Update(elapsed_time);
		if(obj->board_state.state == EnumBoardState::MOVING)
			moving_block.RIGHT_BLOCK->FollowPartnerBlock(gamesystem_input->GetBlockRotation(ID));
	}
}

// ブロック描画処理
void ObjectBoard::NextBlock::Render()
{
	if (moving_block.LEFT_BLOCK)
		moving_block.LEFT_BLOCK->Render();

	if (moving_block.RIGHT_BLOCK)
		moving_block.RIGHT_BLOCK->Render();
}

// エミッシブ部分を描画
void ObjectBoard::NextBlock::EmissiveRender()
{
	if (moving_block.LEFT_BLOCK)
		moving_block.LEFT_BLOCK->EmissiveRender();

	if (moving_block.RIGHT_BLOCK)
		moving_block.RIGHT_BLOCK->EmissiveRender();
}

// NEXTブロックを投入
void ObjectBoard::NextBlock::SetNewBlock()
{
	auto left	= std::make_unique<ObjectBlock>(false, obj);
	auto right	= std::make_unique<ObjectBlock>(false, obj);
	next_block.push_back(std::make_pair(std::move(left), std::move(right)));
}

void ObjectBoard::NextBlock::SuperDrop()
{
	ObjectBlock* left_block = moving_block.LEFT_BLOCK.get();
	ObjectBlock* right_block = moving_block.RIGHT_BLOCK.get();

	if (!left_block || !right_block)	return;

	BlockCell& left_cell = left_block->GetBlockCell();
	BlockCell& right_cell = right_block->GetBlockCell();

	shift_y += ObjectBlock::BLOCK_SIZE * MAX_COLUMN;

	obj->SetBlockColorFromCell(left_cell, EnumBlockColor::UNDEFINE);
	obj->SetBlockColorFromCell(right_cell, EnumBlockColor::UNDEFINE);

	UINT stand_height;
	for (int i = MAX_COLUMN; i > 0; i--)
	{
		bool left_exist = obj->IsExistFromColorMatrix(BlockCell(left_cell.row, SCast(UINT, i)));
		bool right_exist = obj->IsExistFromColorMatrix(BlockCell(right_cell.row, SCast(UINT, i)));
		if (!(left_exist || right_exist))
		{
			stand_height = SCast(UINT, i);
			break;
		}
	}

	while (shift_y >= ObjectBlock::BLOCK_SIZE)
	{
		shift_y -= ObjectBlock::BLOCK_SIZE;
		left_cell.column++;
		right_cell.column++;
		if ((left_cell.column >= stand_height) || (right_cell.column >= stand_height))
		{
			shift_y = 0.0f;
			left_cell.shift_y = 0.0f;
			right_cell.shift_y = 0.0f;

			EnumBlockRotation rotation = GamesystemInput::GetInstance()->GetBlockRotation(obj->player_id);

			left_cell.column = stand_height -
				(rotation == EnumBlockRotation::UNDER ? 1 : 0);
			right_cell.column = stand_height -
				(rotation == EnumBlockRotation::TOP ? 1 : 0);

			GamesystemDirector::GetInstance()->GetAudioManager()->PlaySE(EnumSEBank::STAND);
			standing = true;
		}
	}

	left_cell.shift_y	= shift_y;
	right_cell.shift_y	= shift_y;

	left_block->AdjustFromBlockCell();
	right_block->AdjustFromBlockCell();

	obj->SetBlockColorFromCell(left_cell, left_block->GetBlockColor());
	obj->SetBlockColorFromCell(right_cell, right_block->GetBlockColor());

	obj->board_state.TransitionDroppingState();
}

void ObjectBoard::NextBlock::MoveBlock(float elapsed_time)
{
	ObjectBlock* left_block		= moving_block.LEFT_BLOCK.get();
	ObjectBlock* right_block	= moving_block.RIGHT_BLOCK.get();

	if (!left_block || !right_block)	return;

	BlockCell& left_cell = left_block->GetBlockCell();
	BlockCell& right_cell = right_block->GetBlockCell();

	shift_y += standing ? 0.0f : obj->current_speed * elapsed_time;

	if (standing)
	{
		shift_y = 0.0f;
		bool left_exist = obj->IsExistFromColorMatrix(BlockCell(left_cell.row, left_cell.column + 1));
		bool right_exist = obj->IsExistFromColorMatrix(BlockCell(right_cell.row, right_cell.column + 1));

		EnumBlockRotation rotation = GamesystemInput::GetInstance()->GetBlockRotation(obj->player_id);
		switch (rotation)
		{
			using enum EnumBlockRotation;
		case LEFT:
		case RIGHT:
			if (!(left_exist || right_exist))
				standing = false;
			break;

		case TOP:
			if (!left_exist)
				standing = false;
			break;

		case UNDER:
			if (!right_exist)
				standing = false;
			break;
		}
	}

	if (shift_y < ObjectBlock::BLOCK_SIZE)
	{
		left_cell.shift_y = shift_y;
		right_cell.shift_y = shift_y;
		left_block->AdjustFromBlockCell();
		right_block->AdjustFromBlockCell();
		return;
	}

	obj->SetBlockColorFromCell(left_cell, EnumBlockColor::UNDEFINE);
	obj->SetBlockColorFromCell(right_cell, EnumBlockColor::UNDEFINE);

	UINT stand_height;
	for (int i = MAX_COLUMN; i > 0; i--)
	{
		bool left_exist = obj->IsExistFromColorMatrix(BlockCell(left_cell.row, SCast(UINT, i)));
		bool right_exist = obj->IsExistFromColorMatrix(BlockCell(right_cell.row, SCast(UINT, i)));
		if (!(left_exist || right_exist))
		{
			stand_height = SCast(UINT, i);
			break;
		}
	}

	while (shift_y >= ObjectBlock::BLOCK_SIZE)
	{
		shift_y -= ObjectBlock::BLOCK_SIZE;
		left_cell.column++;
		right_cell.column++;
		if ((left_cell.column >= stand_height) || (right_cell.column >= stand_height))
		{
			shift_y = 0.0f;
			left_cell.shift_y = 0.0f;
			right_cell.shift_y = 0.0f;

			EnumBlockRotation rotation = GamesystemInput::GetInstance()->GetBlockRotation(obj->player_id);

			left_cell.column = stand_height - 
								(rotation == EnumBlockRotation::UNDER ? 1 : 0);
			right_cell.column = stand_height -
								(rotation == EnumBlockRotation::TOP ? 1 : 0);

			GamesystemDirector::GetInstance()->GetAudioManager()->PlaySE(EnumSEBank::STAND);
			standing = true;
		}
	}

	left_cell.shift_y = shift_y;
	right_cell.shift_y = shift_y;

	left_block->AdjustFromBlockCell();
	right_block->AdjustFromBlockCell();

	obj->SetBlockColorFromCell(left_cell, left_block->GetBlockColor());
	obj->SetBlockColorFromCell(right_cell, right_block->GetBlockColor());
}

bool ObjectBoard::NextBlock::RotateBlock(bool flip)
{
	using enum EnumBlockRotation;
	const EnumBlockRotation rotation = GamesystemInput::GetInstance()->GetBlockRotation(0);

	auto root_block = moving_block.LEFT_BLOCK.get();
	auto rotate_block = moving_block.RIGHT_BLOCK.get();

	auto& block_cell = rotate_block->GetBlockCell();

	BlockCell& root_cell = root_block->GetBlockCell();
	const BlockCell& CELL_L = BlockCell(root_cell.row - 1, root_cell.column);
	const BlockCell& CELL_UL = BlockCell(root_cell.row - 1, root_cell.column + 1);

	const BlockCell& CELL_R = BlockCell(root_cell.row + 1, root_cell.column);
	const BlockCell& CELL_UR = BlockCell(root_cell.row + 1, root_cell.column + 1);

	const BlockCell& CELL_DU = BlockCell(root_cell.row, root_cell.column + 2);

	size_t rotation_index = SCast(size_t, rotation);

	BlockCell& root_ghost = root_block->GetGhostCell();
	BlockCell& rotate_ghost = rotate_block->GetGhostCell();

	obj->SetBlockColorFromCell(root_cell, EnumBlockColor::UNDEFINE);
	obj->SetBlockColorFromCell(block_cell, EnumBlockColor::UNDEFINE);

	switch (rotation)
	{
	case RIGHT:
		if (block_cell.row == MAX_ROW || obj->IsExistFromColorMatrix(CELL_R))
		{
			if (!obj->IsExistFromColorMatrix(CELL_L))
			{
				root_block->LiftBlock(rotation);

				const BlockCell moved_root_cell = root_block->GetBlockCell();
				if (obj->IsExistFromColorMatrix(
					BlockCell(moved_root_cell.row, moved_root_cell.column + 1)))
				{
					root_block->GetBlockState().TransitionStandState();
					GamesystemDirector::GetInstance()->GetAudioManager()->PlaySE(EnumSEBank::STAND);
					standing = true;
				}
				
			}
			else
				return false;
		}
		else if (obj->IsExistFromColorMatrix(CELL_UR))
		{
			root_cell.shift_y = 0.0f;
			block_cell.shift_y = 0.0f;
			root_block->GetBlockState().TransitionStandState();
			GamesystemDirector::GetInstance()->GetAudioManager()->PlaySE(EnumSEBank::STAND);
			standing = true;
		}

		break;

	case UNDER:
		if ((moving_block.LEFT_BLOCK->GetBlockState().state == EnumBlockState::STAND)
			|| (moving_block.RIGHT_BLOCK->GetBlockState().state == EnumBlockState::STAND))
			root_block->LiftBlock(rotation);
		else if (obj->IsExistFromColorMatrix(CELL_DU))
		{
			if (!flip)
			{
				root_cell.shift_y = 0.0f;
				block_cell.shift_y = 0.0f;
				root_block->GetBlockState().TransitionStandState();
				GamesystemDirector::GetInstance()->GetAudioManager()->PlaySE(EnumSEBank::STAND);
				standing = true;
			}
		}

		break;

	case LEFT:
		if (block_cell.row == 1 || obj->IsExistFromColorMatrix(CELL_L))
		{
			if (!obj->IsExistFromColorMatrix(CELL_R))
			{
				root_block->LiftBlock(rotation);

				const BlockCell moved_root_cell = root_block->GetBlockCell();
				if (obj->IsExistFromColorMatrix(
					BlockCell(moved_root_cell.row, moved_root_cell.column + 1)))
				{
					root_block->GetBlockState().TransitionStandState();
					GamesystemDirector::GetInstance()->GetAudioManager()->PlaySE(EnumSEBank::STAND);
					standing = true;
				}
			}
			else
				return false;
		}
		else if (obj->IsExistFromColorMatrix(CELL_UL))
		{
			root_cell.shift_y = 0.0f;
			block_cell.shift_y = 0.0f;
			root_block->GetBlockState().TransitionStandState();
			GamesystemDirector::GetInstance()->GetAudioManager()->PlaySE(EnumSEBank::STAND);
			standing = true;
		}

		break;

	case TOP:
		root_block->LiftBlock(rotation);
		break;

	default:
		break;
	}


	if (!flip)
	{
		BlockCell rotated_cell = root_cell + ROTATION_CELL[rotation_index];
		obj->SetBlockColorFromCell(block_cell, EnumBlockColor::UNDEFINE);
		block_cell = rotated_cell;
		obj->SetBlockColorFromCell(block_cell, rotate_block->GetBlockColor());
	}
	else
	{
		BlockCell temp_cell = root_cell;
		root_cell = block_cell;
		block_cell = temp_cell;
	}

	root_ghost		= BlockCell(root_cell.row, 
						obj->stand_collision_heignt[root_cell.row - 1] + (rotation == UNDER ? 0 : 1));
	rotate_ghost	= BlockCell(block_cell.row, 
						obj->stand_collision_heignt[block_cell.row - 1] + (rotation == TOP ? 0 : 1));

	root_block->AdjustFromBlockCell();
	rotate_block->AdjustFromBlockCell();


	return true;
}

//---------------------------------------------------------------------------------------
//		BoardState Transition系
//---------------------------------------------------------------------------------------

void ObjectBoard::BoardState::TransitionStandbyState()
{
	state = EnumBoardState::STANDBY;
	obj->state_update = nullptr;

	AudioManager* audio_manager = GamesystemDirector::GetInstance()->GetAudioManager();
	audio_manager->StopBGM();
	audio_manager->PlayBGM(EnumBGMBank::MODE_SELECT, true);

	obj->ui_alpha = 0.0f;
}

// ゲーム開始
void ObjectBoard::BoardState::TransitionStartState(int game_mode_id)
{
	AudioManager*		audio_manager	= GamesystemDirector::GetInstance()->GetAudioManager();
	Camera*				camera			= GamesystemDirector::GetInstance()->GetCamera();
	Camera::TPVData*	tpv				= camera->GetTPVCamera(SCast(size_t, EnumCameraChannel::GAME));

	// 状態変更後処理
	state = EnumBoardState::START;
	obj->state_update = &ObjectBoard::UpdateStartState;

	std::string mode_name(magic_enum::enum_name(SCast(EnumGameMode, game_mode_id)).data());

	Parameters	mode_params;
	ParamPtr	mode_setting_params;

	obj->game_mode		= SCast(EnumGameMode, game_mode_id);
	obj->board_color	= BOARD_COLOR_SET[game_mode_id];
	
	obj->sprite_ui.at(SCast(size_t, GAME_MODE))->SetIndexValue(0, game_mode_id);
	
	if (obj->game_mode != EnumGameMode::BONUS)
	{
		obj->board_model->GetDisolveFactor() = 0.0f;

		audio_manager->StopBGM();

		int level = SCast(int, obj->game_data.speed_level) * 1000;
		int section = SCast(int, (obj->current_rank < 6 && obj->speed_rank[obj->current_rank + 1] != -1) ? obj->speed_rank[obj->current_rank + 1] : obj->game_data.max_level);
		obj->value_ui.at(SCast(size_t, SPEED_LEVEL))->SetIntValue(0, level + section);

		obj->game_data.speed_level = obj->game_data.init_level;
	}
	else
	{
		obj->flag_system.SetFlag(EnumBoardFlags::PLAYING, true);
		audio_manager->PlayBGM(obj->before_game_mode == EnumGameMode::EASY ? EnumBGMBank::EASY_BONUS_TIME : EnumBGMBank::BONUS_TIME, false);
	}

	obj->UpdateStandCollisionHeight();
}

// ブロック移動開始
void ObjectBoard::BoardState::TransitionDropStartState()
{
	GamesystemInput* gamesystem_input = GamesystemInput::GetInstance();

	// 状態変更後処理
	state = EnumBoardState::DROP_START;
	obj->next_block.standing = false;

	if (obj->game_mode == EnumGameMode::BONUS)
	{
		if (obj->object_time < 0.0f)
		{
			TransitionGameOverState(true);
			return;
		}
	}
	
	obj->ClearDeleteBlockList();

	int count = 0;
	for(auto& erase_particle_data : obj->erase_block_particle)
	{
		if (erase_particle_data.second < 0.0f)
		{
			obj->erase_block_particle.erase(obj->erase_block_particle.begin() + count);
			obj->erase_block_particle.shrink_to_fit();
		}
		else
			count++;
	}

	obj->state_update = nullptr;
	obj->standing_time = 0.0f;
	obj->chain = 0;

	gamesystem_input->ResetRotate(obj->player_id);
	obj->flag_system.SetFlag(EnumBoardFlags::BLOCK_REGISTERED, false);

	obj->next_block.Start();
	obj->next_block.SetNewBlock();

	TransitionMovingState();
}

// ブロック移動開始
void ObjectBoard::BoardState::TransitionMovingState()
{
	// 状態変更後処理
	state = EnumBoardState::MOVING;
	obj->state_update = &ObjectBoard::UpdateMovingState;
	obj->flag_system.SetFlag(EnumBoardFlags::CAN_MOVE, true);

	obj->UpdateStandCollisionHeight();
}

// ブロック落下開始
void ObjectBoard::BoardState::TransitionDroppingState()
{
	// 状態変更後処理
	state = EnumBoardState::DROPPING;
	obj->state_update = &ObjectBoard::UpdateDroppingState;
	obj->standing_time = 0.0f;

	obj->current_speed = 0.0f;

	obj->UpdateStandCollisionHeight();
}

// 着地
void ObjectBoard::BoardState::TransitionLandingState()
{
	// 状態変更後処理
	state = EnumBoardState::LANDING;
	obj->state_update = &ObjectBoard::UpdateLandingState;
	obj->standing_time = 0.0f;
	obj->current_speed = 0.0f;

	for (UINT row = 1; row <= MAX_ROW; row++)
	{
		obj->VerticalLineCheck(row);
	}

	for (UINT column = 1; column <= MAX_COLUMN + DIAGONAL_ADJUST; column++)
	{
		if(column <= MAX_COLUMN)
			obj->HorizontalLineCheck(column);
		
		obj->DiagonalLineCheckLU(column);
		obj->DiagonalLineCheckLD(column);
	}

	obj->flag_system.SetFlag(EnumBoardFlags::RESULT, obj->MoveToDeletedBlockList());

	obj->UpdateStandCollisionHeight();

	if(obj->flag_system.GetFlag(EnumBoardFlags::RESULT))
	{
		ParticleSystem::CbParticleEmitter	cb_emitter;
		cb_emitter.emit_amounts = 50000;
		cb_emitter.emit_speed	= 5.0f;
		cb_emitter.life_time	= 1.0f;
		cb_emitter.emit_size	= 0.2f;
		cb_emitter.emit_radius	= 0.5f;

		EraseParticleData& erase_particle_data = obj->erase_block_particle.emplace_back();

		erase_particle_data.first = std::make_unique<ParticleSystem>(cb_emitter, true, "accumulate_particle_ps.cso");

		auto call_back = [&](ID3D11PixelShader* accumulate_ps) {
			for (const auto& erased_block : obj->erased_block_list)
			{
				erased_block->AccumulateBlockParticle(accumulate_ps);
			}
			};
		erase_particle_data.first->AccumulateParticles(call_back);

		erase_particle_data.second = 1.0f;
	}
}

// ゲームオーバー
void ObjectBoard::BoardState::TransitionGameOverState(bool cleared)
{
	state = EnumBoardState::GAME_OVER;
	obj->state_update = &ObjectBoard::UpdateGameOverState;

	if (obj->game_mode != EnumGameMode::BONUS)
	{
		AudioManager* audio_manarger = GamesystemDirector::GetInstance()->GetAudioManager();
		audio_manarger->StopBGM();
		if(cleared)		audio_manarger->PlaySE(EnumSEBank::GAME_CLEAR);
	}

	for (const auto& block : obj->block_list)
	{
		block->EraseRegist();
	}

	obj->MoveToDeletedBlockList();

	for (int row = 0; row < MAX_ROW; row++)
	{
		for (int column = MAX_COLUMN - 1; column >= 0; column--)
		{
			obj->block_color_matrix[row][column] = EnumBlockColor::UNDEFINE;
		}
	}

	obj->game_data.cleared = cleared;

	obj->flag_system.SetFlag(EnumBoardFlags::PLAYING, false);
}

void ObjectBoard::BoardState::TransitionResultState()
{
	// 状態変更後処理
	state = EnumBoardState::RESULT;
	obj->state_update = nullptr;
}

//---------------------------------------------------------------------------------------
//		State別Update系
//---------------------------------------------------------------------------------------
// カウントダウン
void ObjectBoard::UpdateStartState(float elapsed_time)
{
	AudioManager*		audio_manager	= GamesystemDirector::GetInstance()->GetAudioManager();
	Camera*				camera			= GamesystemDirector::GetInstance()->GetCamera();
	Camera::TPVData*	tpv				= camera->GetTPVCamera(SCast(size_t, EnumCameraChannel::GAME));

	count_down_time -= elapsed_time;
	count_down_se_time += elapsed_time;
	float& dissolve = board_model->GetDisolveFactor();
	dissolve = Easing::Out(EnumEasingType::QUAD, EnumEasingMode::RATE, 0.0f, COUNT_DOWN, COUNT_DOWN - count_down_time);

	ui_alpha = dissolve;

	tpv->tpv_distance = std::lerp(ZOOM_MIN, DEFAULT_ZOOM, dissolve);

	if(game_mode != EnumGameMode::BONUS)
	{
		if (count_down_time > 0.0f)
		{
			if (count_down_se_time > count_down_se_span)
			{
				audio_manager->PlaySE(EnumSEBank::COUNT_DOWN);

				count_down_se_time -= count_down_se_span;
			}
		}
		else if (count_down_time < 0.0f)
		{
			tpv->tpv_distance = DEFAULT_ZOOM;
			dissolve = 1.0f;
			audio_manager->PlayBGM(SCast(EnumBGMBank, current_rank), true);
			flag_system.SetFlag(EnumBoardFlags::PLAYING, true);
			board_state.TransitionDropStartState();
		}
	}
	else
	{
		flag_system.SetFlag(EnumBoardFlags::PLAYING, true);
		board_state.TransitionDropStartState();
	}
}

// ブロック移動中の処理
void ObjectBoard::UpdateMovingState(float elapsed_time)
{
	GamesystemInput* gamesystem_input = GamesystemInput::GetInstance();

	current_speed = level_speed;

	// 左右どちらかがが静止状態であるとき
	if (next_block.moving_block.LEFT_BLOCK->GetBlockState().state == EnumBlockState::STAND
		|| next_block.moving_block.RIGHT_BLOCK->GetBlockState().state == EnumBlockState::STAND)
	{
		// 両方を静止状態にして、静止時間タイマーを作動させる
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

		// 回転軸でないブロックとの位置関係で、浮いているかの判定に使うブロックを変えている
		switch (gamesystem_input->GetBlockRotation(player_id))
		{
		case EnumBlockRotation::RIGHT:
		case EnumBlockRotation::LEFT:
			result = !(IsExistFromColorMatrix(under_cell_l) || IsExistFromColorMatrix(under_cell_r));
			break;

		case EnumBlockRotation::UNDER:
			result = !IsExistFromColorMatrix(under_cell_r);
			break;

		case EnumBlockRotation::TOP:
			result = !IsExistFromColorMatrix(under_cell_l);
			break;
		}

		if (result)
		{
			next_block.moving_block.LEFT_BLOCK->ResetStanding();
			next_block.moving_block.RIGHT_BLOCK->ResetStanding();
		}
	}
}

// ブロック落下中の処理
void ObjectBoard::UpdateDroppingState(float elapsed_time)
{
	const float MAX_SPEED = ObjectBlock::BLOCK_SIZE * 10.0f;

	if (current_speed < MAX_SPEED)
		current_speed += elapsed_time;
	else
		current_speed = MAX_SPEED;

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

// ブロックがすべて着地した時の処理
void ObjectBoard::UpdateLandingState(float elapsed_time)
{
	current_speed = 0.0f;

	if (waiting_time > waiting_time_limit)
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

// ゲームオーバー処理
void ObjectBoard::UpdateGameOverState(float elapsed_time)
{
	AudioManager* audio_manager = GamesystemDirector::GetInstance()->GetAudioManager();
	if(audio_manager->IsPlaying())
		audio_manager->StopBGM();

	game_over_time += elapsed_time;

	if(game_data.cleared)
	{
		if (game_over_time > 3.0f)
		{
			game_over_time = 0.0f;

			if (game_mode != EnumGameMode::BONUS)
				BonusStart(game_mode == EnumGameMode::EASY);

			else
				board_state.TransitionResultState();
		}
	}
	else
	{
		float& disolve_factor = board_model->GetDisolveFactor();
		disolve_factor -= 0.02f;

		if (game_over_time > 5.0f)
		{
			game_over_time = 0.0f;
			board_state.TransitionResultState();
		}
	}
}

// ブロックを移動させる関数
void ObjectBoard::SignalMove(GamePadButton input)
{
	GamesystemInput* gamesystem_input = GamesystemInput::GetInstance();

	bool		result = false;

	UINT shift_factor = next_block.moving_block.LEFT_BLOCK->GetBlockCell().shift_y < FLT_EPSILON ? 0 : 1;

	const BlockCell&	LEFT_CELL_L		= BlockCell(next_block.moving_block.LEFT_BLOCK->GetBlockCell().row - 1,
											next_block.moving_block.LEFT_BLOCK->GetBlockCell().column + shift_factor);

	const BlockCell&	LEFT_CELL_R		= BlockCell(next_block.moving_block.RIGHT_BLOCK->GetBlockCell().row - 1,
											next_block.moving_block.RIGHT_BLOCK->GetBlockCell().column + shift_factor);

	const BlockCell&	RIGHT_CELL_L	= BlockCell(next_block.moving_block.LEFT_BLOCK->GetBlockCell().row + 1,
											next_block.moving_block.LEFT_BLOCK->GetBlockCell().column + shift_factor);

	const BlockCell&	RIGHT_CELL_R	= BlockCell(next_block.moving_block.RIGHT_BLOCK->GetBlockCell().row + 1,
											next_block.moving_block.RIGHT_BLOCK->GetBlockCell().column + shift_factor);

	if (input & BTN_LEFT)
	{
		switch (gamesystem_input->GetBlockRotation(player_id))
		{
		case EnumBlockRotation::RIGHT:
		case EnumBlockRotation::TOP:
			if (!IsExistFromColorMatrix(LEFT_CELL_L))
				result = true;
			break;
		case EnumBlockRotation::UNDER:
		case EnumBlockRotation::LEFT:
			if (!IsExistFromColorMatrix(LEFT_CELL_R))
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
			if (!IsExistFromColorMatrix(RIGHT_CELL_R))
				result = true;
			break;
		case EnumBlockRotation::LEFT:
		case EnumBlockRotation::TOP:
			if (!IsExistFromColorMatrix(RIGHT_CELL_L))
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

// コンストラクタ
ObjectBoard::ObjectBoard(UINT player_id)
	: player_id(player_id)
{
	translation = { 0.0f,0.0f,0.0f };
	rotation.y = DirectX::XM_PIDIV2;
	board_model = std::make_unique<GameModel>("resources/model/board/board.gltf");
	board_model->SetMaskTexture(L"resources/sprite/mask_texture.png");

	existing_matrix.resize(MAX_ROW);
	for (int x = 0; x < MAX_ROW; x++)
	{
		existing_matrix.at(x).resize(MAX_COLUMN);
	}

	for (UINT r = 0; r < MAX_ROW; r++)
	{
		std::vector<EnumBlockColor>& block_color_line = block_color_matrix.emplace_back();
		for (UINT c = 0; c < MAX_COLUMN; c++)
		{
			block_color_line.push_back(EnumBlockColor::UNDEFINE);
		}
	}

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
		L"resources/sprite/mode/easy.png",
		L"resources/sprite/mode/impact.png",
		L"resources/sprite/mode/endless.png",
		L"resources/sprite/mode/mission.png",
		L"resources/sprite/mode/tempest.png",
		L"resources/sprite/mode/bonus.png",
	};

	for (const wchar_t* w_filename : w_game_mode_filename)
	{
		ui_game_mode->PushSpriteImage(w_filename, UI_GAME_MODE_SIZE);
	}

	ui_game_mode->PushImagePosition({ 0.0f, UI_GAME_MODE_SIZE.y * 0.5f });
	ui_game_mode->SetUIScale({ 1.0f, 1.0f });

	auto& ui_time = value_ui.emplace_back(std::make_unique<ValueUI>());
	ui_time->Initialize(L"resources/sprite/UI/UI_time.png", L"TIME", { UI_RIGHT_X, UI_CENTER_Y });
	ui_time->PushIntValue(SCast(int, object_time), TEXT_LEFT_ALIGN_POS);
	ui_time->SetUIScale(DEFAULT_UI_SCALE);

	auto& ui_score = value_ui.emplace_back(std::make_unique<ValueUI>());
	ui_score->Initialize(L"resources/sprite/UI/UI_score.png", L"SCORE", { UI_RIGHT_X, UI_UNDER_Y });
	ui_score->PushIntValue(game_data.score, TEXT_LEFT_ALIGN_POS);
	ui_score->SetUIScale(DEFAULT_UI_SCALE);

	auto& ui_speed_level = value_ui.emplace_back(std::make_unique<ValueUI>());
	ui_speed_level->Initialize(L"resources/sprite/UI/UI_speed_lv.png", L"SPEED_LEVEL", { UI_LEFT_X, UI_CENTER_Y });
	ui_speed_level->PushIntValue(game_data.speed_level, TEXT_LEFT_ALIGN_POS);
	ui_speed_level->SetUIScale(DEFAULT_UI_SCALE);

	auto& ui_erased_blocks = value_ui.emplace_back(std::make_unique<ValueUI>());
	ui_erased_blocks->Initialize(L"resources/sprite/UI/UI_deleted_blocks.png", L"ERASED_BLOCKS", { UI_LEFT_X, UI_UNDER_Y });
	ui_erased_blocks->PushIntValue(game_data.deleted_block_count, TEXT_LEFT_ALIGN_POS);
	ui_erased_blocks->SetUIScale(DEFAULT_UI_SCALE);

	delete_block_sorter.color_block.resize(6);

	board_state.TransitionStandbyState();
}

// デストラクタ
ObjectBoard::~ObjectBoard()
{
}

// 更新処理
void ObjectBoard::Update(float elapsed_time)
{
	GamesystemInput* gamesystem_input = GamesystemInput::GetInstance();
	const GamePad* game_pad = gamesystem_input->GetGamePad(player_id);
	AudioManager* audio_manager = GamesystemDirector::GetInstance()->GetAudioManager();

	if (pause)
	{
		if (gamesystem_input->GetGamePadButtonDown(player_id) & BTN_START)
		{
			pause = false;
		}
		return;
	}
	else if (flag_system.GetFlag(EnumBoardFlags::PLAYING))
	{
		if (gamesystem_input->GetGamePadButtonDown(player_id) & BTN_START)
		{
			pause = true;
		}
	}

	// state別アップデート
	if (state_update)
		(this->*state_update)(elapsed_time);

	ClearDeleteBlockList();

	if (flag_system.GetFlag(EnumBoardFlags::PLAYING))
	{
		if (game_mode != EnumGameMode::BONUS)
		{
			object_time += elapsed_time;
		}
		else
		{
			object_time -= elapsed_time;
		}
	}

	// カメラ移動
	CameraZoom(elapsed_time);
	CameraMove(elapsed_time);

	// 揺らし処理
	DirectX::XMFLOAT3 root_translation = translation;
	shake_position	= XMFloatCalclation::XMFloat3Add(shake_position, shake_speed);
	translation		= XMFloatCalclation::XMFloat3Subtract(translation, shake_position);

	// transform更新
	UpdateTransform();

	// translationを元に戻す
	translation		= root_translation;
	shake_speed		= XMFloatCalclation::XMFloat3Lerp(shake_speed, { 0.0f,0.0f,0.0f }, 0.3f);
	shake_position	= XMFloatCalclation::XMFloat3Lerp(shake_position, { 0.0f,0.0f,0.0f }, 0.03f);

	// blockの更新処理
	for (const auto& block : block_list)
	{
		block->Update(elapsed_time);
	}

	// 消去演出用の更新処理
	for (const auto& erased_block : erased_block_list)
	{
		if (!erased_block)	continue;
		if (erased_block->GetBlockState().state == EnumBlockState::ERASE)
			erased_block->ErasingUpdate(elapsed_time);
	}

	if(flag_system.GetFlag(EnumBoardFlags::PLAYING))
	{
		next_block.Update(elapsed_time);
	}

	if (game_mode != EnumGameMode::BONUS)
	{
		int game_decimal = SCast(int, object_time * 100.0f) - SCast(int, object_time) * 100;
		int game_second = SCast(int, object_time) % 60;
		int game_minute = SCast(int, object_time) / 60;
		value_ui.at(SCast(size_t, EnumValueUIIndex::TIME))->SetIntValue(0, game_decimal + game_second * 100 + game_minute * 10000, 0.0f);
	}

	UIUpdate(elapsed_time);

	board_model->InstanceUpdate();

	int count = 0;
	for (auto& erase_particle_data : erase_block_particle)
	{
		erase_particle_data.first->Update(elapsed_time, Graphics::GetInstance()->GetComputeShader(EnumComputeShader::BLOCK).Get());
		erase_particle_data.second -= elapsed_time;
	}
}

// UIの更新処理
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

// ImGui表示
void ObjectBoard::DebugGUI()
{
	if (ImGui::CollapsingHeader("ObjectBoard"))
	{
		for (int i = 0; i < 6; i++)
		{
			int erasing_block_count = SCast(int, delete_block_sorter.color_block[i].size());
			ImGui::InputInt("erasing_block##1", &erasing_block_count);
		}

		ImGui::Separator();

		int e_size = erase_block_particle.size();
		ImGui::InputInt("Erase_particle Size", &e_size);
		ImGui::InputFloat("Object Time", &object_time);
		ImGui::InputFloat("Current Speed", &current_speed);
		ImGui::InputFloat("Rigidity Time", &standing_time);
		ImGui::InputFloat("Rigidity Time Limit", &standing_time_limit);

		int		row_l		= next_block.moving_block.first ? next_block.moving_block.first->GetBlockCell().row : -1;
		int		column_l	= next_block.moving_block.first ? next_block.moving_block.first->GetBlockCell().column : -1;
		float	shift_l		= next_block.moving_block.first ? next_block.moving_block.first->GetBlockCell().shift_y : -1.0f;

		int		row_r		= next_block.moving_block.first ? next_block.moving_block.second->GetBlockCell().row : -1;
		int		column_r	= next_block.moving_block.first ? next_block.moving_block.second->GetBlockCell().column : -1;
		float	shift_r		= next_block.moving_block.first ? next_block.moving_block.second->GetBlockCell().shift_y : -1.0f;

		int cell_l[] = { row_l, column_l };
		int cell_r[] = { row_r, column_r };

		ImGui::InputInt2("Left Block Cell", cell_l);
		ImGui::InputFloat("Left Block Shift", &shift_l);
		ImGui::InputInt2("Right Block Cell", cell_r);
		ImGui::InputFloat("Right Block Shift", &shift_r);

		void* vp_level = &game_data.speed_level;
		if (ImGui::InputScalar("Speed Level", ImGuiDataType_U16, vp_level))
		{
			if (game_data.speed_level < game_data.init_level)		game_data.speed_level = game_data.init_level;
			else if (game_data.speed_level > game_data.max_level)	game_data.speed_level = game_data.max_level;
			else													game_data.speed_level = game_data.speed_level;
		}

		int id = SCast(int, player_id);
		ImGui::InputInt("Player Id", &id);

		for (int i = 0; i < delete_block_sorter.color_block.size(); i++)
		{
			size_t		size	= delete_block_sorter.color_block[i].size();
			std::string str_id	= "Sorter" + std::to_string(i);
			ImGui::InputInt(str_id.c_str(), &id);
		}

		int erased = SCast(int, game_data.deleted_block_count);
		ImGui::InputInt("deleted_block_count", &erased);

		char block_state_str[TEXT_SIZE] = "Board State : ";
		auto state = magic_enum::enum_name(board_state.state);
		strcat_s(block_state_str, sizeof(block_state_str), state.data());
		ImGui::Text(block_state_str);
	}
}

// 描画処理
void ObjectBoard::Render()
{
	Graphics* graphics = Graphics::GetInstance();

	graphics->SetDepthStencilState(EnumDepthState::ZT_ON_ZW_ON);
	graphics->SetRasterizerState(EnumRasterizerState::SOLID);
	graphics->SetBlendState(EnumBlendState::ALPHA, nullptr, 0xFFFFFFFF);

	board_model->Render(false, transform, board_color);

	if(!pause)
	{
		for (UPtrVector<ObjectBlock>::const_reference block : block_list)
			block->Render();

		for (const auto& erased_block : erased_block_list)
		{
			if (!erased_block)	continue;
			if (erased_block->GetBlockState().state == EnumBlockState::ERASE)
				erased_block->Render();
		}

		for (auto& erase_particle_data : erase_block_particle)
		{
			erase_particle_data.first->Render();
		}

		next_block.Render();
	}
}

// エミッシブ部分を描画
void ObjectBoard::EmissiveRender()
{
	Graphics* graphics = Graphics::GetInstance();

	graphics->SetDepthStencilState(EnumDepthState::ZT_ON_ZW_ON);
	graphics->SetRasterizerState(EnumRasterizerState::SOLID);
	graphics->SetBlendState(EnumBlendState::ALPHA, nullptr, 0xFFFFFFFF);

	if(!pause)
	{
		board_model->Render(false, transform, board_color, graphics->GetPixelShader(EnumPixelShader::EXTRACT_EMISSIVE).Get());

		for (UPtrVector<ObjectBlock>::const_reference block : block_list)
			block->EmissiveRender();

		for (const auto& erased_block : erased_block_list)
		{
			if (!erased_block)	continue;
			if (erased_block->GetBlockState().state == EnumBlockState::ERASE)
				erased_block->EmissiveRender();
		}

		next_block.EmissiveRender();

		for (auto& erase_particle_data : erase_block_particle)
		{
			erase_particle_data.first->Render();
		}
	}

}

// UIを描画
void ObjectBoard::UIRender()
{
	float width		= Graphics::GetInstance()->GetScreenWidth() * 0.5f;
	float height	= Graphics::GetInstance()->GetScreenHeight() * 0.5f;

	for (auto const& ui : value_ui)
	{
		if (ui->GetName() == L"TIME")
			ui->Render(EnumNumberAlignment::LEFT_ALIGNMENT, ui_alpha, { 1.0f, 1.0f, 1.0f }, "NNCNNCNN");
		else if (ui->GetName() == L"SPEED_LEVEL")
			ui->Render(EnumNumberAlignment::LEFT_ALIGNMENT, ui_alpha, ui_color, "NNNSNNN");
		else if (ui->GetName() == L"SCORE")
			ui->Render(EnumNumberAlignment::LEFT_ALIGNMENT, ui_alpha);
		else if (ui->GetName() == L"ERASED_BLOCKS")
			ui->Render(EnumNumberAlignment::LEFT_ALIGNMENT, ui_alpha);
	}

	for (auto const& ui : sprite_ui)
		ui->Render(ui_alpha);

	if (game_mode != EnumGameMode::BONUS && board_state.state == EnumBoardState::START)
	{
		NumberRenderer* number_renderer = GamesystemDirector::GetInstance()->GetNumberRenderer();
		number_renderer->RenderInt(SCast(int, count_down_time) + 1, { width,height }, { 0.5f,0.5f }, EnumNumberAlignment::CENTER_ALIGNMENT);
	}
}

// 操作していたブロックを盤面に登録
void ObjectBoard::RegisterBlock()
{
	BlockCell& left_cell = next_block.moving_block.LEFT_BLOCK->GetBlockCell();
	next_block.moving_block.LEFT_BLOCK->GetBlockState().TransitionDropState();
	SetBlockColorFromCell(left_cell, next_block.moving_block.LEFT_BLOCK->GetBlockColor());
	block_list.push_back(std::move(next_block.moving_block.LEFT_BLOCK));

	BlockCell& right_cell = next_block.moving_block.RIGHT_BLOCK->GetBlockCell();
	next_block.moving_block.RIGHT_BLOCK->GetBlockState().TransitionDropState();
	SetBlockColorFromCell(right_cell, next_block.moving_block.RIGHT_BLOCK->GetBlockColor());
	block_list.push_back(std::move(next_block.moving_block.RIGHT_BLOCK));
}

// 消去リストを初期化
void ObjectBoard::ClearDeleteBlockList()
{
	// ブロックを消去
	if (erased_block_list.empty())
		return;

	int pop_count = 0;
	for (auto& erased_block : erased_block_list)
	{
		if (!erased_block || erased_block->GetBlockState().state != EnumBlockState::DISAPPEARED)
			continue;

		// 消したいブロックをreset
		erased_block.reset();

		pop_count++;
	}
	SortInList(erased_block_list);
	while (pop_count > 0)
	{
		erased_block_list.pop_back();
		pop_count--;
	}
	erased_block_list.shrink_to_fit();
}

// ブロックを消去リストに送る関数
bool ObjectBoard::MoveToDeletedBlockList()
{
	int		pop_count = 0; // ブロック削除配列に送った個数

	// 削除リストに移動する処理
	for (auto& block : block_list)
	{
		if (block->GetErase())
		{
			block->GetBlockState().TransitionEraseState();
			erased_block_list.push_back(std::move(block));
			pop_count++;
		}
	}

	if (pop_count == 0)	return false;

	// block_list内の余白をできる限り減らしていく処理
	SortInList(block_list);
	for (int i = 0; i < pop_count; i++)
	{
		block_list.pop_back();
	}
	block_list.shrink_to_fit();

	// ブロックを消すことができたので、連鎖数、スコア、消したブロックの数を加算する
	// (ゲームオーバー時を除く)
	if (board_state.state != EnumBoardState::GAME_OVER)
	{
		chain++;
		game_data.deleted_block_count += pop_count;
		game_data.score += CalcScore(pop_count, chain);
		value_ui.at(SCast(size_t, SCORE))->SetIntValue(0, game_data.score);
		value_ui.at(SCast(size_t, DELETED_BLOCKS))->SetIntValue(0, game_data.deleted_block_count);

		if (chain >= 2)
		{
			GamesystemDirector::GetInstance()->GetAudioManager()->PlaySE(EnumSEBank::CHAIN);
			if (chain >= 3)
			{
				Camera* camera = GamesystemDirector::GetInstance()->GetCamera();
				auto& scene_constants = camera->GetSceneConstants();
				scene_constants.post_effect_blend = 2.0f;
			}
		}
		if ((game_mode != EnumGameMode::BONUS)
			&& (game_data.deleted_block_count >= game_data.level_up_block))
			LevelUp();
	}
	return true;
}

// カメラを盤面に近づけたり遠ざけたりする処理
void ObjectBoard::CameraZoom(float elapsed_time)
{
	Camera*				camera		= GamesystemDirector::GetInstance()->GetCamera();
	Camera::TPVData*	tpv			= camera->GetTPVCamera(SCast(size_t, EnumCameraChannel::GAME));
	GamePad*			game_pad	= GamesystemInput::GetInstance()->GetGamePad();

	if (!tpv)	return;

	if(fabsf(game_pad->GetAxisLY()) > FLT_EPSILON)
	{
		tpv->tpv_distance -= game_pad->GetAxisLY();

		if (tpv->tpv_distance < ZOOM_MIN)
			tpv->tpv_distance = ZOOM_MIN;
		else if (tpv->tpv_distance > ZOOM_MAX)
			tpv->tpv_distance = ZOOM_MAX;
	}
	
	if (game_pad->GetButtonDown() & BTN_LEFT_THUMB)
		tpv->tpv_distance = DEFAULT_ZOOM;
}

// カメラの位置を動かす処理
void ObjectBoard::CameraMove(float elapsed_time)
{
	Camera*				camera		= GamesystemDirector::GetInstance()->GetCamera();
	Camera::TPVData*	tpv			= camera->GetTPVCamera(SCast(size_t, EnumCameraChannel::GAME));
	GamePad*			game_pad	= GamesystemInput::GetInstance()->GetGamePad();

	if (!tpv)	return;

	if(fabsf(game_pad->GetAxisRX()) > FLT_EPSILON || fabsf(game_pad->GetAxisRY()) > FLT_EPSILON)
	{
		camera_rot = {
			-game_pad->GetAxisRY(),
			game_pad->GetAxisRX(),
			0.0f
		};

		DirectX::XMVECTOR	v_camera_rot = DirectX::XMVectorScale(DirectX::XMLoadFloat3(&camera_rot), elapsed_time);
		float				angle = DirectX::XMVectorGetX(DirectX::XMVector3Length(v_camera_rot));
		DirectX::XMVECTOR	nv_camera_rot = DirectX::XMVector3Normalize(v_camera_rot);

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
		DirectX::XMVECTOR v_base_direction = DirectX::XMLoadFloat4(&DEFAULT_DIRECTION);
		v_base_direction = DirectX::XMVectorScale(v_base_direction, -1);

		v_direction = DirectX::XMVector3Rotate(v_direction, q_rotation);
		DirectX::XMStoreFloat4(&tpv->tpv_direction, v_direction);
	}

	if (game_pad->GetButtonDown() & BTN_RIGHT_THUMB)
	{
		tpv->tpv_direction = DEFAULT_DIRECTION;
	}
}

// 盤面を揺らす処理
void ObjectBoard::BoardShake(DirectX::XMFLOAT3 shake)
{
	shake_speed = XMFloatCalclation::XMFloat3Add(shake_speed, shake);
}

// 縦方向チェック
void ObjectBoard::VerticalLineCheck(const UINT row_line)
{
	// ブロックのイテレータを格納する配列
	std::vector<UPtrVector<ObjectBlock>::iterator> itr_array;

	// 一応最小行数、最大行数を超えていないかチェック
	if (row_line < MIN_CELL || row_line > MAX_ROW)
	{
		return;
	}

	UINT			continuous_color = 1;					// 色が連続している個数を記録
	UINT			last_column = 0;						// チェックが終了した列数
	EnumBlockColor	last_color = EnumBlockColor::UNDEFINE;	// 直前のブロックの色

	// 判定開始、下から開始
	for (UINT check_column = MAX_COLUMN; check_column >= MIN_CELL; check_column--)
	{
		// チェックするブロックのイテレータを取得
		//UPtrVector<ObjectBlock>::iterator	block_itr;
		//block_itr = GetBlockFromCell(BlockCell(check_row, column_line));

		const EnumBlockColor& block_color = GetBlockColorFromMatrix(BlockCell(row_line, check_column));

		using enum EnumBlockColor;
		// ブロックが存在していなければ、ここから上のブロックも存在していないので終了
		if (block_color == UNDEFINE)
		{
			last_column = check_column + 1;
			break;
		}

		// ブロックが存在していたが、色が違っていた場合
		else if (last_color != block_color)
		{
			last_color = block_color;
			UINT push_count = continuous_color;

			continuous_color = 1;

			// 規定数以上繋がっていなかったらつぎのセルへ
			if (push_count < ERASE_CONDITION)
				continue;

			// つながっていたブロック数分イテレータ配列へ
			for (UINT count = 1; count <= push_count; count++)
			{
				UPtrVector<ObjectBlock>::iterator	move_to_array;
				move_to_array = GetBlockFromCell(BlockCell(row_line, check_column + count));
				itr_array.push_back(move_to_array);
			}
		}

		// 前回のブロックと同じ色だった時
		else
		{
			continuous_color++;
		}
	}

	// 最後の列まで同じ色だったら最後にイテレータ配列に挿入する
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

	// 色が揃っていたブロックを消去状態にしておく
	for (const auto& block_itr : itr_array)
	{
		if (block_itr == block_list.end())	continue;
		EnumBlockState b_state = (*block_itr)->GetBlockState().state;
		(*block_itr)->EraseRegist();
		(*block_itr)->SetColorUndefine();
	}
}

// 横方向チェック
void ObjectBoard::HorizontalLineCheck(const UINT column_line)
{
	// ブロックのイテレータを格納する配列
	std::vector<UPtrVector<ObjectBlock>::iterator> itr_array;

	// 一応最小行数、最大行数を超えていないかチェック
	if (column_line < MIN_CELL || column_line > MAX_COLUMN)
	{
		return;
	}

	UINT			continuous_color = 1;						// 色が連続している個数を記録
	EnumBlockColor	last_color = EnumBlockColor::UNDEFINE;	// 直前のブロックの色

	// 判定開始、左から開始
	for (UINT check_row = MIN_CELL; check_row <= MAX_ROW; check_row++)
	{
		// チェックするブロックのイテレータを取得
		//UPtrVector<ObjectBlock>::iterator	block_itr;
		//block_itr = GetBlockFromCell(BlockCell(check_row, column_line));

		const EnumBlockColor& block_color = GetBlockColorFromMatrix(BlockCell(check_row, column_line));

		using enum EnumBlockColor;
		// ブロックが存在していて、前回のブロックと同じ色だった時
		if (block_color != UNDEFINE && last_color == block_color)
		{
			continuous_color++;
		}

		// ブロックが存在していなかった場合、または色が違っていた場合
		else
		{
			last_color = block_color;
			UINT push_count = continuous_color;

			continuous_color = 1;

			// 規定数以上繋がっていなかったらつぎのセルへ
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

	// 最後の行まで同じ色だったら最後にイテレータ配列に挿入する
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

	// 色が揃っていたブロックを消去状態にしておく
	for (const auto& block_itr : itr_array)
	{
		if (block_itr == block_list.end())	continue;
		EnumBlockState b_state = (*block_itr)->GetBlockState().state;
		(*block_itr)->EraseRegist();
		(*block_itr)->SetColorUndefine();
	}
}

void ObjectBoard::DiagonalLineCheckLU(const UINT column)
{
	// ブロックのイテレータを格納する配列
	std::vector<UPtrVector<ObjectBlock>::iterator> itr_array;

	// 一応最小行数、最大行数を超えていないかチェック
	if (column < MIN_CELL || column > MAX_COLUMN + DIAGONAL_ADJUST)
	{
		return;
	}

	UINT			continuous_color = 1;						// 色が連続している個数を記録
	EnumBlockColor	last_color = EnumBlockColor::UNDEFINE;	// 直前のブロックの色

	// 判定開始、左から開始
	for (UINT check_row = MIN_CELL; check_row <= MAX_ROW; check_row++)
	{
		// チェックするブロックのイテレータを取得
		//UPtrVector<ObjectBlock>::iterator	block_itr;
		//block_itr = GetBlockFromCell(BlockCell(check_row, column_line));

		const UINT column_line = column - (check_row - 1);
		const EnumBlockColor& block_color = GetBlockColorFromMatrix(BlockCell(check_row, column_line));

		using enum EnumBlockColor;
		// ブロックが存在していて、前回のブロックと同じ色だった時
		if (block_color != UNDEFINE && last_color == block_color)
		{
			continuous_color++;
		}

		// ブロックが存在していなかった場合、または色が違っていた場合
		else
		{
			last_color = block_color;
			UINT push_count = continuous_color;

			continuous_color = 1;

			// 規定数以上繋がっていなかったらつぎのセルへ
			if (push_count < ERASE_CONDITION)
				continue;

			for (UINT count = 1; count <= push_count; count++)
			{
				UPtrVector<ObjectBlock>::iterator	move_to_array;
				move_to_array = GetBlockFromCell(BlockCell(check_row - count, column_line + count));
				itr_array.push_back(move_to_array);
			}
		}
	}

	// 最後の行まで同じ色だったら最後にイテレータ配列に挿入する
	if (continuous_color >= 3)
	{
		UINT push_count = continuous_color;
		for (UINT count = 0; count < push_count; count++)
		{
			UPtrVector<ObjectBlock>::iterator	move_to_array;

			const UINT column_line = column - (DIAGONAL_ADJUST - count);
			move_to_array = GetBlockFromCell(BlockCell(MAX_ROW - count, column_line + 1));
			itr_array.push_back(move_to_array);
		}
	}

	// 色が揃っていたブロックを消去状態にしておく
	for (const auto& block_itr : itr_array)
	{
		if (block_itr == block_list.end())	continue;
		EnumBlockState b_state = (*block_itr)->GetBlockState().state;
		(*block_itr)->EraseRegist();
		(*block_itr)->SetColorUndefine();
	}
}

void ObjectBoard::DiagonalLineCheckLD(const UINT column)
{
	// ブロックのイテレータを格納する配列
	std::vector<UPtrVector<ObjectBlock>::iterator> itr_array;

	// 一応最小行数、最大行数を超えていないかチェック
	if (column < MIN_CELL || column > MAX_COLUMN + DIAGONAL_ADJUST)
	{
		return;
	}

	UINT			continuous_color = 1;						// 色が連続している個数を記録
	EnumBlockColor	last_color = EnumBlockColor::UNDEFINE;	// 直前のブロックの色

	// 判定開始、左から開始
	for (UINT check_row = MIN_CELL; check_row <= MAX_ROW; check_row++)
	{
		// チェックするブロックのイテレータを取得
		//UPtrVector<ObjectBlock>::iterator	block_itr;
		//block_itr = GetBlockFromCell(BlockCell(check_row, column_line));

		const UINT column_line = column - (MAX_ROW - check_row);
		const EnumBlockColor& block_color = GetBlockColorFromMatrix(BlockCell(check_row, column_line));

		using enum EnumBlockColor;
		// ブロックが存在していて、前回のブロックと同じ色だった時
		if (block_color != UNDEFINE && last_color == block_color)
		{
			continuous_color++;
		}

		// ブロックが存在していなかった場合、または色が違っていた場合
		else
		{
			last_color = block_color;
			UINT push_count = continuous_color;

			continuous_color = 1;

			// 規定数以上繋がっていなかったらつぎのセルへ
			if (push_count < ERASE_CONDITION)
				continue;

			for (UINT count = 1; count <= push_count; count++)
			{
				UPtrVector<ObjectBlock>::iterator	move_to_array;
				move_to_array = GetBlockFromCell(BlockCell(check_row - count, column_line - count));
				itr_array.push_back(move_to_array);
			}
		}
	}

	// 最後の行まで同じ色だったら最後にイテレータ配列に挿入する
	if (continuous_color >= 3)
	{
		UINT push_count = continuous_color;
		for (UINT count = 0; count < push_count; count++)
		{
			UPtrVector<ObjectBlock>::iterator	move_to_array;

			const UINT column_line = column - count;
			move_to_array = GetBlockFromCell(BlockCell(MAX_ROW - count, column_line));
			itr_array.push_back(move_to_array);
		}
	}

	// 色が揃っていたブロックを消去状態にしておく
	for (const auto& block_itr : itr_array)
	{
		if (block_itr == block_list.end())	continue;
		EnumBlockState b_state = (*block_itr)->GetBlockState().state;
		(*block_itr)->EraseRegist();
		(*block_itr)->SetColorUndefine();
	}
}

// ブロックの配列の間隔を埋める
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

// ゲーム開始
void ObjectBoard::GameStart(int game_mode_id)
{
	JSONEditor* json_editor = JSONEditor::GetInstance();
	AudioManager* audio_manager = GamesystemDirector::GetInstance()->GetAudioManager();
	audio_manager->StopBGM();

	std::string mode_name(magic_enum::enum_name(SCast(EnumGameMode, game_mode_id)).data());

	Parameters	mode_params;
	ParamPtr	mode_setting_params;

	game_mode			= SCast(EnumGameMode, game_mode_id);
	before_game_mode	= SCast(EnumGameMode, game_mode_id);
	board_color = BOARD_COLOR_SET[game_mode_id];

	std::filesystem::path json_path("resources/json_data/mode_data.json");
	json_editor->ImportJSON(json_path, &mode_params);
	mode_setting_params = GET_PARAMETER_IN_PARAMPTR(mode_name, Parameters, &mode_params);

	game_data.init_level	= SCast(UINT, *(GET_PARAMETER_IN_PARAMPTR("InitLevel", int, mode_setting_params)));
	game_data.max_level		= SCast(UINT, *(GET_PARAMETER_IN_PARAMPTR("MaxLevel", int, mode_setting_params)));
	level_speed				= *(GET_PARAMETER_IN_PARAMPTR("StartSpeed", float, mode_setting_params));
	speed_increase_factor	= *(GET_PARAMETER_IN_PARAMPTR("SpeedIncrease", float, mode_setting_params));
	si_rank_bonus			= *(GET_PARAMETER_IN_PARAMPTR("SIRank", float, mode_setting_params));
	stand_decrease_factor	= *(GET_PARAMETER_IN_PARAMPTR("StandDecrease", float, mode_setting_params));

	ParamPtr speed_rank_params = GET_PARAMETER_IN_PARAMPTR("SpeedRank", Parameters, mode_setting_params);

	for (int i = 0; i < 6; i++)
	{
		int rank = *(GET_PARAMETER_IN_PARAMPTR(std::to_string(i), int, speed_rank_params));
		if ((rank != -1) && (current_rank == -1))
			current_rank = i;
		speed_rank.push_back(rank);
	}

	game_data.deleted_block_count	= 0;
	game_data.level_up_block		= LV_UP_BLOCK_COUNT;
	game_data.speed_level			= game_data.init_level;
	game_data.score					= 0;

	if (game_mode == EnumGameMode::TEMPEST)
		waiting_time_limit = TEMPEST_WAIT_TIME;

	current_speed = 0.0f;

	value_ui.at(SCast(size_t, EnumValueUIIndex::SPEED_LEVEL))->SetIntValue(0, 1, 0.0f);
	value_ui.at(SCast(size_t, EnumValueUIIndex::DELETED_BLOCKS))->SetIntValue(0, 0, 0.0f);
	value_ui.at(SCast(size_t, EnumValueUIIndex::SCORE))->SetIntValue(0, 0, 0.0f);
	board_state.TransitionStartState(game_mode_id);
}

void ObjectBoard::BonusStart(bool easy_mode)
{
	GamesystemDirector::GetInstance()->GetAudioManager()->StopBGM();
	game_data.level_up_block		= INT_MAX;

	switch (before_game_mode)
	{
	case EnumGameMode::NORMAL:
		level_speed = BONUS_SPEED;
		break;
	case EnumGameMode::EASY:
		level_speed = EASY_BONUS_SPEED;
		break;
	case EnumGameMode::IMPACT:
		level_speed = SPEED_LIMIT;
		break;
	case EnumGameMode::ENDLESS:
		break;
	case EnumGameMode::MISSION:
		break;
	case EnumGameMode::TEMPEST:
		standing_time_limit = 0.2f;
		break;
	case EnumGameMode::BONUS:
		break;
	case EnumGameMode::UNDEFINE:
		break;
	default:
		break;
	}

	object_time = 60.0f;
	
	board_state.TransitionStartState(SCast(int, EnumGameMode::BONUS));
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
}

// 天井に到達したかをチェックする関数
bool ObjectBoard::CheckGameOver()
{
	bool game_over = false;
	for (UINT row = 1; row <= MAX_ROW; row++)		// block_cell番号は1から始まるためこのfor文も1で開始
	{
		game_over = IsExistFromColorMatrix(BlockCell(row, 1));
		if (game_over)
		{
			break;
		}
	}
	return game_over;
}

void ObjectBoard::LevelUp()
{
	AudioManager* audio_manager = GamesystemDirector::GetInstance()->GetAudioManager();
	if ((game_data.speed_level >= game_data.max_level))
	{
		if (game_mode == EnumGameMode::ENDLESS)
		{
			game_data.level_up_block = UINT_MAX;
			level_speed = SPEED_LIMIT;
			ui_color = { 1.0f, 1.0f, 0.0f };
		}
		else
		{
			board_state.TransitionGameOverState(true);
			ui_color = { 1.0f, 1.0f, 0.0f };
		}
	}
	else
	{
		do
		{
			game_data.speed_level++;
			game_data.level_up_block += LV_UP_BLOCK_COUNT;
			level_speed += speed_increase_factor;
			standing_time_limit -= stand_decrease_factor;

			if ((current_rank < 5) && (game_data.speed_level >= speed_rank[current_rank + 1]))	// スピードランクごとのレベル到達でBGM変化
			{
				current_rank++;
				level_speed += si_rank_bonus;

				audio_manager->PlaySE(EnumSEBank::RANK_UP);
				audio_manager->StopBGM();
				audio_manager->PlayBGM(SCast(EnumBGMBank, current_rank), true);
			}

			if ((game_data.speed_level < 100) && (game_data.speed_level % 10 == 0))
			{
				flag_system.SetFlag(EnumBoardFlags::CHANGE_PARTICLE_TYPE, true);
			}
		} while ((game_data.deleted_block_count >= game_data.level_up_block) && game_data.speed_level < game_data.max_level);

		if (level_speed > SPEED_LIMIT)
			level_speed = SPEED_LIMIT;

		audio_manager->PlaySE(EnumSEBank::LEVEL_UP);
		int level	= SCast(int, game_data.speed_level) * 1000;
		int section = SCast(int, (current_rank < 5 && speed_rank[current_rank + 1] != -1) ? speed_rank[current_rank + 1] : game_data.max_level);
		value_ui.at(SCast(size_t, SPEED_LEVEL))->SetIntValue(0,level + section);
	}
}

void ObjectBoard::UpdateStandCollisionHeight()
{
	for (UINT row = 0; row < MAX_ROW; row++)
	{
		int column = MAX_COLUMN - 1;
		while (IsExistFromColorMatrix(BlockCell(row + 1, column + 1)))
		{
			if (column <= 0)	break;
			column--;
		}
		stand_collision_heignt[row] = SCast(UINT, column);
	}
}

// スコアを計算
UINT ObjectBoard::CalcScore(UINT block_count, UINT chain)
{
	return (SCORE_BASE + (game_data.speed_level - 1)) * block_count * (chain * chain);
}

// 指定したセルにブロックが存在するかを色の二次元配列から取得
const bool ObjectBoard::IsExistFromColorMatrix(BlockCell cell)
{
	if (cell.row < 1 || cell.row > MAX_ROW)				return true;
	if (cell.column < 1 || cell.column > MAX_COLUMN)	return true;

	return block_color_matrix[cell.row - 1][cell.column - 1] != EnumBlockColor::UNDEFINE;
}

// 指定したセルのイテレータが無効でないかの判定
const bool ObjectBoard::IsInvalidBlockFromCell(BlockCell cell)
{
	auto itr = GetBlockFromCell(cell);
	bool invalid = (itr == block_list.end());
	return invalid;
}

// イテレータが無効でないかの判定
const bool ObjectBoard::IsInvalidBlockFromIterator(UPtrVector<ObjectBlock>::iterator itr)
{
	bool invalid = itr == block_list.end();
	return invalid;
}

// 指定したセルからブロックのイテレータを取得
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

const EnumBlockColor& ObjectBoard::GetBlockColorFromMatrix(const BlockCell cell)
{
	if (cell.row < 1 || cell.row > MAX_ROW)					return EnumBlockColor::UNDEFINE;
	else if (cell.column < 1 || cell.column > MAX_COLUMN)	return EnumBlockColor::UNDEFINE;
	return block_color_matrix[cell.row - 1][cell.column - 1];
}