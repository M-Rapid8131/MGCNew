// <>インクルード
#include <imgui.h>
#include <implot.h>

// ""インクルード
// LightBlueEngine
#include "graphics/graphics.h"
#include "scene/scene_loading.h"
#include "gamesystem_director.h"
#include "game_system/gamesystem_input.h"

#include "easing.h"
#include "json_editor.h"

// ゲームソースファイル
#include "scene_main_manu.h"
#include "scene_game.h"

//-------------------------------------------
// GamesystemDirector メンバ関数
//-------------------------------------------

// 初期化処理
void GamesystemDirector::Initialize(HWND hwnd, EnumCameraMode& start_mode)
{
	// 各必須クラスオブジェクトの初期化
	Graphics* graphics = Graphics::GetInstance();
	graphics->Initialize(hwnd, windowed);
	graphics->LoadTransitionTexture(L"resources/sprite/screen_disolve.png");
	graphics->LoadTransitionBackTexture(L"resources/sprite/loading_screen.png");

	GamesystemInput::GetInstance()->Initialize(graphics->GetHWND());

	audio					= std::make_unique<Audio>();
	audio_manager			= std::make_unique<AudioManager>();
	camera					= std::make_unique<Camera>(start_mode);
	framebuffer_manager		= std::make_unique<FramebufferManager>();
	imgui_renderer			= std::make_unique<ImGuiRenderer>(hwnd);
	light					= std::make_unique<Light>();
	MakeRandomNoise();
	MakeNumberRenderer();
	scene_manager			= std::make_unique<SceneManager>();

	auto start_scene	= std::make_unique<SceneMainManu>();
	auto scene_loading	= std::make_unique<SceneLoading>(start_scene.release());

	scene_manager->ChangeScene(scene_loading.release());

	const uint32_t WIDTH	= graphics->GetScreenWidth();
	const uint32_t HEIGHT	= graphics->GetScreenHeight();

	framebuffer_manager->MakeFramebuffer("main", WIDTH, HEIGHT);
	framebuffer_manager->MakeFramebuffer("game", WIDTH, HEIGHT);
	framebuffer_manager->MakeFramebuffer("emissive_renderer", WIDTH, HEIGHT);

	// 音声読み込み
	audio_manager->SetAudioBankFromJSON(audio.get(), std::filesystem::path("./resources/json_data/audio_data.json"));

	JSONEditor::GetInstance()->Initialize("*.json");
}

// クラスオブジェクトの更新処理
void GamesystemDirector::Update(float elapsed_time)
{
	Graphics*			graphics	= Graphics::GetInstance();
	GamesystemInput*	input		= GamesystemInput::GetInstance();
	graphics->Update();
	input->Update(elapsed_time);
	light->Update();
	camera->Update();
	scene_manager->Update(elapsed_time);

#ifdef USE_IMGUI
	if ((input->GetGamePad(0)->GetButton() & BTN_UP) && (input->GetGamePad(0)->GetButtonDown() & BTN_DOWN))
		show_imgui = !show_imgui;
#endif // USE_IMGUI

}

// ImGui表示
void GamesystemDirector::DebugGUI()
{
	imgui_renderer->Begin();
	char apprication_name[256];
	size_t len;
	wcstombs_s(&len, apprication_name, sizeof(apprication_name), Graphics::W_APPLICATION_NAME, _TRUNCATE);

	// DebugGUI関数を持つクラスを一挙表示
	if(show_imgui)
	{
		if (ImGui::Begin(apprication_name))
		{
			framebuffer_manager->DebugGUI();
			if (using_class_flag.GetFlag(EnumUsingClass::SHADOW_MAP))
				shadow_map->DebugGUI();
			Graphics::GetInstance()->DebugGUI();
			camera->DebugGUI();
			light->DebugGUI();
			scene_manager->DebugGUI();
			if (using_class_flag.GetFlag(EnumUsingClass::RANDOM_NOISE))
				random_noise->DebugGUI();
			GamesystemInput::GetInstance()->DebugGUI();

#if 1
			if (ImPlot::BeginPlot("easing_graph"))
			{
				float x_data[1001];
				float y_data[1001];

				for (int i = 0.; i <= 1000; i++)
				{
					x_data[i] = (float)i * 0.001f;
					y_data[i] = Easing::In(EnumEasingType::BOUNCE, x_data[i]);
				}

				ImPlot::PlotLine("Ease Line", x_data, y_data, 1001);
				ImPlot::EndPlot();
			}
#endif
		}
		ImGui::End();
	}
}

// 描画処理
void GamesystemDirector::Render()
{
	// フレームバッファ初期化
	framebuffer_manager->Clear("main",				{ 0.0f,0.0f,0.0f,1.0f });
	framebuffer_manager->Clear("game",				{ 0.0f,0.0f,0.0f,1.0f });
	framebuffer_manager->Clear("emissive_renderer", { 0.0f,0.0f,0.0f,1.0f });

	// シーン描画
	scene_manager->Render();

#ifdef USE_IMGUI
	// ImGui描画
	imgui_renderer->Render();
#endif // USE_IMGUI
}

// ゲーム終了時に、ウィンドウ状態にする
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

// ShadowMap作成
void GamesystemDirector::MakeShadowMap()
{
	shadow_map = std::make_unique<ShadowMap>();
	using_class_flag.SetFlag(EnumUsingClass::SHADOW_MAP, true);
}

// SkyMap作成
void GamesystemDirector::MakeSkyMap(const wchar_t* w_filename)
{
	sky_map = std::make_unique<SkyMap>(w_filename);
	using_class_flag.SetFlag(EnumUsingClass::SKY_MAP, true);
}

// FontRenderer作成
void GamesystemDirector::MakeFontRenderer(const wchar_t* w_font_name, DirectX::XMFLOAT4 color)
{
	font_renderer = std::make_unique<FontRenderer>(w_font_name, color);
	using_class_flag.SetFlag(EnumUsingClass::FONT_RENDERER, true);
}

// NumberRenderer作成
void GamesystemDirector::MakeNumberRenderer()
{
	number_renderer = std::make_unique<NumberRenderer>();
	using_class_flag.SetFlag(EnumUsingClass::NUMBER_RENDERER, true);
}

// RandomNoise作成
void GamesystemDirector::MakeRandomNoise()
{
	random_noise = std::make_unique<RandomNoise>();
	using_class_flag.SetFlag(EnumUsingClass::RANDOM_NOISE, true);
}
