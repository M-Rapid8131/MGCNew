#ifndef __OBJECT_BOARD_H__
#define __OBJECT_BOARD_H__

// <>�C���N���[�h
#include <unordered_map>
#include <memory>
#include <queue>
#include <thread>

// ""�C���N���[�h
// LightBlueEngine
#include "game_object/game_object.h"
#include "game_object/game_state.h"
#include "UI/value_UI.h"
#include "UI/sprite_UI.h"

// �Q�[���\�[�X�t�@�C��
#include "object_block.h"

// using
using PairBlock			= std::pair<std::unique_ptr<ObjectBlock>, std::unique_ptr<ObjectBlock>>;
using EraseParticleData = std::pair<std::unique_ptr<ParticleSystem>, float>;

template<typename T>
using UPtrVector = std::vector<std::unique_ptr<T>>;

// define�}�N��
#define LEFT_BLOCK	PairBlock::first 
#define RIGHT_BLOCK PairBlock::second 

// enum
enum class EnumBoardFlags
{
	RESULT,
	PLAYING,
	CAN_MOVE,
	WAITING,
	BLOCK_REGISTERED,
	CHANGE_PARTICLE_TYPE,
};

enum class EnumBoardState
{
	STANDBY,		// ���[�h�I��
	START,			// �Q�[���X�^�[�g
	DROP_START,		// �u���b�N�����X�^�[�g
	MOVING,			// �u���b�N���쒆
	DROPPING,		// �u���b�N������
	LANDING,		// �u���b�N�����n
	GAME_OVER,		// �����������Ƃ�
	RESULT			// ���U���g��ʂɑJ��
};

enum class EnumCheckDirection
{
	VERTICAL,
	HORIZONTAL,
};

enum class EnumGameStyle
{
	STANDARD,
	SMOOTH,
	SUDDEN_DEATH
};

enum class EnumGameMode
{
	NORMAL,
	EASY,
	IMPACT,
	ENDLESS,
	MISSION,
	TEMPEST,
	BONUS,
	
	UNDEFINE
};

// �萔
const float	SHAKE_FACTOR = 0.1f;

const DirectX::XMFLOAT3 BOARD_COLOR_SET[SCast(int, EnumGameMode::UNDEFINE)] =
{
	{1.0f, 1.0f, 1.0f},
	{0.0f, 1.0f, 0.0f},
	{1.0f, 0.0f, 0.0f},
	{0.0f, 1.0f, 1.0f},
	{1.0f, 0.0f, 1.0f},
	{0.4f, 0.0f, 1.0f},
	{1.0f, 1.0f, 0.0f},
};

// class >> [ObjectBoard] �p���FglTFObject
class ObjectBoard : public GameObject
{
protected:
	// protected:�萔
	static const UINT ERASE_CONDITION			= 3;
	static const UINT DEFAULT_START_LEVEL		= 1;
	static const UINT LV_UP_BLOCK_COUNT			= 5;
	static const UINT SCORE_BASE				= 10;
	static const UINT DIAGONAL_ADJUST			= MAX_ROW;

	static constexpr float				SPEED_FACTOR		= ObjectBlock::BLOCK_SIZE;
	static constexpr float				ACCEL_SPEED			= ObjectBlock::BLOCK_SIZE * 25.0f;
	static constexpr float				SPEED_LIMIT			= ObjectBlock::BLOCK_SIZE * 2000.0f;
	static constexpr float				BONUS_SPEED			= 500.0f;
	static constexpr float				EASY_BONUS_SPEED	= 200.0f;
	static constexpr float				ZOOM_MIN			= 10.0f;
	static constexpr float				DEFAULT_ZOOM		= 60.0f;
	static constexpr float				ZOOM_OUT_RATE		= 0.025f;
	static constexpr float				ZOOM_MAX			= 200.0f;
	static constexpr float				COUNT_DOWN			= 3.0f;
	static constexpr float				CAMERA_ANGLE_LIMIT	= 0.25f;
	static constexpr float				DEFAULT_WAIT_TIME	= 0.5f;
	static constexpr float				TEMPEST_WAIT_TIME	= 0.1f;
	static constexpr DirectX::XMFLOAT3	ROOT_POSITION		= { -9.0f, 17.5f, 0.0f };
	static constexpr DirectX::XMFLOAT4	DEFAULT_DIRECTION	= { 0.0f, 0.0f, -1.0f, 0.0f };
	

	// protected:�\����

	// struct >> ObjectBlock >> [BlockState]
	struct BoardState
	{
		// �֐�
		void TransitionStandbyState();
		void TransitionStartState(int);
		void TransitionDropStartState();
		void TransitionMovingState();
		void TransitionDroppingState();
		void TransitionLandingState();
		void TransitionGameOverState(bool);
		void TransitionResultState();

