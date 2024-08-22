#ifndef __OBJECT_BLOCK_H__
#define __OBJECT_BLOCK_H__

// <>�C���N���[�h
#include <magic_enum_containers.hpp>

// ""�C���N���[�h
// LightBlueEngine
#include "game_object/game_object.h"
#include "graphics/light.h"
#include "graphics/particle_system.h"
#include "graphics/primitive_renderer.h"
#include "input/input.h"

// �O���錾
class		ObjectBoard;
enum class	EnumBlockRotation;

// �萔
const UINT MIN_CELL		= 1;
const UINT MAX_ROW		= 8;
const UINT MAX_COLUMN	= 16;

const std::vector<DirectX::XMFLOAT3> BLOCK_COLOR_SET = {
	{1.0f,0.0f,0.0f},		// RED
	{0.0f,0.5f,1.0f},		// BLUE
	{0.0f,1.0f,0.0f},		// GREEN
	{1.0f,1.0f,0.0f},		// YELLOW
	{1.0f,0.0f,1.0f},		// PURPLE
	{0.5f,0.5f,0.5f}		// GRAY
};

// enum
enum class EnumBlockFlags
{
	BLINK,				// ���݉�]���̃u���b�N�ł��邩�ǂ������킩��₷�����邽��
	MOVING,				// ���ݓ������Ă���u���b�N���ǂ���
	OBSTACLE_BLOCK,		// ������܃u���b�N(�D�F�̃u���b�N)���ǂ��� �������V���O���ł͎g��Ȃ�
	CHECKED,			// �u���b�N�����̎��Ɏg�p
	STANDING,			// �d�������ǂ���
	RECORD_TRAIL,
	FIRING_PARTICLE,
	SPLASHED_PARTICLE,
	ERASE,
	ERASING,
	BLACK_OUT,
};


enum class EnumBlockState
{
	NEXT,
	MOVE,
	STAND,
	DROP,
	PUT,
	ERASE,
	DISAPPEARED
};

enum class EnumBlockPlace
{
	LEFT,
	RIGHT,
	NONE
};

// struct >> [BlockCell]
struct BlockCell
{
	// �R���X�g���N�^
	BlockCell() {}
	BlockCell(UINT i_row, UINT i_column) : shift_y(0.0f), row(i_row), column(i_column) {}

	// operator

	// �u���b�N�̈ʒu�ݒ�p
	BlockCell operator ()(UINT i_row, UINT i_column)
	{
		this->row		= i_row;
		this->column	= i_column;
		return *this;
	}

	// �u���b�N�T���p
	bool operator ==(UINT cell[2]) const
	{
		if (this->row		!= cell[0])	return false;
		if (this->column	!= cell[1])	return false;
		return true;
	}

	bool operator ==(BlockCell cell) const
	{
		if (this->row		!= cell.row)		return false;
		if (this->column	!= cell.column)	return false;
		return true;
	}

	// �u���b�N��]�p
	BlockCell operator +(const std::pair<int, int> cell) const
	{
		BlockCell ret_cell;
		ret_cell.shift_y	= this->shift_y;
		ret_cell.row		= this->row + cell.first;
		ret_cell.column		= this->column + cell.second;
		return ret_cell;
	}

	// TIPS
	//	  �P�Q�R�S�T�U ��row
	//	1 ������������
	//	2 ������������
	//	3 ������������
	//	| �b�b�b�b�b�b
	// 18 ������������
	// 19 ������������
	// 20 ������������
	// ��
	// COLUMN
	// ���܂��Ֆʂɑ��݂��Ă��Ȃ��Ƃ��� row = 0�Acolumn = 0�Ƃ���

	// �ϐ�
	float	shift_y = 0.0f;
	UINT	row		= 0;
	UINT	column	= 0;
};

// BlockCell�̒萔
// �y�A�u���b�N�̃X�^�[�g�ʒu
const BlockCell LEFT_START	= BlockCell(4, 1);
const BlockCell RIGHT_START = BlockCell(5, 1);

// ��]
const std::vector<std::pair<int, int>> ROTATION_CELL =
{
	std::pair<int,int>(+1, 0),
	std::pair<int,int>(0,+1),
	std::pair<int,int>(-1, 0),
	std::pair<int,int>(0,-1)
};

// class >> [ObjectBlock] �p���FGameObject
class ObjectBlock : public GameObject
{
private:
	// �}�N��
#define UPPER_CELL			BlockCell(block_cell.row,		block_cell.column - 1)
#define UNDER_CELL			BlockCell(block_cell.row,		block_cell.column + 1)
#define LEFT_CELL			BlockCell(block_cell.row - 1,	block_cell.column)
#define RIGHT_CELL			BlockCell(block_cell.row + 1,	block_cell.column)

	// private:�\����
	static const int		CORNER_COUNT		= 2;
	static const int		MAX_TRAIL_BUFFER	= 10;
	static constexpr float	DROP_SPEED			= 0.5f;
	static constexpr float	BLINK_SPEED			= 2.0f;
	static constexpr float	PARTICLE_LIFE		= 1.0f;

