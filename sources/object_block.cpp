// <>インクルード
#include <random>
#include <imgui.h>
#include <math.h>
#include <magic_enum_iostream.hpp>

// ""インクルード
#include "game_system/gamesystem_director.h"
#include "game_system/gamesystem_input.h"
#include "object_block.h"
#include "object_board.h"
#include "xmfloat_calclation.h"
#include "shader.h"

// 定数
const size_t TEXT_SIZE = 256;

//---------------------------------------------------------------------------------------
//		BoardState Transition系
//---------------------------------------------------------------------------------------

// まだ盤面に出ていない状態
void ObjectBlock::BlockState::TransitionNextState()
{
	state = EnumBlockState::NEXT;

	obj->state_update = nullptr;
}

// 盤面に出現させるとき
void ObjectBlock::BlockState::TransitionMoveState(EnumBlockPlace place)
{
	if (place == EnumBlockPlace::LEFT)
	{
		obj->block_cell = LEFT_START;
		if (obj->parent_board.IsExistingBlockFromCell(LEFT_START))
		{
			return;
		}
		obj->parent_board.SetExistingBlockFromCell(LEFT_START, true);
		obj->flag_system.SetFlag(EnumBlockFlags::BLINK, true);
	}
	else if (place == EnumBlockPlace::RIGHT)
	{
		obj->block_cell = RIGHT_START;
		if (obj->parent_board.IsExistingBlockFromCell(RIGHT_START))
		{
			return;
		}
		obj->parent_board.SetExistingBlockFromCell(RIGHT_START, true);
	}

	obj->ghost_cell.row		= obj->block_cell.row;
	obj->ghost_cell.column	= obj->parent_board.GetStandCollisionHeight(obj->block_cell.row - 1) + 1;
	state = EnumBlockState::MOVE;
	obj->AdjustFromBlockCell();

	obj->flag_system.SetFlag(EnumBlockFlags::MOVING, true);
	obj->flag_system.SetFlag(EnumBlockFlags::FIRING_PARTICLE, true);
	obj->state_update = &ObjectBlock::UpdateMoveState;

	if (obj->parent_board.GetStandCollisionHeight(obj->block_cell.row - 1) == 0)
		TransitionStandState();
}

// 操作中のブロックが接地したとき
void ObjectBlock::BlockState::TransitionStandState()
{
	AudioManager* audio_manager = GamesystemDirector::GetInstance()->GetAudioManager();
	state = EnumBlockState::STAND;

	obj->ResetShiftY();

	audio_manager->PlaySE(EnumSEBank::STAND);
	obj->flag_system.SetFlag(EnumBlockFlags::STANDING, true);
	obj->flag_system.SetFlag(EnumBlockFlags::FIRING_PARTICLE, false);

	obj->parent_board.BoardShake({ 0.0f,SHAKE_FACTOR,0.0f });

	obj->state_update = &ObjectBlock::UpdateStandState;
}

// ブロックが落下するとき
void ObjectBlock::BlockState::TransitionDropState()
{
	state = EnumBlockState::DROP;

	obj->flag_system.SetFlag(EnumBlockFlags::MOVING, false);
	obj->flag_system.SetFlag(EnumBlockFlags::STANDING, false);
	obj->flag_system.SetFlag(EnumBlockFlags::BLINK, false);
	obj->flag_system.SetFlag(EnumBlockFlags::FIRING_PARTICLE, true);

	obj->blink_time = 0.0f;

	obj->state_update = &ObjectBlock::UpdateDropState;
}

// ブロックが落下後静止するとき
void ObjectBlock::BlockState::TransitionPutState()
{
	if(state != EnumBlockState::PUT)
	{
		AudioManager* audio_manager = GamesystemDirector::GetInstance()->GetAudioManager();
		audio_manager->PlaySE(EnumSEBank::PUT);
	}

	state = EnumBlockState::PUT;

	obj->ResetShiftY();


	obj->flag_system.SetFlag(EnumBlockFlags::FIRING_PARTICLE, false);

	obj->state_update = &ObjectBlock::UpdatePutState;
}

