#ifndef __GAMESYSTEM_TIMER_H__
#define __GAMESYSTEM_TIMER_H__

// <>�C���N���[�h
#include <windows.h>

// ""�C���N���[�h
#include "singleton.h"
#include "misc.h"

// class >> [GameSystemTimer] �p���FSingleton<GameSystemTimer>
// �Q�[�����̎��Ԃ��Ǘ�����N���X�B
class GameSystemTimer : public Singleton<GameSystemTimer>
{
public:
	// public:�R���X�g���N�^�E�f�X�g���N�^
	GameSystemTimer() {};
	~GameSystemTimer() = default;

	//public:�R�s�[�R���X�g���N�^�E���[�u�R���X�g���N�^
	GameSystemTimer(const GameSystemTimer&)					= delete;
	GameSystemTimer& operator = (const GameSystemTimer&)	= delete;
	GameSystemTimer(GameSystemTimer&&) noexcept				= delete;
	GameSystemTimer& operator= (GameSystemTimer&&) noexcept	= delete;

	// public:�ʏ�֐�
	void TimerInitialize();
	void TimerReset();
	void TimerStart();
	void TimerStop();
	void TimerUpdate();
	void DebugGUI();

	// public:�Q�b�^�[�֐�
	float TimeStamp();
	float GetDeltaTime() { return SCast(float, delta_time); };

private:
	// private:�ϐ�
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