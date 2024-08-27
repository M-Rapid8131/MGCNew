#ifndef __GAME_STATE_H__
#define __GAME_STATE_H__

// ""インクルード
#include <vector>

template<class Obj>
class GameState
{
public:
	GameState(Obj* owner_obj) : obj(owner_obj) {}
	virtual ~GameState() {}

	virtual void Transition() = 0;
	virtual void Update(float elapsed_time) = 0;
	virtual void Exit() = 0;
protected:
	Obj* obj;
};

template<class Obj>
class StateMachine
{
public:
	StateMachine() {};
	~StateMachine() {};

	void Update(float elapsed_time)
	{
		current_state->Update(elapsed_time);
	}

	void SetState(int set_state)
	{
		current_state = state_pool[set_state];
	}

	void ChangeState(int new_state)
	{
		current_state->Exit();
		SetState(new_state);
		current_state->Transition();
	}

	void RegisterState(GameState<Obj>* state)
	{
		state_pool.emplace_back(state);
	}

private:
	GameState<Obj>* current_state = nullptr;
	std::vector<GameState<Obj>*> state_pool;
};


#endif // !__GAME_STATE_H__