// ブロックが消去されるとき
void ObjectBlock::BlockState::TransitionEraseState()
{
	state = EnumBlockState::ERASE;
	obj->parent_board.SetExistingBlockFromCell(obj->block_cell, false);

	obj->block_particle->GetCbParticleEmitter().emit_color.w = 0.0f;

	GamesystemDirector::GetInstance()->SetRadialBlur();
	GamesystemDirector::GetInstance()->GetAudioManager()->PlaySE(EnumSEBank::ERASE);

	obj->state_update = nullptr;
}

// ブロックが消滅したとき
void ObjectBlock::BlockState::TransitionDisappearedState()
{
	state = EnumBlockState::DISAPPEARED;
	obj->state_update = nullptr;
}

//---------------------------------------------------------------------------------------
//		ObjectBlock メンバ関数
//---------------------------------------------------------------------------------------

// ブロックを操作しているとき
void ObjectBlock::UpdateMoveState(float elapsed_time)
{
	bool lowest = false;
	if (block_cell.column >= MAX_COLUMN) lowest = true;
	bool fall = JudgeFallBlock();

	if (flag_system.GetFlag(EnumBlockFlags::BLINK))
		blink_time += elapsed_time;

	if (!lowest && !fall)
	{
		block_state.TransitionStandState();
	}
	else if (!lowest && fall)
		block_cell.shift_y += parent_board.GetCurrentSpeed();
	else
		block_state.TransitionStandState();

}

// ブロックが接地していて、まだ操作可能なとき
void ObjectBlock::UpdateStandState(float elapsed_time)
{
	if (flag_system.GetFlag(EnumBlockFlags::BLINK))
		blink_time += elapsed_time;
}

// ブロックが落下中のとき
void ObjectBlock::UpdateDropState(float elapsed_time)
{
	bool lowest = false;
	if (block_cell.column >= MAX_COLUMN) lowest = true;
	flag_system.SetFlag(EnumBlockFlags::BLINK, false);

	bool fall = JudgeFallBlock();
	if (!lowest && fall)
	{
		auto under_block_itr = parent_board.GetBlockFromCell(UNDER_CELL);
		bool moving = true;
		if (!parent_board.IsInvalidBlockFromIterator(under_block_itr))
			moving = (*under_block_itr)->flag_system.GetFlag(EnumBlockFlags::MOVING);
		if (moving)
		{
			block_cell.shift_y += parent_board.GetCurrentSpeed();
		}
		else
		{
			block_state.TransitionPutState();
		}
	}
	else if (lowest)
		block_state.TransitionPutState();
	else
		block_cell.shift_y += parent_board.GetCurrentSpeed();

}

// ブロックが動いていないとき
void ObjectBlock::UpdatePutState(float elapsed_time)
{
	bool lowest = false;
	if (block_cell.column >= MAX_COLUMN) lowest = true;
	bool exist = parent_board.IsExistingBlockFromCell(UNDER_CELL);
	if (!lowest && exist)
	{
		// 下のブロックのイテレータを取得、これにより下のブロックが動いているかどうかを確認できる
		auto under_block_itr = parent_board.GetBlockFromCell(UNDER_CELL);
		bool moving = true;
		if (!parent_board.IsInvalidBlockFromIterator(under_block_itr))
			moving = (*under_block_itr)->flag_system.GetFlag(EnumBlockFlags::MOVING);
		if (moving)
		{
			SetDropping();
		}
	}
	else if (!lowest && !exist)
	{
		SetDropping();
	}
}

// パーティクルの行列を取得
DirectX::XMFLOAT4X4 ObjectBlock::GetParticleTransform() const
{
	DirectX::XMFLOAT4X4 particle_transform;
	DirectX::XMStoreFloat4x4(&particle_transform,
		DirectX::XMLoadFloat4x4(&transform) * DirectX::XMMatrixTranslation(+1, 0, 0));

	return particle_transform;
}