		// �ϐ�
		EnumBoardState	state = EnumBoardState::START;
		ObjectBoard*	obj;
	};

	// struct >> ObjectBlock >> [NextBlock]
	struct NextBlock
	{
		// �֐�
		void Start();
		void Update(float);
		void Render();
		void EmissiveRender();
		void SetNewBlock();
		void SuperDrop();
		void MoveBlock(float);
		bool RotateBlock(bool);

		// �ϐ�
		bool					standing;
		float					shift_y			= 0.0f;
		BlockCell				root_cell		= { 0,0 };
		std::list<PairBlock>	next_block;
		//BlockCell				right_cell		= { 0,0 };
		PairBlock				moving_block;
		ObjectBoard*			obj;
	};

	// struct >> ObjectBlock >> [DeleteBlockSorter]
	struct DeleteBlockSorter
	{
		// �֐�
		bool IsAllOff()
		{
			for (bool flg : erase_flag)
			{
				if (flg)	return false;
			}
			return true;
		}

		void AllOff()
		{
			for (int i = 0; i < erase_flag.size(); i++)
			{
				if (erase_flag.at(i))		erase_flag.at(i).flip();
			}
		}

		// �ϐ�
		std::vector<UPtrVector<ObjectBlock>>		color_block;
		std::vector<bool>						erase_flag = { false,false,false,false,false,false };
	};

public:
	// public:�萔
	static const DirectX::XMFLOAT3 INIT_POSITION;

	// public:�R���X�g���N�^�E�f�X�g���N�^
	ObjectBoard(UINT);
	~ObjectBoard() override;

	// public:�ʏ�֐�
	void	Update(float);
	void	UIUpdate(float);
	void	DebugGUI();
	void	Render();
	void	EmissiveRender();
	void	UIRender();
	void	RegisterBlock();
	void	ClearDeleteBlockList();
	void	CameraZoom(float);
	void	CameraMove(float);
	void	BoardShake(DirectX::XMFLOAT3);
	void	VerticalLineCheck(const UINT);
	void	HorizontalLineCheck(const UINT);
	void	DiagonalLineCheckLU(const UINT);
	void	DiagonalLineCheckLD(const UINT);
	void	SortInList(UPtrVector<ObjectBlock>&);
	void	GameStart(int);
	void	BonusStart(bool);
	void	AccumulateBoardParticle();
	void	LevelUp();
	void	SmoothLevelUp(UINT = 0, UINT = 0);
	void	UpdateStandCollisionHeight();

	bool	MoveToDeletedBlockList();
	bool	CheckGameOver();
	bool	JudgeSameColor(EnumCheckDirection);

	UINT	CalcScore(UINT, UINT);

	void	SignalMove(GamePadButton);

	const DirectX::XMFLOAT3&	GetRootPosition() { return ROOT_POSITION; }

	// state_update�p
	void	UpdateStartState(float);
	void	UpdateMovingState(float);
	void	UpdateDroppingState(float);
	void	UpdateLandingState(float);
	void	UpdateGameOverState(float);

	// public:�Q�b�^�[�֐�
	const bool							IsPausing()									{ return pause; }
	const bool							ChangeParticleType()						{ return flag_system.GetFlag(EnumBoardFlags::CHANGE_PARTICLE_TYPE); }
	const bool							IsExistFromColorMatrix(BlockCell);
	const bool							IsInvalidBlockFromCell(BlockCell);
	const bool							IsInvalidBlockFromIterator(UPtrVector<ObjectBlock>::iterator);
	const float							GetCurrentSpeed() const						{ return current_speed; }
	const UINT							GetCurrentRank() const						{ return current_rank; }
	const UINT							GetEraseBlockCount() const					{ return game_data.deleted_block_count; }
	const UINT							GetStandCollisionHeight(UINT index) const	{ return stand_collision_heignt[index]; }
	const UINT							GetPlayerID() const							{ return player_id; }
	const UINT							GetRootBlockColumn() const					{ return root_block_column; }
	const DirectX::XMFLOAT3				GetShakePosiiton() const					{ return shake_position; }
	UPtrVector<ObjectBlock>::iterator	GetBlockFromCell(const BlockCell);
	BoardState&							GetBoardState()								{ return board_state; }
	GameData&							GetGameData()								{ return game_data; }
	const EnumBlockColor&				GetBlockColorFromMatrix(const BlockCell);

	template<typename Enum>
	FlagSystem<Enum>&					GetFlagSystem()				{ return flag_system; }

	// public:�Z�b�^�[�֐�
	void								SetState(EnumBoardState new_state) { state = new_state; }
	void								ChangeBoardColorFromGameMode(const EnumGameMode game_mode) { board_color = BOARD_COLOR_SET[SCast(size_t, game_mode)]; }
	void								ChangedParticleType()		{ flag_system.SetFlag(EnumBoardFlags::CHANGE_PARTICLE_TYPE, false); }
	void								IncreaseEraseBlockCount()	{ game_data.deleted_block_count++; }

