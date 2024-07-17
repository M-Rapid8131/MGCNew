// <>インクルード
#include <imgui.h>
#include <implot.h>
#include <ImGuiFileDialog.h>

// ""インクルード
// LightBlueEngine
#include "game_system/gamesystem_director.h"
#include "game_system/gamesystem_input.h"

// ゲームソースファイル
#include "scene_game.h"

// 定数
const DirectX::XMFLOAT2 CONFIG_MOVE_POS		= {
	Graphics::GetInstance()->GetScreenWidth() * 0.7f,
	Graphics::GetInstance()->GetScreenHeight() * 0.05f
};

const DirectX::XMFLOAT2 CONFIG_ROTATE_POS	= { 
	Graphics::GetInstance()->GetScreenWidth() * 0.7f,
	Graphics::GetInstance()->GetScreenHeight() * 0.25f
};

const float				DEFAULT_CAMERA_DISTANCE		= 50.0f;
const DirectX::XMFLOAT3 DEFAULT_CAMERA_TARGET		= { 0.0f,0.0f,0.0f };
const DirectX::XMFLOAT4 DEFAULT_CAMERA_DIRECTION	= { 0.0f,0.0f,-1.0f, 0.0f };

const ParticleSystem::CbParticleEmitter DEFAULT_EMITTER_SETTING = {
	10000,							// emit_amounts
	true,							// random_color
	false,							// disable
	0,								// dummy
	{ 0.0f, 0.0f, 50.0f },			// emit_position
	10.0f,							// emit_speed
	{ 0.0f, 0.0f, 0.0f },			// emit_force
	100.0f,							// emit_accel
	{ 0.0f, 0.0f, -1.0f },			// emit_direction
	0.3f,							// spread_rate
	{ 1.0f, 1.0f, 1.0f, 1.0f },		// emit_color
	0.2f,							// emit_size
	20.0f,							// life_time
	0.01f,							// start_diff
	0.5f							// emit_radius
};

const BloomEffect::CbBloom DEFAULT_MAIN_BLOOM = {
	6,			// downsampled_count
	1,			// blur_buffer_mag
	{0, 0},		// dummy
	0.7f,		// bloom_extraction_threshold
	0.5f,		// bloom_intensity
	1.5f,		// smooth_rate
	0.0f		// dummy
};

const BloomEffect::CbBloom DEFAULT_EMISSIVE_BLOOM = {
	8,			// downsampled_count
	2,			// blur_buffer_mag
	{0, 0},		// dummy
	0.0f,		// bloom_extraction_threshold
	1.0f,		// bloom_intensity
	1.0f,		// smooth_rate
	0.0f		// dummy
};

//-------------------------------------------
// SceneGame メンバ関数
//-------------------------------------------

// デストラクタ
SceneGame::~SceneGame()
{
}

// 初期化処理
void SceneGame::Initialize()
{
	Graphics* graphics = Graphics::GetInstance();

	// カメラ作成
	Camera::TPVData tpv_init = {};
	tpv_init.tpv_target		= DEFAULT_CAMERA_TARGET;
	tpv_init.tpv_direction	= DEFAULT_CAMERA_DIRECTION;
	tpv_init.tpv_distance	= DEFAULT_CAMERA_DISTANCE;

	GamesystemDirector::GetInstance()->GetCamera()->AddTPVCamera(&tpv_init);
	GamesystemDirector::GetInstance()->GetCamera()->SetTPVChannel(1);
	auto& cb_light = GamesystemDirector::GetInstance()->GetLight()->GetLightConstants();
	cb_light.directional_light_direction.z = -1.0f;
	cb_light.directional_light_direction.x = 0.0f;
	cb_light.directional_light_direction.y = 0.0f;

	auto player_board = std::make_unique<ObjectBoard>(SCast(UINT, EnumPlayerID::PLAYER_1));
	game_board.emplace_back(std::move(player_board));

	main_bloom = std::make_unique<BloomEffect>(DEFAULT_MAIN_BLOOM, SCast(UINT, graphics->GetScreenWidth()), SCast(UINT, graphics->GetScreenHeight()));

	emissive_bloom = std::make_unique<BloomEffect>(DEFAULT_EMISSIVE_BLOOM, SCast(UINT, graphics->GetScreenWidth()), SCast(UINT, graphics->GetScreenHeight()));

	background_particle = std::make_unique<ParticleSystem>(DEFAULT_EMITTER_SETTING);

	config_move = std::make_unique<Sprite>(L"./resources/sprite/config_move.png");
	config_rotate = std::make_unique<Sprite>(L"./resources/sprite/config_rotate.png");
}

// 更新処理
void SceneGame::Update(float elapsed_time)
{
	switch (scene_state)
	{
		using enum EnumSceneState;
	case SCENE_SETTING:
		scene_time = 0.0f;
		scene_state = TRANSITION_IN_SETTING;
		/* fall through */

	case TRANSITION_IN_SETTING:
		scene_state = TRANSITION_IN;
		/* fall through */

	case TRANSITION_IN:
		scene_state = SCENE_MAIN;
		break;

	case SCENE_MAIN:
		break;

	case TRANSITION_OUT_SETTING:
		scene_state = TRANSITION_OUT;
		/* fall through */

	case TRANSITION_OUT:
		scene_state = SCENE_CHANGE;
		break;

	case SCENE_CHANGE:
		break;

	default:
		break;
	}

	//// 盤面の更新
	game_board[SCast(UINT, EnumPlayerID::PLAYER_1)]->Update(elapsed_time);
	game_board[SCast(UINT, EnumPlayerID::PLAYER_1)]->UIUpdate(elapsed_time);

	// パーティクル更新
	background_particle->Update(elapsed_time);

	scene_time += elapsed_time;
}