// コンストラクタ
ObjectBlock::ObjectBlock(bool obstacle, ObjectBoard* parent_board)
	: parent_board(*parent_board)
{
	Graphics*				graphics			= Graphics::GetInstance();
	ID3D11Device*			device				= graphics->GetDevice().Get();
	ID3D11DeviceContext*	device_context		= graphics->GetDeviceContext().Get();

	DirectX::XMVECTOR v_local_position = DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&translation), DirectX::XMMatrixInverse(NULL, DirectX::XMLoadFloat4x4(&transform)));
	DirectX::XMFLOAT3 local_position;
	DirectX::XMStoreFloat3(&local_position, v_local_position);

	model			= std::make_unique<GameModel>("resources/model/block/block.gltf");
	model->SetMaskTexture(L"resources/sprite/mask_texture.png");

	if (obstacle)
	{
		block_color = EnumBlockColor::GRAY;
		block_color_factor = BLOCK_COLOR_SET.at(magic_enum::enum_integer(EnumBlockColor::GRAY));
	}
	else
	{
		std::random_device rd;
		int color_number = rd() % 5;
		block_color = magic_enum::enum_value<EnumBlockColor>(color_number);
		block_color_factor = BLOCK_COLOR_SET.at(color_number);
	}

	blink_time = 0.0f;
	
	primitive_renderer	= std::make_unique<PrimitiveRenderer>();

	ParticleSystem::CbParticleEmitter emitter = {};
	emitter.emit_amounts		= 300;
	emitter.disable				= true;
	emitter.emit_position.x		= translation.x;
	emitter.emit_position.y		= translation.y - BLOCK_SIZE * 0.5f;
	emitter.emit_position.z		= translation.z - BLOCK_SIZE * 0.25f;
	emitter.emit_color.x		= block_color_factor.x;
	emitter.emit_color.y		= block_color_factor.y;
	emitter.emit_color.z		= block_color_factor.z;

	emitter.emit_size	= 0.2f;
	emitter.spread_rate = 0.0f;
	emitter.emit_speed	= parent_board->GetCurrentSpeed();
	emitter.emit_accel	= -3.0f;
	emitter.life_time	= PARTICLE_LIFE;
	emitter.start_diff	= 0.01f;
	emitter.emit_radius = BLOCK_SIZE * 0.5f;
	block_particle		= std::make_unique<ParticleSystem>(emitter , true, "accumulate_particle_ps.cso");

	flag_system.SetFlag(EnumBlockFlags::OBSTACLE_BLOCK, obstacle);

	Shader::CreatePSFromCso("shader/ghost_block_ps.cso", ghost_pixel_shader.ReleaseAndGetAddressOf());

	if(parent_board)	
	{
		block_state.obj = this;
		block_state.TransitionNextState();
	}
}

// デストラクタ
ObjectBlock::~ObjectBlock()
{
}

// 更新処理
void ObjectBlock::Update(float elapsed_time)
{
	AdjustFromBlockCell();

	if (state_update)
		(this->*state_update)(elapsed_time);

	ParticleSystem::CbParticleEmitter& emitter = block_particle->GetCbParticleEmitter();

	emitter.emit_position.x = translation.x;
	emitter.emit_position.y = translation.y + BLOCK_SIZE * 0.5f;

	if (flag_system.GetFlag(EnumBlockFlags::FIRING_PARTICLE))
		emitter.disable = false;
	else
		emitter.disable = true;

	block_particle->Update(elapsed_time);

	model->InstanceUpdate();

	UpdateTransform();
}

// ブロックが消えている最中に実行される関数
void ObjectBlock::ErasingUpdate(float elapsed_time)
{
	Graphics* graphics = Graphics::GetInstance();

	float& disolve_factor = model->GetDisolveFactor();
	disolve_factor -= elapsed_time;
	
	block_particle->Update(elapsed_time);
	if (disolve_factor < 0.0f)
		block_state.TransitionDisappearedState();
}

