#ifndef __GAMESYSTEM_TIMER_H__
#define __GAMESYSTEM_TIMER_H__

// <>インクルード
#include <windows.h>

// ""インクルード
#include "singleton.h"
#include "misc.h"

// class >> [GameSystemTimer] 継承：Singleton<GameSystemTimer>
// ゲーム内の時間を管理するクラス。
class GameSystemTimer : public Singleton<GameSystemTimer>
{
public:
	// public:コンストラクタ・デストラクタ
	GameSystemTimer() {};
	~GameSystemTimer() = default;

	//public:コピーコンストラクタ・ムーブコンストラクタ
	GameSystemTimer(const GameSystemTimer&)					= delete;
	GameSystemTimer& operator = (const GameSystemTimer&)	= delete;
	GameSystemTimer(GameSystemTimer&&) noexcept				= delete;
	GameSystemTimer& operator= (GameSystemTimer&&) noexcept	= delete;

	// public:通常関数
	void TimerInitialize();
	void TimerReset();
	void TimerStart();
	void TimerStop();
	void TimerUpdate();
	void DebugGUI();

	// public:ゲッター関数
	float TimeStamp();
	float GetDeltaTime() { return SCast(float, delta_time); };

private:
	// private:変数
	bool		pause				= false;
	double		seconds_per_count	= 0.0;
	double		working_time		= 0.0;
	double		delta_time			= 0.0;
	LONGLONG	base_time			= 0LL;
	LONGLONG	pause_time			= 0LL;
	LONGLONG	stop_time			= 0LL;
	LONGLONG	last_time			= 0LL;
	LONGLONG	this_time			= 0LL;
};

#endif // __GAMESYSTEM_TIMER_H__