// ImGui表示
void SceneGame::DebugGUI()
{
	float	time = scene_time;
	int		state = GetSceneStateNum();
	float	t_time = transition_time;

#ifdef USE_IMGUI
	if (ImGui::CollapsingHeader("SceneGame"))
	{
		ImGui::InputFloat("SceneTime", &time);
		ImGui::InputInt("SceneState", &state);
		ImGui::InputFloat("TransitionTime", &t_time);
		background_particle->DebugGUI();
		game_board[SCast(UINT, EnumPlayerID::PLAYER_1)]->DebugGUI();
	}
	main_bloom->DebugGUI("Main Bloom");
	emissive_bloom->DebugGUI("Emissive Bloom");
#endif
}

// 描画処理
void SceneGame::Render()
{
	Graphics* graphics = Graphics::GetInstance();
	FramebufferManager* framebuffer_manager = GamesystemDirector::GetInstance()->GetFramebufferManager();

	ParticleSystem::CbParticleEmitter& emitter = background_particle->GetCbParticleEmitter();

	// フレームバッファに描画(適用範囲を見やすくするため中かっこを使用)
	framebuffer_manager->Activate("main");
	{
		// ゲームシーン
		framebuffer_manager->Activate("game");
		{
			// 盤面描画
			graphics->SetDepthStencilState(EnumDepthState::ZT_ON_ZW_ON);
			graphics->SetRasterizerState(EnumRasterizerState::SOLID);
			graphics->SetBlendState(EnumBlendState::ALPHA, nullptr, 0xFFFFFFFF);
			game_board.at(SCast(size_t, EnumPlayerID::PLAYER_1))->Render();

			// パーティクル描画
			graphics->SetDepthStencilState(EnumDepthState::ZT_ON_ZW_OFF);
			graphics->SetRasterizerState(EnumRasterizerState::PARTICLE);
			background_particle->Render();
		}
		framebuffer_manager->Deactivate("game");

		main_bloom->MakeEffect(framebuffer_manager->GetFramebuffer("game")->GetSceneSRV().Get());

		// Emissiveのみ描画シーン
		framebuffer_manager->Activate("emissive_renderer");
		{
			//// 盤面描画
			graphics->SetDepthStencilState(EnumDepthState::ZT_ON_ZW_ON);
			graphics->SetRasterizerState(EnumRasterizerState::SOLID);
			graphics->SetBlendState(EnumBlendState::ALPHA, nullptr, 0xFFFFFFFF);
			game_board.at(SCast(size_t, EnumPlayerID::PLAYER_1))->EmissiveRender();

			//// パーティクル描画
			graphics->SetDepthStencilState(EnumDepthState::ZT_ON_ZW_OFF);
			graphics->SetRasterizerState(EnumRasterizerState::PARTICLE);
			background_particle->Render();
		}
		framebuffer_manager->Deactivate("emissive_renderer");

		emissive_bloom->MakeEffect(framebuffer_manager->GetFramebuffer("emissive_renderer")->GetSceneSRV().Get());

		ID3D11ShaderResourceView* scene_srvs[] = {
			framebuffer_manager->GetFramebuffer("game")->GetSceneSRV().Get(),
			main_bloom->GetSRV(),
			emissive_bloom->GetSRV(),
		};

		framebuffer_manager->GetFullscreenQuad()->Render(
			scene_srvs,
			SCast(uint32_t, EnumTexture::MAIN_TEXTURE), ARRAYSIZE(scene_srvs)
			,nullptr, graphics->GetPixelShader(EnumPixelShader::BLUR).Get()
		);

		graphics->SetDepthStencilState(EnumDepthState::ZT_OFF_ZW_ON);
		graphics->SetRasterizerState(EnumRasterizerState::SOLID);
		graphics->SetBlendState(EnumBlendState::ALPHA, nullptr, 0xFFFFFFFF);
		game_board.at(SCast(size_t, EnumPlayerID::PLAYER_1))->UIRender();

		// スプライト系描画
		if (game_board[0]->GetBoardState().state != EnumBoardState::STANDBY)
		{
			config_move->Render(CONFIG_MOVE_POS, config_move->GetSpriteSizeWithScaling({ 0.2f,0.2f }));
			config_rotate->Render(CONFIG_ROTATE_POS, config_rotate->GetSpriteSizeWithScaling({ 0.2f,0.2f }));
		}
	}
	framebuffer_manager->Deactivate("main");

	framebuffer_manager->GetFullscreenQuad()->Render(
		framebuffer_manager->GetFramebuffer("main")->GetSceneSRV().GetAddressOf(),
		SCast(uint32_t, EnumTexture::MAIN_TEXTURE), 1,
		nullptr, graphics->GetPixelShader(EnumPixelShader::SCREEN_EFFECT).Get()
	);
}