void ObjectBlock::Render()
{
	Graphics* graphics = Graphics::GetInstance();

	// モデルの描画
	model->SetBlinkFactor(fabsf(sinf(blink_time * BLINK_SPEED)) * 0.3f);

	graphics->SetDepthStencilState(EnumDepthState::ZT_ON_ZW_ON);
	graphics->SetBlendState(EnumBlendState::ALPHA, nullptr, 0xFFFFFFFF);
	graphics->SetRasterizerState(EnumRasterizerState::SOLID);

	if((block_state.state == EnumBlockState::MOVE)
		|| (block_state.state == EnumBlockState::STAND))
	{
		DirectX::XMVECTOR v_ghost_translate = DirectX::XMLoadFloat3(&ghost_translation);
		DirectX::XMMATRIX m_ghost_translate = DirectX::XMMatrixTranslationFromVector(v_ghost_translate);

		DirectX::XMMATRIX m_scale = DirectX::XMMatrixScaling(0.5f, 0.5f, 0.5f);

		DirectX::XMMATRIX m_ghost_transform = m_scale * m_ghost_translate;
		DirectX::XMFLOAT4X4 ghost_transform;

		DirectX::XMStoreFloat4x4(&ghost_transform, m_ghost_transform);
		model->Render(false, ghost_transform, block_color_factor, graphics->GetPixelShader(EnumPixelShader::EXTRACT_EMISSIVE).Get());
	}

	model->Render(false, transform, block_color_factor);

	graphics->SetDepthStencilState(EnumDepthState::ZT_ON_ZW_OFF);
	graphics->SetBlendState(EnumBlendState::ALPHA, nullptr, 0xFFFFFFFF);
	graphics->SetRasterizerState(EnumRasterizerState::CULL_NONE);

	// ブロックの軌跡の描画
	block_particle->Render();
}

// エミッシブ成分のみ描画
void ObjectBlock::EmissiveRender()
{
	Graphics* graphics = Graphics::GetInstance();

	// モデルの描画
	graphics->SetDepthStencilState(EnumDepthState::ZT_ON_ZW_ON);
	graphics->SetRasterizerState(EnumRasterizerState::CULL_NONE);
	graphics->SetBlendState(EnumBlendState::ALPHA, nullptr, 0xFFFFFFFF);

	model->SetBlinkFactor(0.0f);
	
	model->Render(false, transform, block_color_factor, graphics->GetPixelShader(EnumPixelShader::EXTRACT_EMISSIVE).Get());
	
	block_particle->Render();
}

// ブロック位置からセル位置を補正
void ObjectBlock::AdjustFromBlockPosition()
{
	if (-translation.y > MAX_COLUMN * BLOCK_SIZE)
		translation.y = -SCast(int, MAX_COLUMN) * BLOCK_SIZE;
	block_cell =
		BlockCell(SCast(UINT, translation.x / BLOCK_SIZE), SCast(UINT, (-(translation.y + block_cell.shift_y)) / BLOCK_SIZE));
}

// セル位置からブロック位置を補正
void ObjectBlock::AdjustFromBlockCell()
{
	DirectX::XMFLOAT3 shake = parent_board.GetShakePosiiton();

	if (block_cell.column >= MAX_COLUMN)
	{
		block_cell.column = MAX_COLUMN;
		block_cell.shift_y = 0;
	}
	else if (!parent_board.IsInvalidBlockFromCell(UNDER_CELL))
	{
		block_cell.shift_y = 0;
	}

	translation.x = (SCast(float, block_cell.row) * BLOCK_SIZE) - shake.x + parent_board.GetRootPosition().x;
	translation.y = -(SCast(float, block_cell.column) * BLOCK_SIZE  + block_cell.shift_y) - shake.y + parent_board.GetRootPosition().y;
	translation.z = parent_board.GetRootPosition().z;

	ghost_translation.x = (SCast(float, ghost_cell.row) * BLOCK_SIZE) - shake.x + parent_board.GetRootPosition().x;
	ghost_translation.y = -(SCast(float, ghost_cell.column) * BLOCK_SIZE) - shake.y + parent_board.GetRootPosition().y;
	ghost_translation.z = parent_board.GetRootPosition().z;
}

