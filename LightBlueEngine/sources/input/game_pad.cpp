// <>インクルード
#include <Windows.h>
#include <imgui.h>
#include <math.h>
#include <Xinput.h>

// ""インクルード
// LightBlueEngine
#include "game_pad.h"

// ゲームソースファイル
#include "misc.h"

//-------------------------------------------
// GamePad メンバ関数
//-------------------------------------------

// 更新処理
void GamePad::Update()
{
	axis_lx		= axis_ly	= 0.0f;
	axis_rx		= axis_ry	= 0.0f;
	trigger_l	= trigger_r = 0.0f;

	GamePadButton new_button_state = 0;
	EnumInputDevice tentative_device = EnumInputDevice::UNDEFINE;

	XINPUT_STATE xinput_state;
	// XBOXコントローラー設定
	if (XInputGetState(slot, &xinput_state) == ERROR_SUCCESS)
	{
		tentative_device = EnumInputDevice::XBOX;

		XINPUT_GAMEPAD& pad = xinput_state.Gamepad;

		// 入力判定
		if (pad.wButtons & XINPUT_GAMEPAD_DPAD_UP)					new_button_state |= BTN_UP;
		if (pad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)				new_button_state |= BTN_RIGHT;
		if (pad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN)				new_button_state |= BTN_DOWN;
		if (pad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT)				new_button_state |= BTN_LEFT;
		if (pad.wButtons & XINPUT_GAMEPAD_A)						new_button_state |= BTN_A;
		if (pad.wButtons & XINPUT_GAMEPAD_B)						new_button_state |= BTN_B;
		if (pad.wButtons & XINPUT_GAMEPAD_X)						new_button_state |= BTN_X;
		if (pad.wButtons & XINPUT_GAMEPAD_Y)						new_button_state |= BTN_Y;
		if (pad.wButtons & XINPUT_GAMEPAD_START)					new_button_state |= BTN_START;
		if (pad.wButtons & XINPUT_GAMEPAD_BACK)						new_button_state |= BTN_BACK;
		if (pad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB)				new_button_state |= BTN_LEFT_THUMB;
		if (pad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB)				new_button_state |= BTN_RIGHT_THUMB;
		if (pad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER)			new_button_state |= BTN_LEFT_SHOULDER;
		if (pad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER)			new_button_state |= BTN_RIGHT_SHOULDER;
		if (pad.bLeftTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD)	new_button_state |= BTN_LEFT_TRIGGER;
		if (pad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD)	new_button_state |= BTN_RIGHT_TRIGGER;

		// スティックのデッドゾーン判定
		if ((pad.sThumbLX < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE && pad.sThumbLX > -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) &&
			(pad.sThumbLY < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE && pad.sThumbLY > -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE))
		{
			pad.sThumbLX = 0;
			pad.sThumbLY = 0;
		}

		if ((pad.sThumbRX < XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE && pad.sThumbRX > -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) &&
			(pad.sThumbRY < XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE && pad.sThumbRY > -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE))
		{
			pad.sThumbRX = 0;
			pad.sThumbRY = 0;
		}

		// トリガーとスティックの入力を保存
		trigger_l	= SCast(float, pad.bLeftTrigger) / 255.0f;
		trigger_r	= SCast(float, pad.bRightTrigger) / 255.0f;
		axis_lx		= SCast(float, pad.sThumbLX) / SCast(float, 0x8000);
		axis_ly		= SCast(float, pad.sThumbLY) / SCast(float, 0x8000);
		axis_rx		= SCast(float, pad.sThumbRX) / SCast(float, 0x8000);
		axis_ry		= SCast(float, pad.sThumbRY) / SCast(float, 0x8000);
	}

	// PS4コントローラー設定
	else
	{
		JOYINFOEX joy_info;
		joy_info.dwSize		= sizeof(JOYINFOEX);
		joy_info.dwFlags		= JOY_RETURNALL;

		if (joyGetPosEx(slot, &joy_info) == JOYERR_NOERROR)
		{
			tentative_device = EnumInputDevice::PS;

			static const WORD PS4_PID = 1476;

			JOYCAPS joy_caps;
			if (joyGetDevCaps(slot, &joy_caps, sizeof(JOYCAPS)) == JOYERR_NOERROR)
			{
				if (joy_info.dwPOV != 0xFFFF)
				{
					static const int pov_bit[8] = {
						BTN_UP,
						BTN_RIGHT | BTN_UP,
						BTN_RIGHT,
						BTN_RIGHT | BTN_DOWN,
						BTN_DOWN,
						BTN_LEFT | BTN_DOWN,
						BTN_LEFT,
						BTN_LEFT | BTN_UP
					};
					int angle = joy_info.dwPOV / 4500;
					new_button_state |= pov_bit[angle];
				}
				if (joy_caps.wPid == PS4_PID)
				{
					// 入力判定
					if (joy_info.dwButtons & JOY_BUTTON1)		new_button_state |= BTN_Y;
					if (joy_info.dwButtons & JOY_BUTTON2)		new_button_state |= BTN_B;
					if (joy_info.dwButtons & JOY_BUTTON3)		new_button_state |= BTN_A;
					if (joy_info.dwButtons & JOY_BUTTON4)		new_button_state |= BTN_X;
					if (joy_info.dwButtons & JOY_BUTTON5)		new_button_state |= BTN_LEFT_SHOULDER;
					if (joy_info.dwButtons & JOY_BUTTON6)		new_button_state |= BTN_RIGHT_SHOULDER;
					if (joy_info.dwButtons & JOY_BUTTON7)		new_button_state |= BTN_LEFT_TRIGGER;
					if (joy_info.dwButtons & JOY_BUTTON8)		new_button_state |= BTN_RIGHT_TRIGGER;
					if (joy_info.dwButtons & JOY_BUTTON9)		new_button_state |= BTN_BACK;
					if (joy_info.dwButtons & JOY_BUTTON10)		new_button_state |= BTN_START;
					if (joy_info.dwButtons & JOY_BUTTON11)		new_button_state |= BTN_LEFT_THUMB;
					if (joy_info.dwButtons & JOY_BUTTON12)		new_button_state |= BTN_RIGHT_THUMB;

					// トリガー・スティック入力
					axis_lx		= SCast(int, joy_info.dwXpos - 0x7FFF) / SCast(float, 0x8000);
					axis_ly		= SCast(int, joy_info.dwYpos - 0x7FFF) / SCast(float, 0x8000);
					axis_rx		= SCast(int, joy_info.dwZpos - 0x7FFF) / SCast(float, 0x8000);
					axis_ry		= SCast(int, joy_info.dwRpos - 0x7FFF) / SCast(float, 0x8000);
					trigger_l	= SCast(int, joy_info.dwVpos) / SCast(float, 0xFFFF);
					trigger_r	= SCast(int, joy_info.dwUpos) / SCast(float, 0xFFFF);

					if (axis_lx > -0.25f && axis_lx < 0.25f) axis_lx = 0.0f;
					if (axis_rx > -0.25f && axis_rx < 0.25f) axis_rx = 0.0f;
				}
			}
		}
	}

	// キーボード設定
	float lx = 0.0f;
	float ly = 0.0f;
	float rx = 0.0f;
	float ry = 0.0f;

	// 入力判定
	if (GetAsyncKeyState('W') & 0x8000)			ly = 1.0f;
	if (GetAsyncKeyState('A') & 0x8000)			lx = -1.0f;
	if (GetAsyncKeyState('S') & 0x8000)			ly = -1.0f;
	if (GetAsyncKeyState('D') & 0x8000)			lx = 1.0f;

	if (GetAsyncKeyState('I') & 0x8000)			ry = 1.0f;
	if (GetAsyncKeyState('J') & 0x8000)			rx = -1.0f;
	if (GetAsyncKeyState('K') & 0x8000)			ry = -1.0f;
	if (GetAsyncKeyState('L') & 0x8000)			rx = 1.0f;
	if (GetAsyncKeyState('Z') & 0x8000)			new_button_state |= BTN_A;
	if (GetAsyncKeyState('X') & 0x8000)			new_button_state |= BTN_B;
	if (GetAsyncKeyState('C') & 0x8000)			new_button_state |= BTN_X;
	if (GetAsyncKeyState('V') & 0x8000)			new_button_state |= BTN_Y;
	if (GetAsyncKeyState('Q') & 0x8000)			new_button_state |= BTN_LEFT_SHOULDER;
	if (GetAsyncKeyState('E') & 0x8000)			new_button_state |= BTN_RIGHT_SHOULDER;
	if (GetAsyncKeyState(VK_UP) & 0x8000)		new_button_state |= BTN_UP;
	if (GetAsyncKeyState(VK_RIGHT) & 0x8000)	new_button_state |= BTN_RIGHT;
	if (GetAsyncKeyState(VK_DOWN) & 0x8000)		new_button_state |= BTN_DOWN;
	if (GetAsyncKeyState(VK_LEFT) & 0x8000)		new_button_state |= BTN_LEFT;

    if (GetAsyncKeyState(VK_RETURN) & 0x8000)	new_button_state |= BTN_START;
	if (GetAsyncKeyState(VK_BACK) & 0x8000)		new_button_state |= BTN_BACK;

	if (tentative_device == EnumInputDevice::UNDEFINE)
		tentative_device = EnumInputDevice::KEYBOARD;

	// 入力デバイスの確定
	input_device = tentative_device;

	// スティック入力値のノーマライズ
	if (lx >= 1.0f || lx <= -1.0f || ly >= 1.0f || ly <= -1.0f)
	{
		float power = sqrtf(lx * lx + ly * ly);
		axis_lx = lx / power;
		axis_ly = ly / power;
	}

	if (rx >= 1.0f || rx <= -1.0f || ry >= 1.0f || ry <= -1.0f)
	{
		float power = sqrtf(rx * rx + ry * ry);
		axis_rx = rx / power;
		axis_ry = ry / power;
	}

	// バイブレーション設定
	XINPUT_VIBRATION vibration = {};
	vibration.wLeftMotorSpeed	= SCast(WORD, left_vibration * 65535);
	vibration.wRightMotorSpeed	= SCast(WORD, right_vibration * 65535);

	XInputSetState(SCast(DWORD, slot), &vibration);

	// 入力情報の更新
	button_state[SCast(size_t, EnumGamePadValue::OLD)]
		= button_state[SCast(size_t,EnumGamePadValue::CURRENT)];

	button_state[SCast(size_t, EnumGamePadValue::CURRENT)] = new_button_state;

	button_down = ~button_state[SCast(size_t, EnumGamePadValue::OLD)] & new_button_state;
	button_up = ~new_button_state & button_state[SCast(size_t, EnumGamePadValue::OLD)];
}

// ImGui表示
void GamePad::DebugGUI()
{
	int		debug_slot			= SCast(int, slot);
	int		button_state_int[2] = {
		SCast(int, button_state[SCast(size_t, EnumGamePadValue::CURRENT)]),
		SCast(int, button_state[(size_t)EnumGamePadValue::OLD])};

	int		button_down_int		= SCast(int, button_down);
	int		button_up_int		= SCast(int, button_up);
	float	axis_l[2]			= { axis_lx,axis_ly };
	float	axis_r[2]			= { axis_rx,axis_ry };
	float	trigger[2]			= { trigger_l,trigger_r };

	{
		ImGui::InputInt2("button_state", button_state_int);
		ImGui::InputInt("button_down", &button_down_int);
		ImGui::InputInt("button_up", &button_up_int);
		ImGui::InputFloat2("axis_l", axis_l);
		ImGui::InputFloat2("axis_r", axis_r);
		ImGui::InputFloat2("trigger", trigger);
		ImGui::SliderFloat("left_viblation", &left_vibration, 0.0f, 1.0f);
		ImGui::SliderFloat("right_viblation", &right_vibration, 0.0f, 1.0f);
		ImGui::InputInt("slot", &debug_slot);
	}
}