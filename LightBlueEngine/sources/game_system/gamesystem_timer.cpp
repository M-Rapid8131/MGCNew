// <>�C���N���[�h
#include <imgui.h>

// ""�C���N���[�h
// LightBlueEngine
#include "engine_utility.h"
#include "gamesystem_timer.h"

// �Q�[���\�[�X�t�@�C��
#include "misc.h"

//-------------------------------------------
// GameSystemTimer �����o�֐�
//-------------------------------------------

// �^�C�}�[�̏���������
void GameSystemTimer::TimerInitialize()
{
	// 1�b������̃J�E���g��
	LONGLONG counts_per_sec;
	QueryPerformanceFrequency(RCast(LARGE_INTEGER*, &counts_per_sec));
	seconds_per_count = 1.0 / SCast(double, counts_per_sec);

	// ���Ԑݒ�
	QueryPerformanceCounter(RCast(LARGE_INTEGER*, &this_time));
	base_time = this_time;
	last_time = this_time;
}

// �^�C�}�[�̃��Z�b�g
void GameSystemTimer::TimerReset()
{
	// ���Ԃ̍Đݒ�
	QueryPerformanceCounter(RCast(LARGE_INTEGER*, &this_time));
	base_time = this_time;
	last_time = this_time;

	stop_time	= 0;
	pause		= false;
}

// �^�C�}�[�J�n
void GameSystemTimer::TimerStart()
{
	LONGLONG start_time;
	QueryPerformanceCounter(RCast(LARGE_INTEGER*, &start_time));

	// �|�[�Y��Ԃ������ꍇ�̓|�[�Y���Ԃ��v�Z���A�|�[�Y����������
	if (pause)
	{
		pause_time += (start_time - stop_time);
		last_time	= start_time;
		stop_time	= 0;
		pause		= false;
	}
}

// �^�C�}�[�ꎞ��~
void GameSystemTimer::TimerStop()
{
	// �|�[�Y��ԂłȂ���΃|�[�Y�������Ԃ��L�^���ă|�[�Y��Ԃɂ���
	if (!pause)
	{
		QueryPerformanceCounter(RCast(LARGE_INTEGER*, &stop_time));
		pause = true;
	}
}

// �^�C�}�[�X�V
void GameSystemTimer::TimerUpdate()
{
	// �|�[�Y��Ԃł���ꍇ�́Adelta_time��0.0�ɂ���return����
	if (pause)
	{
		delta_time = 0.0;
		return;
	}

	QueryPerformanceCounter(RCast(LARGE_INTEGER*, &this_time));
	// �L�^����Ă��鎞�Ԃƌ��݂̎��Ԃ̍��ق��v�Z
	delta_time = (this_time - last_time) * seconds_per_count;
	working_time += delta_time;

	last_time = this_time;

	// delta_time�����̐��ɂȂ�Ȃ��悤�ɒ���
	delta_time = (std::max)(delta_time, 0.0);
}

// ImGui�\��
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
	// �|�[�Y��Ԃ̂Ƃ��́A��~�������Ԃ���|�[�Y���ԂƊ���Ԃ��������l��p����
	if (pause)
	{
		return SCast(float, (((stop_time - pause_time) - base_time) * seconds_per_count));
	}

	// �|�[�Y��ԂłȂ��Ƃ��́A���ݎ��Ԃ���|�[�Y���ԂƊ���Ԃ��������l��p����
	else
	{
		return SCast(float, (((this_time - pause_time) - base_time) * seconds_per_count));
	}
}
