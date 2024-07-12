// <>�C���N���[�h
#include <time.h>

// ""�C���N���[�h
// LightBlueEngine
#include "graphics/graphics.h"
#include "game_system/gamesystem_director.h"
#include "game_system/gamesystem_timer.h"

// �Q�[���\�[�X�t�@�C��
#include "framework.h"

//-------------------------------------------
// Framework ���̑�����
//-------------------------------------------

int Framework::Run()
{
	// �V���O���g���N���X
	GameSystemTimer*		timer		= GameSystemTimer::GetInstance();

	MSG	msg	= {};

	if (!Initialize())
	{
		return 0;
	}

	// �Q�[�����[�v
	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			timer->TimerUpdate();
			CalculateFrameStats();

			Update(timer->GetDeltaTime());
			Render();
		}
	}

	GamesystemDirector::GetInstance()->RestoreWindowMode();

	return Uninitialize() ? SCast(int,msg.wParam) : 0;
}

LRESULT CALLBACK Framework::HandleMessage(HWND param_hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	// �V���O���g���N���X
	GameSystemTimer* timer = GameSystemTimer::GetInstance();

#ifdef USE_IMGUI
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) { return true; }
#endif
	switch (msg)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT ps{};
		BeginPaint(param_hwnd, &ps);

		EndPaint(param_hwnd, &ps);
	}
	break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_CREATE:
		break;
	case WM_KEYDOWN:
		if (wparam == VK_ESCAPE)
		{
			PostMessage(param_hwnd, WM_CLOSE, 0, 0);
		}
		break;
	case WM_MOUSEWHEEL:
		break;
	case WM_ENTERSIZEMOVE:
		timer->TimerStop();
		break;
	case WM_EXITSIZEMOVE:
		timer->TimerStart();
		break;
	default:
		return DefWindowProc(param_hwnd, msg, wparam, lparam);
	}
	return 0;
}

void Framework::CalculateFrameStats()
{
	// �V���O���g���N���X
	GameSystemTimer* timer = GameSystemTimer::GetInstance();

	if (++frames, (timer->TimeStamp() - operationg_time) >= 1.0f)
	{
		float fps = SCast(float,frames);
		std::wostringstream outs;
		outs.precision(256);
#ifdef _DEBUG
		outs << Graphics::W_APPLICATION_NAME << L" : FPS : " << fps << L" / " << L"Frame Time : " << 1000.0f / fps << L" (ms)";
#else
		outs << Graphics::W_APPLICATION_NAME;
#endif // DEBUG

		SetWindowTextW(hwnd, outs.str().c_str());

		frames = 0;
		operationg_time += 1.0f;
	}
}

//-------------------------------------------
// Framework �����o�֐�
//-------------------------------------------

// ����������
bool Framework::Initialize()
{
	Py_Initialize();
	//np_cpp::initialize();

	srand(SCast(unsigned int, time(NULL)));

	// �V���O���g���N���X�̏���������
	GameSystemTimer::GetInstance()->TimerInitialize();

	EnumCameraMode init_mode = EnumCameraMode::THIRD_PERSON_VIEW;
	GamesystemDirector::GetInstance()->Initialize(hwnd, init_mode);

	return true;
}

// �X�V����
void Framework::Update(float elapsed_time)
{
	// �X�V����
	GamesystemDirector::GetInstance()->Update(elapsed_time);

#ifdef USE_IMGUI
	GamesystemDirector::GetInstance()->DebugGUI();
#endif // USE_IMGUI
}

// �`�揈��
void Framework::Render()
{
	GamesystemDirector::GetInstance()->Render();

	// �X���b�v�`�F�[��
	const UINT SYNC_INTERVAL = 1;
	Graphics::GetInstance()->SwapChainPresent(SYNC_INTERVAL, 0);
}

// �I������
bool Framework::Uninitialize()
{
	return true;
}

Framework::~Framework()
{
}