	// struct >> ObjectBlock >> [BlockState]
	struct BlockState
	{
		// �֐�
		void TransitionNextState();
		void TransitionMoveState(EnumBlockPlace);
		void TransitionStandState();
		void TransitionDropState();
		void TransitionPutState();
		void TransitionEraseState();
		void TransitionDisappearedState();

		// �ϐ�
		EnumBlockState	state	= EnumBlockState::MOVE;
		ObjectBlock*	obj		= nullptr;
	};

	// private:using
	
	// �X�e�[�g��Update�̊֐��|�C���^
	using StateUpdate = void (ObjectBlock::*)(float);

public:
	// public:�萔
	static constexpr float BLOCK_SIZE = 2.0f;

	// public:�R���X�g���N�^�E�f�X�g���N�^
	ObjectBlock(bool, ObjectBoard*);
	~ObjectBlock() override;

	// public:�R�s�[�R���X�g���N�^�E���[�u�R���X�g���N�^


	// public:�ʏ�֐�
	void	Update(float);
	void	ErasingUpdate(float);
	void	DebugGUI();
	void	Render();
	void	EmissiveRender();
	void	AdjustFromBlockPosition();
	void	AdjustFromBlockCell();
	void	ResetShiftY();
	void	AccumulateBlockParticle(ID3D11PixelShader*);
	void	FollowPartnerBlock(EnumBlockRotation);
	void	MoveBlock(GamePadButton);
	void	LiftBlock(EnumBlockRotation);
	bool	RotateBlock(ObjectBlock*, EnumBlockRotation, bool = false);		// �E�u���b�N�̂ݎg�p
	bool	JudgeFallBlock();

	// state_update�p
	void	UpdateMoveState(float);
	void	UpdateStandState(float);
	void	UpdateDropState(float);
	void	UpdatePutState(float);

	// public:�Q�b�^�[�֐�
	bool						GetErase()						{ return flag_system.GetFlag(EnumBlockFlags::ERASE); };
	bool						GetErasing()						{ return flag_system.GetFlag(EnumBlockFlags::ERASING); };
	const int					GetBlockColorNum()				{ return SCast(int, block_color); }
	const float					GetBlockTopPosition()			{ return this->translation.y - BLOCK_SIZE * 0.5f; }
	DirectX::XMFLOAT4X4			GetParticleTransform() const;
	EnumBlockColor				GetBlockColor()					{ return block_color; }
	GameModel*					GetModel() override				{ return model.get(); }
	BlockCell&					GetBlockCell()					{ return block_cell; }
	BlockCell&					GetGhostCell()					{ return ghost_cell; }	// ��]���S�[�X�g�ɔ��f�����邽�߂Ɏg�p
	BlockState&					GetBlockState()					{ return block_state; }

	// ���W�`�F�b�N�p

	template<typename Enum>
	FlagSystem<Enum>&			GetFlagSystem()			{ return flag_system; }

	// public:�Z�b�^�[�֐�
	void				EraseRegist() { flag_system.SetFlag(EnumBlockFlags::ERASE, true); }
	void				EraseConfirm() 
	{ 
		flag_system.SetFlag(EnumBlockFlags::ERASE, false);
		flag_system.SetFlag(EnumBlockFlags::ERASING, true);
	}
	
	void				SetStanding() 
	{ 
		if(block_state.state != EnumBlockState::STAND)
			block_state.TransitionStandState();
	}
	
	void				ResetStanding() 
	{ 
		if(block_state.state == EnumBlockState::STAND)
			block_state.TransitionMoveState(EnumBlockPlace::NONE); 
	}

	void				SetDropping();
	void				SetColorUndefine() { block_color = EnumBlockColor::UNDEFINE; }
	bool				DropBlock(float speed);

private:
	// private:�ϐ�
	// �u���b�N�̐F�֌W
	float								blink_time;
	EnumBlockColor						block_color		= EnumBlockColor::UNDEFINE;		// block�̐F
	DirectX::XMFLOAT3					block_color_factor;
	DirectX::XMFLOAT3					ghost_translation = {0.0f, 1.0f, 0.0f};
	FlagSystem<EnumBlockFlags>			flag_system;

	// ���f���֌W
	std::unique_ptr<GameModel>			model;			// block�̃��f��
	std::unique_ptr<PrimitiveRenderer>	primitive_renderer;
	std::unique_ptr<ParticleSystem>		block_particle;
	ComPtr<ID3D11PixelShader>			ghost_pixel_shader;

	// �e�Ֆ�
	ObjectBoard&						parent_board;

	// �Q�[���V�X�e���֌W
	BlockCell							block_cell		= BlockCell(0, 0);	// �Ֆʂɂ�����block�̏ꏊ
	BlockCell							ghost_cell		= BlockCell(0, 0);	// �S�[�X�g�̈ʒu
	BlockState							block_state;
	StateUpdate							state_update;
};

#endif // __OBJECT_BLOCK_H__