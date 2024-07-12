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
#include "UI/value_UI.h"
#include "UI/sprite_UI.h"

// ゲームソースファイル
#include "object_block.h"

// using
using PairBlock = std::pair<std::unique_ptr<ObjectBlock>, std::unique_ptr<ObjectBlock>>;

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
	BLOCK_REGISTERED
};

enum class EnumBoardState
{
	STANDBY,		// ゲームスタート前
	START,			// ゲームスタート
	DROP_START,		// ブロック落下スタート
	MOVING,			// ブロック操作中
	DROPPING,		// ブロック落下中
	LANDING,		// ブロックが着地
	GAME_OVER		// 決着がついたとき
};

enum class EnumCheckDirection
{
	VERTICAL,
	HORIZONTAL,
};

enum class EnumGameMode
{
	NORMAL,
	IMPACT,
	ENDLESS,
	MISSION,
	ULTIMATE,
	UNDEFINE
};

// 定数
constexpr float	SHAKE_FACTOR = 0.1f;

// class >> [ObjectBoard] 継承：glTFObject
class ObjectBoard : public GameObject
{
protected:
	// protected:定数
	static const UINT				ERASE_CONDITION			= 3;
	static const UINT				DEFAULT_START_LEVEL		= 1;
	static const UINT				LV_UP_BLOCK_COUNT		= 5;
	static const UINT				SCORE_BASE				= 10;

	static constexpr float				SPEED_FACTOR	= ObjectBlock::BLOCK_SIZE;
	static constexpr float				MODIFY_SPEED	= 0.6f;
	static constexpr float				ZOOM_MIN	= 10.0f;
	static constexpr float				ZOOM_MAX	= 200.0f;
	static constexpr DirectX::XMFLOAT3	ROOT_POSITION	= { -9.0f,17.5f,-0.5f };
	

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

		// 変数
		EnumBoardState	state = EnumBoardState::STANDBY;
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

		// 変数
		std::pair<bool, bool>	stand_switch;
		std::list<PairBlock>	next_block;
		BlockCell				left_cell		= { 0,0 };
		BlockCell				right_cell		= { 0,0 };
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

	// protected:using
	using StateUpdate = void (ObjectBoard::*)(float);

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
	void	CalcBlockSpeed(float);
	void	BoardShake(DirectX::XMFLOAT3);
	void	VerticalLineCheck(const UINT);
	void	HorizontalLineCheck(const UINT);
	void	SortInList(UPtrVector<ObjectBlock>&);
	void	GameStart(int);
	void	AccumulateBoardParticle();

	bool	MoveToDeletedBlockList();
	bool	CheckGameOver();
	bool	JudgeSameColor(EnumCheckDirection);

	UINT	CalcScore(UINT, UINT);

	void	SignalMove(GamePadButton);

	const DirectX::XMFLOAT3&	GetRootPosition() { return ROOT_POSITION; }

	// state_update用
	void	UpdateStandbyState(float);
	void	UpdateStartState(float);
	void	UpdateMovingState(float);
	void	UpdateDroppingState(float);
	void	UpdateLandingState(float);
	void	UpdateGameOverState(float);

	// public:ゲッター関数
	bool								IsExistingBlockFromCell(BlockCell);
	bool								IsInvalidBlockFromCell(BlockCell);
	bool								IsInvalidBlockFromIterator(UPtrVector<ObjectBlock>::iterator);
	float								GetCurrentSpeed() const						{ return current_speed; }
	UINT								GetEraseBlockCount() const					{ return deleted_block_count; }
	UINT								GetStandCollisionHeight(UINT index) const	{ return stand_collision_heignt[index]; }
	DirectX::XMFLOAT3					GetShakePosiiton() const					{ return shake_position; }
	UPtrVector<ObjectBlock>::iterator	GetBlockFromCell(const BlockCell);
	BoardState&							GetBoardState()								{ return board_state; }

	template<typename Enum>
	FlagSystem<Enum>&					GetFlagSystem()				{ return flag_system; }

	// public:セッター関数
	void									IncreaseEraseBlockCount()	{ deleted_block_count++; }

	void	 SetExistingBlockFromCell(const BlockCell cell, bool flg)
	{
		if (cell.row < 1 || cell.row > MAX_ROW)				return;
		if (cell.column < 1 || cell.column > MAX_COLUMN)		return;
		existing_matrix[cell.row - 1][cell.column - 1] = flg;
	}

protected:
	// protected:変数
	FlagSystem<EnumBoardFlags>			flag_system;							// フラグを管理する変数
	
	// モデル関係
	UPtrVector<ParticleSystem>			block_particle;								// 演出用パーティクル
	DirectX::XMFLOAT3					shake_position			= {0.0f,0.0f,0.0f};	// 元の位置からの移動距離
	DirectX::XMFLOAT3					shake_speed				= {0.0f,0.0f,0.0f}; // 元の位置から移動させるための速度
	std::unique_ptr<GameModel>			board_model;								// 盤面のモデル
	std::unique_ptr<ParticleSystem>		erase_block_particle;						// 盤面のモデル
	std::unique_ptr<ParticleSystem>		board_particle;								// 盤面のモデル

	// スピード関係
	float								current_speed			= 0.0f;		// ブロックの落下スピード
	float								speed_increase_factor	= 1.0f;		// ブロックの落下スピード
	UINT								speed_level				= 1;		// 現在のスピードレベル
	UINT								init_level				= 1;
	UINT								max_level				= 50;

	// 接地関係
	float								standing_time			= 0.0f;		// 接地からの時間
	float								standing_time_limit		= 1.0f;		// 接地からの猶予時間
	float								stand_decrease_factor	= 0.0f;		// 接地からの猶予時間
	float								waiting_time			= 0.0f;		// 全ブロック接地からの待機時間
	float								waiting_time_limit		= 0.5f;		// 全ブロック接地から次の処理をするまでの時間

	std::array<UINT, MAX_ROW>			stand_collision_heignt;

	// UI関係
	UPtrVector<ValueUI>					value_ui;
	UPtrVector<SpriteUI>				sprite_ui;

	// ブロック関係
	UINT								deleted_block_count		= 0;		// 消したブロックの数
	UINT								level_up_block			= 0;		// レベルアップに必要なブロック数
	NextBlock							next_block;
	DeleteBlockSorter					delete_block_sorter;
	UPtrVector<ObjectBlock>				block_list;

	std::vector<std::vector<bool>>		existing_matrix;					// ブロックが存在しているかしていないかだけを記録している変数

	// ゲームシステム関係
	float								count_down_time			= 4.0f;
	float								count_down_se_time		= 1.0f;
	float								count_down_se_span		= 1.0f;
	float								game_over_time			= 0.0f;
	float								fade_time				= 0.0f;
	float								fade_time_limit			= 1.0f;
	DirectX::XMFLOAT3					camera_rot				= {0.0f, 0.0f, 0.0f };
	UINT								chain					= 0;		// 連鎖数
	UINT								score					= 0;		// スコア
	UINT								player_id				= 0;		// プレイヤー番号
	EnumGameMode						game_mode				= EnumGameMode::UNDEFINE;
	BoardState							board_state;						// 盤面の状態
	StateUpdate							state_update;						// 状態ごとの処理を格納する関数ポインタ

	std::string chech_time;
};

#endif // __OBJECT_BOARD_H__