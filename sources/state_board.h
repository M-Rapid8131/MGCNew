#ifndef __STATE_BOARD_H__
#define __STATE_BOARD_H__

#include "game_object/game_state.h"
#include "object_board.h"

class StandbyState : public GameState<ObjectBoard>
{
public:
	StandbyState(ObjectBoard* owner_obj) : GameState<ObjectBoard>(owner_obj) {}
	virtual ~StandbyState() {}

	void Transition() {};
	void Update(float elapsed_time) {};
	void Exit() {};
};

#endif // !__STATE_BOARD_H__