// ブロックの移動値を初期化
void ObjectBlock::ResetShiftY()
{
	block_cell.shift_y = 0.0f;
	AdjustFromBlockCell();
}

// ブロックのパーティクルをモデルに沿わせる処理
void ObjectBlock::AccumulateBlockParticle(ID3D11PixelShader* accumlate_ps)
{
	Graphics* graphics = Graphics::GetInstance();

	graphics->SetDepthStencilState(EnumDepthState::ZT_ON_ZW_ON);
	graphics->SetRasterizerState(EnumRasterizerState::SOLID);
	graphics->SetBlendState(EnumBlendState::ALPHA, nullptr, 0xFFFFFFFF);

	DirectX::XMFLOAT4X4 particle_transform;
	DirectX::XMStoreFloat4x4(&particle_transform,
		DirectX::XMLoadFloat4x4(&transform) * DirectX::XMMatrixTranslation(+1, 0, 0));

	model->Render(false, particle_transform, block_color_factor, accumlate_ps);
}

void ObjectBlock::FollowPartnerBlock(EnumBlockRotation rotation, BlockCell& following_cell)
{
	switch (rotation)
	{
	using enum EnumBlockRotation;
	case RIGHT:
		block_cell.row = following_cell.row + 1;
		break;
	case UNDER:
		block_cell.column = following_cell.column + 1;
		break;
	case LEFT:
		block_cell.row = following_cell.row - 1;
		break;
	case TOP:
		block_cell.column = following_cell.column - 1;
		break;
	default:
		break;
	}

	block_cell.shift_y = following_cell.shift_y;

	AdjustFromBlockCell();
}

// ブロックを動かす
void ObjectBlock::MoveBlock(GamePadButton input)
{
	UINT old_row = block_cell.row;
	parent_board.SetExistingBlockFromCell(block_cell, false);

	if (input & BTN_LEFT)
		block_cell.row--;
	else if (input & BTN_RIGHT)
		block_cell.row++;

	// プレイヤーIDを使ってコントローラーを取得
	const UINT			ID = parent_board.GetPlayerID();
	GamesystemInput*	gamesystem_input = GamesystemInput::GetInstance();

	EnumBlockRotation block_rotate = gamesystem_input->GetBlockRotation(ID);
	
	switch (block_rotate)
	{
		using enum EnumBlockRotation;
	case RIGHT:
	case LEFT:
		ghost_cell.row		= block_cell.row;
		ghost_cell.column	= parent_board.GetStandCollisionHeight(block_cell.row - 1) + 1;
		if (ghost_cell.column < 0)	ghost_cell.column = 0;
		break;
	case UNDER:
		ghost_cell.row		= block_cell.row;
		ghost_cell.column	= parent_board.GetStandCollisionHeight(block_cell.row - 1) 
								+ (flag_system.GetFlag(EnumBlockFlags::BLINK) ? 0 : 1);
		if (ghost_cell.column < 0)	ghost_cell.column = 0;
		break;
	case TOP:
		ghost_cell.row = block_cell.row;
		ghost_cell.column = parent_board.GetStandCollisionHeight(block_cell.row - 1)
			+ (flag_system.GetFlag(EnumBlockFlags::BLINK) ? 1 : 0);
		if (ghost_cell.column < 0)	ghost_cell.column = 0;
		break;
	}

	AdjustFromBlockCell();
	parent_board.SetExistingBlockFromCell(block_cell, true);

	if (old_row != block_cell.row)
	{
		if (block_cell.row == 1)
			parent_board.BoardShake({ +SHAKE_FACTOR,0.0f,0.0f });
		else if (block_cell.row == MAX_ROW)
			parent_board.BoardShake({ -SHAKE_FACTOR,0.0f,0.0f });
	}
}