	void	 SetBlockColorFromCell(const BlockCell cell, const EnumBlockColor color)
	{
		if (cell.row < 1 || cell.row > MAX_ROW)				return;
		if (cell.column < 1 || cell.column > MAX_COLUMN)		return;
		block_color_matrix[cell.row - 1][cell.column - 1] = color;
		existing_matrix[cell.row - 1][cell.column - 1] = (color != EnumBlockColor::UNDEFINE);
	}

protected:
	// protected:�ϐ�
	FlagSystem<EnumBoardFlags>			flag_system;							// �t���O���Ǘ�����ϐ�
	
	// ���f���֌W
	DirectX::XMFLOAT3					shake_position			= {0.0f,0.0f,0.0f};	// ���̈ʒu����̈ړ�����
	DirectX::XMFLOAT3					shake_speed				= {0.0f,0.0f,0.0f}; // ���̈ʒu����ړ������邽�߂̑��x
	DirectX::XMFLOAT3					board_color				= {1.0f,1.0f,1.0f}; // ���̈ʒu����ړ������邽�߂̑��x
	std::unique_ptr<GameModel>			board_model;								// �Ֆʂ̃��f��
	std::vector<EraseParticleData>		erase_block_particle;						// �u���b�N�������o�p�[�e�B�N��

	// �X�s�[�h�֌W
	float								level_speed				= 0.0f;		// ���݂̃��x���̃u���b�N�����X�s�[�h
	float								current_speed			= 0.0f;		// ���݂̃u���b�N�����X�s�[�h
	float								speed_increase_factor	= 0.0f;		// �u���b�N�����X�s�[�h�̑�����
	float								si_rank_bonus			= 0.0f;		// �X�s�[�h�����N�㏸���̗����X�s�[�h������
	int									current_rank			= -1;		// ���݂̃X�s�[�h�����N
	std::vector<int>					speed_rank;							// �����X�s�[�h��傫���ω������郌�x�����i�[�BBGM������ŕύX

	// �ڒn�֌W
	float								standing_time			= 0.0f;		// �ڒn����̎���
	float								standing_time_limit		= 1.0f;		// �ڒn����̗P�\����
	float								stand_decrease_factor	= 0.0f;		// �ڒn����̗P�\���Ԃ̌�����
	float								waiting_time			= 0.0f;		// �S�u���b�N�ڒn����̑ҋ@����
	float								waiting_time_limit		= 0.5f;		// �S�u���b�N�ڒn���玟�̏���������܂ł̎���
	UINT								root_block_column = 0;

	std::array<UINT, MAX_ROW>			stand_collision_heignt;

	// UI�֌W
	float								ui_alpha = 1.0f;
	DirectX::XMFLOAT3					ui_color = { 1.0f, 1.0f, 1.0f };
	UPtrVector<ValueUI>					value_ui;
	UPtrVector<SpriteUI>				sprite_ui;

	// �u���b�N�֌W
	NextBlock							next_block;
	DeleteBlockSorter					delete_block_sorter;
	UPtrVector<ObjectBlock>				block_list;
	UPtrVector<ObjectBlock>				erased_block_list;

	std::vector<std::vector<bool>>				existing_matrix;					// �u���b�N�����݂��Ă��邩���Ă��Ȃ����������L�^���Ă���ϐ�
	std::vector<std::vector<EnumBlockColor>>	block_color_matrix;					// �u���b�N�̐F���L�^����ϐ�

	// �Q�[���V�X�e���֌W
	bool								pause					= false;
	float								count_down_time			= COUNT_DOWN;
	float								count_down_se_time		= 1.0f;
	float								count_down_se_span		= 1.0f;
	float								game_over_time			= 0.0f;
	float								fade_time				= 0.0f;
	float								fade_time_limit			= 1.0f;
	DirectX::XMFLOAT3					camera_rot				= {0.0f, 0.0f, 0.0f };
	UINT								chain					= 0;		// �A����
	UINT								player_id				= 0;		// �v���C���[�ԍ�
	EnumGameMode						game_mode				= EnumGameMode::UNDEFINE;
	EnumGameMode						before_game_mode		= EnumGameMode::UNDEFINE;
	EnumGameStyle						game_style				= EnumGameStyle::SMOOTH;
	BoardState							board_state;						// �Ֆʂ̏��
	GameData							game_data;
	ClassFuncPtr<ObjectBoard, float>	state_update;						// ��Ԃ��Ƃ̏������i�[����֐��|�C���^
	EnumBoardState						state;
	StateMachine<ObjectBoard>			state_machine;
};

#endif // __OBJECT_BOARD_H__