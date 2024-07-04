// <>�C���N���[�h
#include <imgui.h>

// ""�C���N���[�h
// LightBlueEngine
#include "mouse.h"

// �萔
static const int KEY_MAP[] = {
	VK_LBUTTON,
	VK_MBUTTON,
	VK_RBUTTON
};

//-------------------------------------------
// Mouse �����o�֐�
//-------------------------------------------

// ����������
void Mouse::Initialize(HWND p_hwnd)
{
	hwnd = p_hwnd;

	GetClientRect(p_hwnd, &point_info.rc);
	screen_width		= point_info.rc.right - point_info.rc.left;
	screen_height	= point_info.rc.bottom - point_info.rc.top;
}

// �X�V����
void Mouse::Update()
{
	MouseButton new_button_state = 0;

	for (int i = 0; i < ARRAYSIZE(KEY_MAP); ++i)
	{
		if (GetAsyncKeyState(KEY_MAP[i]) & 0x8000)
		{
			new_button_state |= (1 << i);
		}
	}

	// ���͏��̍X�V
	wheel[SCast(size_t,EnumMouseValue::OLD)] 
		= wheel[SCast(size_t, EnumMouseValue::CURRENT)];
	wheel[SCast(size_t, EnumMouseValue::CURRENT)] = 0;

	button_state[SCast(size_t, EnumMouseValue::OLD)]
		= button_state[SCast(size_t, EnumMouseValue::CURRENT)];
	button_state[SCast(size_t, EnumMouseValue::CURRENT)] = new_button_state;

	button_down = ~button_state[SCast(size_t, EnumMouseValue::OLD)] & new_button_state;
	button_up	= ~new_button_state & button_state[SCast(size_t, EnumMouseValue::OLD)];

	// �J�[�\�����擾
	POINT cursor;
	GetCursorPos(&cursor);
	ScreenToClient(hwnd, &cursor);
	
	// �X�N���[�����
	RECT rc;
	GetClientRect(hwnd, &rc);
	UINT screen_w	= rc.right - rc.left;
	UINT screen_h	= rc.bottom - rc.top;
	UINT viewport_w = screen_width;
	UINT viewport_h = screen_height;

	// �}�E�X�ʒu���̍X�V
	position_x[SCast(size_t, EnumMouseValue::OLD)] 
		= position_x[SCast(size_t, EnumMouseValue::CURRENT)];
	position_y[SCast(size_t, EnumMouseValue::OLD)]
		= position_y[SCast(size_t, EnumMouseValue::CURRENT)];

	position_x[SCast(size_t, EnumMouseValue::CURRENT)]
		= SCast(LONG,cursor.x / SCast(float,viewport_w) * SCast(float, screen_w));
	position_y[SCast(size_t, EnumMouseValue::CURRENT)]
		= SCast(LONG, cursor.y / SCast(float, viewport_h) * SCast(float, screen_h));

	position_x[SCast(size_t, EnumMouseValue::DELTA)]
		= position_x[SCast(size_t, EnumMouseValue::CURRENT)] - position_x[SCast(size_t, EnumMouseValue::OLD)];
	position_y[SCast(size_t, EnumMouseValue::DELTA)]
		= position_y[SCast(size_t, EnumMouseValue::CURRENT)] - position_y[SCast(size_t, EnumMouseValue::OLD)];
}

// ImGui�\��
void Mouse::DebugGUI()
{
	int button_state_int[2] = { SCast(int,button_state[0]), SCast(int,button_state[1]) };
	int button_down_int		= SCast(int, button_down);
	int button_up_int		= SCast(int, button_up);
	if (ImGui::Begin("mouse"))
	{
		ImGui::InputInt2("button_state" ,button_state_int);
		ImGui::InputInt("button_down", &button_down_int);
		ImGui::InputInt("button_up", &button_up_int);
		ImGui::InputInt3("position_x", position_x);
		ImGui::InputInt3("position_y", position_y);
		ImGui::InputInt2("wheel",wheel);
	}
	ImGui::End();
}