// 回転のときにブロックを上に上げる処理
void ObjectBlock::LiftBlock(EnumBlockRotation rotation)
{
	parent_board.SetExistingBlockFromCell(block_cell, false);
	switch (rotation)
	{
	using enum EnumBlockRotation;
	case RIGHT:
		block_cell.row--;
		break;

	case UNDER:
		block_cell.column--;
		block_cell.shift_y = 0.0f;
		break;

	case LEFT:
		block_cell.row++;
		break;

	default:
		break;
	}
	ghost_cell.row		= block_cell.row;
	ghost_cell.column	= parent_board.GetStandCollisionHeight(block_cell.row - 1) + 1;
	AdjustFromBlockCell();
	parent_board.SetExistingBlockFromCell(block_cell, false);
}

// ブロックを回転させる処理
bool ObjectBlock::RotateBlock(ObjectBlock* root_block, EnumBlockRotation rotation, bool flip)
{
	using enum EnumBlockRotation;
	BlockCell& root_cell = root_block->GetBlockCell();
	const BlockCell& CELL_L = BlockCell(root_cell.row - 1, root_cell.column);
	const BlockCell& CELL_UL = BlockCell(root_cell.row - 1, root_cell.column + 1);

	const BlockCell& CELL_R = BlockCell(root_cell.row + 1, root_cell.column);
	const BlockCell& CELL_UR = BlockCell(root_cell.row + 1, root_cell.column + 1);

	const BlockCell& CELL_DU = BlockCell(root_cell.row, root_cell.column + 2);

	size_t rotation_index = SCast(size_t, rotation);

	BlockCell& root_ghost = root_block->GetGhostCell();

	switch (rotation)
	{
	case RIGHT:
		if (block_cell.row == 1 || parent_board.IsExistingBlockFromCell(CELL_R))
		{
			if (!parent_board.IsExistingBlockFromCell(CELL_L))
			{
				root_block->LiftBlock(rotation);
				block_cell.shift_y = 0.0f;
				
				const BlockCell moved_root_cell = root_block->block_cell;
				if(parent_board.IsExistingBlockFromCell(
					BlockCell(moved_root_cell.row, moved_root_cell.column + 1)))
					root_block->GetBlockState().TransitionStandState();
			}
			else
				return false;
		}
		else if (parent_board.IsExistingBlockFromCell(CELL_UR))
		{
			root_cell.shift_y = 0.0f;
			block_cell.shift_y = 0.0f;
			root_block->GetBlockState().TransitionStandState();
		}

		break;

	case UNDER:
		if (block_state.state == EnumBlockState::STAND)
			root_block->LiftBlock(rotation);
		else if (parent_board.IsExistingBlockFromCell(CELL_DU))
		{
			if(!flip)
			{
				root_cell.shift_y = 0.0f;
				block_cell.shift_y = 0.0f;
				root_block->GetBlockState().TransitionStandState();
			}
		}

		break;

	case LEFT:
		if (block_cell.row == 1 || parent_board.IsExistingBlockFromCell(CELL_L))
		{
			if (!parent_board.IsExistingBlockFromCell(CELL_R))
			{
				root_block->LiftBlock(rotation);
				block_cell.shift_y = 0.0f;

				const BlockCell moved_root_cell = root_block->block_cell;
				if (parent_board.IsExistingBlockFromCell(
					BlockCell(moved_root_cell.row, moved_root_cell.column + 1)))
					root_block->GetBlockState().TransitionStandState();
			}
			else
				return false;
		}
		else if (parent_board.IsExistingBlockFromCell(CELL_UL))
		{
			root_cell.shift_y = 0.0f;
			block_cell.shift_y = 0.0f;
			root_block->GetBlockState().TransitionStandState();
		}

		break;

	case TOP:
		root_block->LiftBlock(rotation);
		break;

	default:
		break;
	}


	if(!flip)
	{
		BlockCell rotated_cell = root_block->block_cell + ROTATION_CELL[rotation_index];
		parent_board.SetExistingBlockFromCell(block_cell, false);
		block_cell = rotated_cell;
		parent_board.SetExistingBlockFromCell(block_cell, true);
	}
	else
	{
		BlockCell temp_cell = root_cell;
		root_cell = block_cell;
		block_cell = temp_cell;
	}

	// ゴーストの位置変更
	ghost_cell.row		= block_cell.row;
	ghost_cell.column	= parent_board.GetStandCollisionHeight(block_cell.row - 1) 
							+ (rotation == EnumBlockRotation::TOP ? 0 : 1);

	const BlockCell& moved_root_cell = root_block->block_cell;
	root_ghost.row		= moved_root_cell.row;
	root_ghost.column	= parent_board.GetStandCollisionHeight(moved_root_cell.row - 1) 
							+ (rotation == EnumBlockRotation::UNDER ? 0 : 1);
	AdjustFromBlockCell();

	return true;
}

