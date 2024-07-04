// <>�C���N���[�h
#include <imgui.h>

// ""�C���N���[�h
// LightBlueEngine
#include "graphics/graphics.h"
#include "scene/scene_loading.h"
#include "gamesystem_director.h"
#include "game_system/gamesystem_input.h"
#include "json_editor.h"

// �Q�[���\�[�X�t�@�C��
#include "scene_main_manu.h"
#include "scene_game.h"

//-------------------------------------------
// GamesystemDirector �����o�֐�
//-------------------------------------------

// ����������
void GamesystemDirector::Initialize(HWND hwnd, EnumCameraMode& start_mode)
{
	// �e�K�{�N���X�I�u�W�F�N�g�̏�����
	Graphics* graphics = Graphics::GetInstance();
	graphics->Initialize(hwnd, !fullscreen_mode);

	GamesystemInput::GetInstance()->Initialize(hwnd);

	audio					= std::make_unique<Audio>();
	audio_manager			= std::make_unique<AudioManager>();
	camera					= std::make_unique<Camera>(start_mode);
	framebuffer_manager		= std::make_unique<FramebufferManager>();
	imgui_renderer			= std::make_unique<ImGuiRenderer>(hwnd);
	light					= std::make_unique<Light>();
	MakeRandomNoise();
	MakeNumberRenderer();
	scene_manager			= std::make_unique<SceneManager>();

	auto start_scene	= std::make_unique<SceneGame>();
	auto scene_loading	= std::make_unique<SceneLoading>(start_scene.release());

	scene_manager->ChangeScene(scene_loading.release());

	const uint32_t WIDTH	= graphics->GetScreenWidth();
	const uint32_t HEIGHT	= graphics->GetScreenHeight();

	framebuffer_manager->MakeFramebuffer("main", WIDTH, HEIGHT);
	framebuffer_manager->MakeFramebuffer("game", WIDTH, HEIGHT);
	framebuffer_manager->MakeFramebuffer("emissive_renderer", WIDTH, HEIGHT);

	// �����ǂݍ���
	audio_manager->SetAudioBankFromJSON(audio.get(), std::filesystem::path("./resources/json_data/audio_data.json"));

	JSONEditor::GetInstance()->Initialize("*.json");
}

// �N���X�I�u�W�F�N�g�̍X�V����
void GamesystemDirector::Update(float elapsed_time)
{
	Graphics* graphics = Graphics::GetInstance();
	graphics->Update();
	GamesystemInput::GetInstance()->Update(elapsed_time);
	light->Update();
	camera->Update();
	scene_manager->Update(elapsed_time);
}

// ImGui�\��
void GamesystemDirector::DebugGUI()
{
	imgui_renderer->Begin();
	char apprication_name[256];
	size_t len;
	wcstombs_s(&len, apprication_name, sizeof(apprication_name), Graphics::W_APPLICATION_NAME, _TRUNCATE);

	// DebugGUI�֐������N���X���ꋓ�\��
	if (ImGui::Begin(apprication_name))
	{
		framebuffer_manager->DebugGUI();
		if (using_class_flag.GetFlag(EnumUsingClass::SHADOW_MAP))
			shadow_map->DebugGUI();
		camera->DebugGUI();
		light->DebugGUI();
		scene_manager->DebugGUI();
		if (using_class_flag.GetFlag(EnumUsingClass::RANDOM_NOISE))
			random_noise->DebugGUI();
		GamesystemInput::GetInstance()->DebugGUI();
	}
	ImGui::End();
}

// �`�揈��
void GamesystemDirector::Render()
{
	// �t���[���o�b�t�@������
	framebuffer_manager->Clear("main",				{ 0.0f,0.0f,0.0f,1.0f });
	framebuffer_manager->Clear("game",				{ 0.0f,0.0f,0.0f,1.0f });
	framebuffer_manager->Clear("emissive_renderer", { 0.0f,0.0f,0.0f,1.0f });

	// �V�[���`��
	scene_manager->Render();

#ifdef USE_IMGUI
	// ImGui�`��
	imgui_renderer->Render();
#endif // USE_IMGUI
}

// �Q�[���I�����ɁA�E�B���h�E��Ԃɂ���
void GamesystemDirector::RestoreWindowMode()
{
	Graphics* graphics = Graphics::GetInstance();
	BOOL fullscreen = 0;
	graphics->GetFullscreenState(&fullscreen, 0);
	if (fullscreen)
	{
		graphics->SetFullscreenState(FALSE, 0);
	}
}

// ShadowMap�쐬
void GamesystemDirector::MakeShadowMap()
{
	shadow_map = std::make_unique<ShadowMap>();
	using_class_flag.SetFlag(EnumUsingClass::SHADOW_MAP, true);
}

// SkyMap�쐬
void GamesystemDirector::MakeSkyMap(const wchar_t* w_filename)
{
	sky_map = std::make_unique<SkyMap>(w_filename);
	using_class_flag.SetFlag(EnumUsingClass::SKY_MAP, true);
}

// FontRenderer�쐬
void GamesystemDirector::MakeFontRenderer(const wchar_t* w_font_name, DirectX::XMFLOAT4 color)
{
	font_renderer = std::make_unique<FontRenderer>(w_font_name, color);
	using_class_flag.SetFlag(EnumUsingClass::FONT_RENDERER, true);
}

// NumberRenderer�쐬
void GamesystemDirector::MakeNumberRenderer()
{
	number_renderer = std::make_unique<NumberRenderer>();
	using_class_flag.SetFlag(EnumUsingClass::NUMBER_RENDERER, true);
}

// RandomNoise�쐬
void GamesystemDirector::MakeRandomNoise()
{
	random_noise = std::make_unique<RandomNoise>();
	using_class_flag.SetFlag(EnumUsingClass::RANDOM_NOISE, true);
}
