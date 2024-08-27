#ifndef __OBJECT_BOARD_H__
#define __OBJECT_BOARD_H__

// <>インクルード
#include <unordered_map>
#include <memory>
#include <queue>
#include <thread>

// ""インクルード
// LightBlueEngine
#include "game_object/game_object.h"
#include "game_object/game_state.h"
#include "UI/value_UI.h"
#include "UI/sprite_UI.h"

// ゲームソースファイル
#include "object_block.h"

// using
using PairBlock			= std::pair<std::unique_ptr<ObjectBlock>, std::unique_ptr<ObjectBlock>>;
using EraseParticleData = std::pair<std::unique_ptr<ParticleSystem>, float>;

template<typename T>
using UPtrVector = std::vector<std::unique_ptr<T>>;

// defineマクロ
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
	STANDBY,		// モード選択中
	START,			// ゲームスタート
	DROP_START,		// ブロック落下スタート
	MOVING,			// ブロック操作中
	DROPPING,		// ブロック落下中
	LANDING,		// ブロックが着地
	GAME_OVER,		// 決着がついたとき
	RESULT			// リザルト画面に遷移
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

// 定数
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

// class >> [ObjectBoard] 継承：glTFObject
class ObjectBoard : public GameObject
{
protected:
	// protected:定数
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
	

	// protected:構造体

	// struct >> ObjectBlock >> [BlockState]
	struct BoardState
	{
		// 関数
		void TransitionStandbyState();
		void TransitionStartState(int);
		void TransitionDropStartState();
		void TransitionMovingState();
		void TransitionDroppingState();
		void TransitionLandingState();
		void TransitionGameOverState(bool);
		void TransitionResultState();

		// 変数
		EnumBoardState	state = EnumBoardState::START;
		ObjectBoard*	obj;
	};

	// struct >> ObjectBlock >> [NextBlock]
	struct NextBlock
	{
		// 関数
		void Start();
		void Update(float);
		void Render();
		void EmissiveRender();
		void SetNewBlock();
		void SuperDrop();
		void MoveBlock(float);
		bool RotateBlock(bool);

		// 変数
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
		// 関数
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

		// 変数
		std::vector<UPtrVector<ObjectBlock>>		color_block;
		std::vector<bool>						erase_flag = { false,false,false,false,false,false };
	};

public:
	// public:定数
	static const DirectX::XMFLOAT3 INIT_POSITION;

	// public:コンストラクタ・デストラクタ
	ObjectBoard(UINT);
	~ObjectBoard() override;

	// public:通常関数
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

	// state_update用
	void	UpdateStartState(float);
	void	UpdateMovingState(float);
	void	UpdateDroppingState(float);
	void	UpdateLandingState(float);
	void	UpdateGameOverState(float);

	// public:ゲッター関数
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

	// public:セッター関数
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
	// protected:変数
	FlagSystem<EnumBoardFlags>			flag_system;							// フラグを管理する変数
	
	// モデル関係
	DirectX::XMFLOAT3					shake_position			= {0.0f,0.0f,0.0f};	// 元の位置からの移動距離
	DirectX::XMFLOAT3					shake_speed				= {0.0f,0.0f,0.0f}; // 元の位置から移動させるための速度
	DirectX::XMFLOAT3					board_color				= {1.0f,1.0f,1.0f}; // 元の位置から移動させるための速度
	std::unique_ptr<GameModel>			board_model;								// 盤面のモデル
	std::vector<EraseParticleData>		erase_block_particle;						// ブロック消去演出パーティクル

	// スピード関係
	float								level_speed				= 0.0f;		// 現在のレベルのブロック落下スピード
	float								current_speed			= 0.0f;		// 現在のブロック落下スピード
	float								speed_increase_factor	= 0.0f;		// ブロック落下スピードの増加量
	float								si_rank_bonus			= 0.0f;		// スピードランク上昇時の落下スピード増加量
	int									current_rank			= -1;		// 現在のスピードランク
	std::vector<int>					speed_rank;							// 落下スピードを大きく変化させるレベルを格納。BGMもこれで変更

	// 接地関係
	float								standing_time			= 0.0f;		// 接地からの時間
	float								standing_time_limit		= 1.0f;		// 接地からの猶予時間
	float								stand_decrease_factor	= 0.0f;		// 接地からの猶予時間の減少量
	float								waiting_time			= 0.0f;		// 全ブロック接地からの待機時間
	float								waiting_time_limit		= 0.5f;		// 全ブロック接地から次の処理をするまでの時間
	UINT								root_block_column = 0;

	std::array<UINT, MAX_ROW>			stand_collision_heignt;

	// UI関係
	float								ui_alpha = 1.0f;
	DirectX::XMFLOAT3					ui_color = { 1.0f, 1.0f, 1.0f };
	UPtrVector<ValueUI>					value_ui;
	UPtrVector<SpriteUI>				sprite_ui;

	// ブロック関係
	NextBlock							next_block;
	DeleteBlockSorter					delete_block_sorter;
	UPtrVector<ObjectBlock>				block_list;
	UPtrVector<ObjectBlock>				erased_block_list;

	std::vector<std::vector<bool>>				existing_matrix;					// ブロックが存在しているかしていないかだけを記録している変数
	std::vector<std::vector<EnumBlockColor>>	block_color_matrix;					// ブロックの色を記録する変数

	// ゲームシステム関係
	bool								pause					= false;
	float								count_down_time			= COUNT_DOWN;
	float								count_down_se_time		= 1.0f;
	float								count_down_se_span		= 1.0f;
	float								game_over_time			= 0.0f;
	float								fade_time				= 0.0f;
	float								fade_time_limit			= 1.0f;
	DirectX::XMFLOAT3					camera_rot				= {0.0f, 0.0f, 0.0f };
	UINT								chain					= 0;		// 連鎖数
	UINT								player_id				= 0;		// プレイヤー番号
	EnumGameMode						game_mode				= EnumGameMode::UNDEFINE;
	EnumGameMode						before_game_mode		= EnumGameMode::UNDEFINE;
	EnumGameStyle						game_style				= EnumGameStyle::SMOOTH;
	BoardState							board_state;						// 盤面の状態
	GameData							game_data;
	ClassFuncPtr<ObjectBoard, float>	state_update;						// 状態ごとの処理を格納する関数ポインタ
	EnumBoardState						state;
	StateMachine<ObjectBoard>			state_machine;
};

#endif // __OBJECT_BOARD_H__