// ブロックが落ちることができるかどうかを判定するための関数
bool ObjectBlock::JudgeFallBlock()
{
	if (block_cell.shift_y <= BLOCK_SIZE)		return true;
	parent_board.SetExistingBlockFromCell(block_cell, false);

	EnumBlockRotation rotation		= GamesystemInput::GetInstance()->GetBlockRotation(parent_board.GetPlayerID());
	EnumBlockRotation top_rotation	= 
		flag_system.GetFlag(EnumBlockFlags::BLINK) 
		? EnumBlockRotation::UNDER : EnumBlockRotation::TOP;

	const UINT STAND_HEIGHT = parent_board.GetStandCollisionHeight(block_cell.row - 1);
	const UINT MODIFIED_MAX_COLUMN = flag_system.GetFlag(EnumBlockFlags::MOVING) && (rotation == top_rotation) ? STAND_HEIGHT : STAND_HEIGHT + 1;

	while (block_cell.shift_y >= BLOCK_SIZE)
	{
		if (block_cell.column < MODIFIED_MAX_COLUMN)
		{
			block_cell.shift_y -= BLOCK_SIZE;
			block_cell.column++;
		}
		else
		{
			block_cell.shift_y = 0.0f;
			block_cell.column = MODIFIED_MAX_COLUMN;
		}
	}

	AdjustFromBlockCell();
	parent_board.SetExistingBlockFromCell(block_cell, true);

	auto under_block = parent_board.GetBlockFromCell(UNDER_CELL);
	bool invalid = 
		parent_board.IsInvalidBlockFromIterator(under_block) 
		|| ((*under_block)->block_state.state == EnumBlockState::ERASE)
		|| ((*under_block)->block_state.state == EnumBlockState::DISAPPEARED);

	return invalid;
}

// ImGui表示
void ObjectBlock::DebugGUI()
{
	bool debug_b = flag_system.GetFlag(EnumBlockFlags::MOVING);
	ImGui::Checkbox("Moving", &debug_b);

	debug_b = flag_system.GetFlag(EnumBlockFlags::OBSTACLE_BLOCK);
	ImGui::Checkbox("Obstacle Block", &debug_b);

	ImGui::DragFloat2("Block Position", &translation.x);

	UINT block_cell_array[2] = { block_cell.row,block_cell.column };
	ImGui::InputInt2("Block Cell", RCast(int*, block_cell_array));

	ImGui::InputFloat("Shift Y", &block_cell.shift_y);

	char block_color_str[TEXT_SIZE] = "Block Color : ";
	auto color = magic_enum::enum_name(block_color);
	strcat_s(block_color_str, sizeof(block_color_str), color.data());
	ImGui::Text(block_color_str);

	char block_state_str[TEXT_SIZE] = "Block State : ";
	auto state = magic_enum::enum_name(block_state.state);
	strcat_s(block_state_str, sizeof(block_state_str), state.data());
	ImGui::Text(block_state_str);
}

// 落下状態に設定する関数
void ObjectBlock::SetDropping()
{
	if (block_state.state != EnumBlockState::DROP)
		block_state.TransitionDropState();

	// 自ブロックより上のブロックを落下状態にする再帰処理関数
	auto upper_itr = parent_board.GetBlockFromCell(UPPER_CELL);
	if (!parent_board.IsInvalidBlockFromIterator(upper_itr))
		(*upper_itr)->SetDropping();
}