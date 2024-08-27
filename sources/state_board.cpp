// ""インクルード
#include "game_system/gamesystem_director.h"
#include "state_board.h"

void StandbyState::Transition()
{
	obj->SetState(EnumBoardState::STANDBY);

	AudioManager* audio_manager = GamesystemDirector::GetInstance()->GetAudioManager();
	audio_manager->StopBGM();
	audio_manager->PlayBGM(EnumBGMBank::MODE_SELECT, true);

	obj->ui_alpha = 0.0f;
}