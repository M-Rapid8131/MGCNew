// <>インクルード
#include <imgui.h>

// ""インクルード
// LightBlueEngine
#include "engine_utility.h"
#include "gamesystem_timer.h"

// ゲームソースファイル
#include "misc.h"

//-------------------------------------------
// GameSystemTimer メンバ関数
//-------------------------------------------

// タイマーの初期化処理
void GameSystemTimer::TimerInitialize()
{
	// 1秒あたりのカウント数
	LONGLONG counts_per_sec;
	QueryPerformanceFrequency(RCast(LARGE_INTEGER*, &counts_per_sec));
	seconds_per_count = 1.0 / SCast(double, counts_per_sec);

	// 時間設定
	QueryPerformanceCounter(RCast(LARGE_INTEGER*, &this_time));
	base_time = this_time;
	last_time = this_time;
}

// タイマーのリセット
void GameSystemTimer::TimerReset()
{
	// 時間の再設定
	QueryPerformanceCounter(RCast(LARGE_INTEGER*, &this_time));
	base_time = this_time;
	last_time = this_time;

	stop_time	= 0;
	pause		= false;
}

// タイマー開始
void GameSystemTimer::TimerStart()
{
	LONGLONG start_time;
	QueryPerformanceCounter(RCast(LARGE_INTEGER*, &start_time));

	// ポーズ状態だった場合はポーズ時間を計算し、ポーズを解除する
	if (pause)
	{
		pause_time += (start_time - stop_time);
		last_time	= start_time;
		stop_time	= 0;
		pause		= false;
	}
}

// タイマー一時停止
void GameSystemTimer::TimerStop()
{
	// ポーズ状態でなければポーズした時間を記録してポーズ状態にする
	if (!pause)
	{
		QueryPerformanceCounter(RCast(LARGE_INTEGER*, &stop_time));
		pause = true;
	}
}

// タイマー更新
void GameSystemTimer::TimerUpdate()
{
	// ポーズ状態である場合は、delta_timeを0.0にしてreturnする
	if (pause)
	{
		delta_time = 0.0;
		return;
	}

	QueryPerformanceCounter(RCast(LARGE_INTEGER*, &this_time));
	// 記録されている時間と現在の時間の差異を計算
	delta_time = (this_time - last_time) * seconds_per_count;
	working_time += delta_time;

	last_time = this_time;

	// delta_timeが負の数にならないように調整
	delta_time = (std::max)(delta_time, 0.0);
}

// ImGui表示
void GameSystemTimer::DebugGUI()
{
	if (ImGui::Begin("GameSystemTimer"))
	{
		if (ImGui::Checkbox("Pause", &pause))
		{

		}
		
		if (ImGui::InputDouble("Second Per Count", &seconds_per_count))
		{

		}

		if (ImGui::InputDouble("Working Time", &working_time))
		{

		}

		if (ImGui::InputDouble("Delta Time", &delta_time))
		{

		}

		if (ImGui::InputScalar("Base Time", ImGuiDataType_::ImGuiDataType_S64, &base_time))
		{

		}

		if (ImGui::InputScalar("Pause Time", ImGuiDataType_::ImGuiDataType_S64, &pause_time))
		{

		}

		if (ImGui::InputScalar("Stop Time", ImGuiDataType_::ImGuiDataType_S64, &stop_time))
		{

		}

		if (ImGui::InputScalar("Last Time", ImGuiDataType_::ImGuiDataType_S64, &last_time))
		{

		}

		if (ImGui::InputScalar("This Time", ImGuiDataType_::ImGuiDataType_S64, &this_time))
		{

		}
	}
	ImGui::End();
}

float GameSystemTimer::TimeStamp()
{
	// ポーズ状態のときは、停止した時間からポーズ時間と基準時間を引いた値を用いる
	if (pause)
	{
		return SCast(float, (((stop_time - pause_time) - base_time) * seconds_per_count));
	}

	// ポーズ状態でないときは、現在時間からポーズ時間と基準時間を引いた値を用いる
	else
	{
		return SCast(float, (((this_time - pause_time) - base_time) * seconds_per_count));
	}
}
