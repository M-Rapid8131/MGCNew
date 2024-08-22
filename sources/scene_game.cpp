// <>インクルード
#include <imgui.h>
#include <implot.h>
#include <ImGuiFileDialog.h>

// ""インクルード
// LightBlueEngine
#include "game_system/gamesystem_director.h"
#include "game_system/gamesystem_input.h"
#include "scene/scene_loading.h"
#include "easing.h"

// ゲームソースファイル
#include "scene_main_manu.h"
#include "scene_game.h"
#include "scene_result.h"

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
	50000,							// emit_amounts
	true,							// random_color
	false,							// disable
	0,								// dummy
	{ 0.0f, -60.0f, 70.0f },		// emit_position
	5.0f,							// emit_speed
	{ 0.0f, 1.0f, 0.0f },			// emit_force
	100.0f,							// emit_accel
	{ 0.0f, -0.5f, -1.0f },			// emit_direction
	0.5f,							// spread_rate
	{ 1.0f, 1.0f, 1.0f, 1.0f },		// emit_color
	0.2f,							// emit_size
	15.0f,							// life_time
	0.01f,							// start_diff
	5.0f							// emit_radius
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

const std::vector<EnumGameMode>	SceneGame::game_mode_list = {
	EnumGameMode::EASY,
	EnumGameMode::NORMAL,
	EnumGameMode::IMPACT,
	EnumGameMode::ENDLESS,
	EnumGameMode::TEMPEST,
};

// デストラクタ
SceneGame::~SceneGame()
{
}

// 初期化処理
void SceneGame::Initialize()
{
	Graphics*	graphics	= Graphics::GetInstance();
	Camera*		camera		= GamesystemDirector::GetInstance()->GetCamera();

	// カメラ作成・設定
	Camera::TPVData* tpv = camera->GetTPVCamera(SCast(size_t, EnumCameraChannel::GAME));
	if (tpv)
	{
		tpv->tpv_target		= DEFAULT_CAMERA_TARGET;
		tpv->tpv_direction	= DEFAULT_CAMERA_DIRECTION;
		tpv->tpv_distance	= DEFAULT_CAMERA_DISTANCE;
	}
	else
	{
		Camera::TPVData tpv_init = {};
		tpv_init.tpv_target		= DEFAULT_CAMERA_TARGET;
		tpv_init.tpv_direction	= DEFAULT_CAMERA_DIRECTION;
		tpv_init.tpv_distance	= DEFAULT_CAMERA_DISTANCE;

		camera->AddTPVCamera(&tpv_init);
	}
	camera->SetTPVChannel(SCast(size_t, EnumCameraChannel::GAME));

	// ライトの方向を設定
	auto& cb_light = GamesystemDirector::GetInstance()->GetLight()->GetLightConstants();
	cb_light.directional_light_direction.z = -1.0f;
	cb_light.directional_light_direction.x = 0.0f;
	cb_light.directional_light_direction.y = 0.0f;

	// 盤面を作成
	auto player_board = std::make_unique<ObjectBoard>(SCast(UINT, EnumPlayerID::PLAYER_1));
	game_board.emplace_back(std::move(player_board));

	// ブルームを作成
	main_bloom		= std::make_unique<BloomEffect>(DEFAULT_MAIN_BLOOM, SCast(UINT, graphics->GetScreenWidth()), SCast(UINT, graphics->GetScreenHeight()));
	emissive_bloom	= std::make_unique<BloomEffect>(DEFAULT_EMISSIVE_BLOOM, SCast(UINT, graphics->GetScreenWidth()), SCast(UINT, graphics->GetScreenHeight()));

	// パーティクルを作成
	background_particle = std::make_unique<ParticleSystem>("Game1");

	const wchar_t* w_game_mode_filename[] = {
		L"resources/sprite/mode/easy.png",
		L"resources/sprite/mode/normal.png",
		L"resources/sprite/mode/impact.png",
		L"resources/sprite/mode/endless.png",
		L"resources/sprite/mode/tempest.png",
	};

	const wchar_t* w_game_mode_detail_filename[] = {
		L"resources/sprite/mode_detail/easy.png",
		L"resources/sprite/mode_detail/normal.png",
		L"resources/sprite/mode_detail/impact.png",
		L"resources/sprite/mode_detail/endless.png",
		L"resources/sprite/mode_detail/tempest.png",
	};

	mode_select = std::make_unique<Sprite>(L"resources/sprite/mode_select.png");

	for (int i = 0; i < game_mode_list.size(); i++)
	{
		auto& mode_spr = game_mode_sprite.emplace_back();
		mode_spr = std::make_unique<Sprite>(w_game_mode_filename[i]);

		auto& detail_spr = game_mode_detail.emplace_back();
		detail_spr = std::make_unique<Sprite>(w_game_mode_detail_filename[i]);
	}

	mode_arrow_u_sprite = std::make_unique<Sprite>(L"resources/sprite/mode_select_up.png");
	mode_arrow_d_sprite = std::make_unique<Sprite>(L"resources/sprite/mode_select_down.png");

	game_board[SCast(UINT, EnumPlayerID::PLAYER_1)]->ChangeBoardColorFromGameMode(game_mode_list[selecting_mode_num]);

	const wchar_t* w_pause_menu_filename[] = {
		L"resources/sprite/resume.png",
		L"resources/sprite/retry.png",
		L"resources/sprite/give_up.png"
	};

	pause_sprite = std::make_unique<Sprite>(L"resources/sprite/pause.png");

	for (int i = 0; i < 3; i++)
	{
		auto& pause_menu_spr = pause_menu.emplace_back();
		pause_menu_spr = std::make_unique<Sprite>(w_pause_menu_filename[i]);
	}
}

// 更新処理
void SceneGame::Update(float elapsed_time)
{
	GamesystemDirector* director		= GamesystemDirector::GetInstance();
	FramebufferManager* fb_manager		= director->GetFramebufferManager();
	AudioManager*		audio_manager	= director->GetAudioManager();
	Graphics*			graphics		= Graphics::GetInstance();

	std::unique_ptr<SceneResult>	next_scene;
	std::unique_ptr<SceneGame>		retry_scene;
	std::unique_ptr<SceneMainManu>	give_up_scene;
	std::unique_ptr<SceneLoading>	scene_loading;

	Graphics::CbTransition& transition_constants = graphics->GetCbTransition();

	switch (scene_state)
	{
		using enum EnumSceneState;
	case SCENE_SETTING:
		scene_time = 0.0f;
		scene_state = TRANSITION_IN_SETTING;
		/* fall through */

	case TRANSITION_IN_SETTING:
		//fb_manager->.reverse = true;
		transition_time = 1.0f;
		transition = true;
		transition_constants.reverse = false;
		transition_constants.transition_prog = 1.0f;
		scene_state = TRANSITION_IN;
		/* fall through */

	case TRANSITION_IN:
		transition_time -= elapsed_time;
		transition_constants.transition_prog = Easing::In(EnumEasingType::CUBIC, transition_time);
		if (transition_time < 0.0f)
		{
			transition = false;
			transition_time = 0.0f;
			scene_state = SCENE_MAIN;
		}
		break;

	case SCENE_MAIN:
		if (game_board[SCast(UINT, EnumPlayerID::PLAYER_1)]->IsPausing())
		{
			pausing_game = true;
			GamesystemInput* input = GamesystemInput::GetInstance();
			const GamePad* PAD = input->GetGamePad();

			if ((PAD->GetButtonDown() & BTN_UP) && selecting_pause_menu_num > 0)
			{
				before_pause_menu_num = selecting_mode_num;
				selecting_pause_menu_num--;
				sprite_move_time = 1.0f;
				audio_manager->PlaySE(EnumSEBank::LEVEL_UP);
			}

			if ((PAD->GetButtonDown() & BTN_DOWN) && selecting_pause_menu_num < 2)
			{
				before_pause_menu_num = selecting_mode_num;
				selecting_pause_menu_num++;
				sprite_move_time = 1.0f;
				audio_manager->PlaySE(EnumSEBank::LEVEL_UP);
			}

			if ((PAD->GetButtonDown() & BTN_START))
			{
				audio_manager->PlaySE(EnumSEBank::STAND);

				switch (selecting_pause_menu_num)
				{
				case 0:
					pausing_game = false;
					break;
				case 1:
				case 2:
					scene_state = EnumSceneState::TRANSITION_OUT_SETTING;
					break;
				default:
					break;
				}
			}
		}

		if (game_board[SCast(UINT, EnumPlayerID::PLAYER_1)]->GetBoardState().state == EnumBoardState::RESULT
			&& scene_state == EnumSceneState::SCENE_MAIN)
		{
			scene_state = EnumSceneState::TRANSITION_OUT_SETTING;
			director->SaveGameData(game_board[SCast(UINT, EnumPlayerID::PLAYER_1)]->GetGameData());
			next_scene = std::make_unique<SceneResult>();
		}
		break;

	case TRANSITION_OUT_SETTING:
		transition_time = 0.0f;
		transition = true;
		transition_constants.reverse = false;
		transition_constants.transition_prog = 0.0f;
		scene_state = TRANSITION_OUT;
		/* fall through */

	case TRANSITION_OUT:
		transition_time += elapsed_time;
		transition_constants.transition_prog = Easing::In(EnumEasingType::CUBIC, transition_time);
		if (transition_time > 1.0f)
		{
			transition = false;
			transition_time = 1.0f;
			scene_state = SCENE_CHANGE;
		}
		break;

	case SCENE_CHANGE:
		if(pausing_game)
		{
			switch (selecting_pause_menu_num)
			{
			case 0:
				break;
			case 1:
				retry_scene = std::make_unique<SceneGame>();
				scene_loading = std::make_unique<SceneLoading>(retry_scene.release());
				break;
			case 2:
				give_up_scene = std::make_unique<SceneMainManu>();
				scene_loading = std::make_unique<SceneLoading>(give_up_scene.release());
				break;
			default:
				break;
			}
		}
		else
		{
			next_scene = std::make_unique<SceneResult>();
			scene_loading = std::make_unique<SceneLoading>(next_scene.release());
		}

		director->GetSceneManager()->ChangeScene(scene_loading.release());
		break;

	default:
		break;
	}

	//// 盤面の更新
	game_board[SCast(UINT, EnumPlayerID::PLAYER_1)]->Update(elapsed_time);
	game_board[SCast(UINT, EnumPlayerID::PLAYER_1)]->UIUpdate(elapsed_time);

	if (!playing_game)
	{
		GamesystemInput* input = GamesystemInput::GetInstance();
		const GamePad* PAD = input->GetGamePad();

		if ((PAD->GetButtonDown() & BTN_UP) && selecting_mode_num > 0)
		{
			selecting_mode_num--;
			sprite_move_time = -1.0f;
			game_board[SCast(UINT, EnumPlayerID::PLAYER_1)]->ChangeBoardColorFromGameMode(game_mode_list[selecting_mode_num]);
			audio_manager->PlaySE(EnumSEBank::LEVEL_UP);
		}

		if ((PAD->GetButtonDown() & BTN_DOWN) && selecting_mode_num < game_mode_list.size() - 1)
		{
			selecting_mode_num++;
			sprite_move_time = 1.0f;
			game_board[SCast(UINT, EnumPlayerID::PLAYER_1)]->ChangeBoardColorFromGameMode(game_mode_list[selecting_mode_num]);
			audio_manager->PlaySE(EnumSEBank::LEVEL_UP);
		}

		if ((PAD->GetButtonDown() & BTN_START))
		{
			game_board[SCast(UINT, EnumPlayerID::PLAYER_1)]->GameStart(SCast(int, game_mode_list.at(selecting_mode_num)));
			playing_game = true;
			audio_manager->PlaySE(EnumSEBank::STAND);
		}
	}

	// パーティクル更新
	background_particle->Update(elapsed_time);

	//if (game_board[SCast(UINT, EnumPlayerID::PLAYER_1)]->ChangeParticleType())
	//{
	//	int level = game_board[SCast(UINT, EnumPlayerID::PLAYER_1)]->GetGameData().speed_level;
	//	std::string key = "Game" + std::to_string(level / 10 + 1);
	//	background_particle->SetEmitterConstantsFromJSON(key);
	//	game_board[SCast(UINT, EnumPlayerID::PLAYER_1)]->ChangedParticleType();
	//}

	if (sprite_move_time > 0.0f)
	{
		sprite_move_time -= elapsed_time;
		if (sprite_move_time < 0.0f)
		{
			sprite_move_time = 0.0f;
		}
		move_easing_rate = Easing::In(EnumEasingType::QUINT, sprite_move_time);
		sprite_move = move_easing_rate * MENU_DISTANCE;
	}
	else if (sprite_move_time < 0.0f)
	{
		sprite_move_time += elapsed_time;
		if (sprite_move_time > 0.0f)
		{
			sprite_move_time = 0.0f;
		}
		move_easing_rate = Easing::In(EnumEasingType::QUINT, -sprite_move_time);
		sprite_move = move_easing_rate * -MENU_DISTANCE;
	}

	scene_time += elapsed_time;
}

// ImGui表示
void SceneGame::DebugGUI()
{
	float	time	= scene_time;
	int		state	= GetSceneStateNum();
	float	t_time	= transition_time;

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
	Graphics*			graphics			= Graphics::GetInstance();
	FramebufferManager* framebuffer_manager	= GamesystemDirector::GetInstance()->GetFramebufferManager();
	GamesystemInput*	input				= GamesystemInput::GetInstance();

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
			0, ARRAYSIZE(scene_srvs)
			,nullptr, graphics->GetPixelShader(EnumPixelShader::BLUR).Get()
		);

		graphics->SetDepthStencilState(EnumDepthState::ZT_OFF_ZW_ON);
		graphics->SetRasterizerState(EnumRasterizerState::SOLID);
		graphics->SetBlendState(EnumBlendState::ALPHA, nullptr, 0xFFFFFFFF);
		game_board.at(SCast(size_t, EnumPlayerID::PLAYER_1))->UIRender();

		// モード選択画面
		if (!playing_game)
		{
			float width		= SCast(float, graphics->GetScreenWidth());
			float height	= SCast(float, graphics->GetScreenHeight());
			float spr_wdt	= game_mode_sprite[selecting_mode_num]->GetSpriteSizeWithScaling({ 0.5f, 0.5f }).x;
			float spr_hgt	= game_mode_sprite[selecting_mode_num]->GetSpriteSizeWithScaling({ 0.5f, 0.5f }).y;

			// 選択中のモード
			game_mode_sprite[selecting_mode_num]->Render(
				{ (width - spr_wdt) * 0.5f, (height - spr_hgt) * 0.5f + sprite_move },
				{ spr_wdt, spr_hgt }
			);

			float spr_wdt_select = mode_select->GetSpriteSizeWithScaling({ 0.4f, 0.4f }).x;
			float spr_hgt_select = mode_select->GetSpriteSizeWithScaling({ 0.4f, 0.4f }).y;

			mode_select->Render(
				{ MOVE_FACTOR, (height - spr_hgt_select) * 0.5f },
				{ spr_wdt_select, spr_hgt_select }
			);

			float spr_wdt_arrow = mode_arrow_d_sprite->GetSpriteSizeWithScaling({ 0.5f, 0.5f }).x;
			float spr_hgt_arrow = mode_arrow_d_sprite->GetSpriteSizeWithScaling({ 0.5f, 0.5f }).y;

			if (selecting_mode_num > 0)
			{
				DirectX::XMFLOAT4 arrow_color;
				arrow_color.x = BOARD_COLOR_SET[SCast(int, game_mode_list[selecting_mode_num - 1])].x;
				arrow_color.y = BOARD_COLOR_SET[SCast(int, game_mode_list[selecting_mode_num - 1])].y;
				arrow_color.z = BOARD_COLOR_SET[SCast(int, game_mode_list[selecting_mode_num - 1])].z;
				arrow_color.w = 1.0f;

				mode_arrow_u_sprite->Render(
					{ (width - spr_wdt) * 0.5f, (height - spr_hgt) * 0.5f - spr_hgt_arrow + sprite_move },
					{ spr_wdt_arrow, spr_hgt_arrow },
					arrow_color
					);

				mode_arrow_u_sprite->Render(
					{ (width + spr_wdt) * 0.5f - spr_wdt_arrow, (height - spr_hgt) * 0.5f - spr_hgt_arrow + sprite_move },
					{ spr_wdt_arrow, spr_hgt_arrow },
					arrow_color
					);
			}

			if (selecting_mode_num < game_mode_list.size() - 1)
			{
				DirectX::XMFLOAT4 arrow_color;
				arrow_color.x = BOARD_COLOR_SET[SCast(int, game_mode_list[selecting_mode_num + 1])].x;
				arrow_color.y = BOARD_COLOR_SET[SCast(int, game_mode_list[selecting_mode_num + 1])].y;
				arrow_color.z = BOARD_COLOR_SET[SCast(int, game_mode_list[selecting_mode_num + 1])].z;
				arrow_color.w = 1.0f;

				mode_arrow_d_sprite->Render(
					{ (width - spr_wdt) * 0.5f, (height + spr_hgt_arrow) * 0.5f + sprite_move },
					{ spr_wdt_arrow, spr_hgt_arrow },
					arrow_color
					);
				
				mode_arrow_d_sprite->Render(
					{ (width + spr_wdt) * 0.5f - spr_wdt_arrow, (height + spr_hgt_arrow) * 0.5f + sprite_move },
					{ spr_wdt_arrow, spr_hgt_arrow },
					arrow_color
					);
			}

			float spr_wdt_detail = game_mode_detail[selecting_mode_num]->GetSpriteSizeWithScaling({ 0.5f, 0.5f }).x;
			float spr_hgt_detail = game_mode_detail[selecting_mode_num]->GetSpriteSizeWithScaling({ 0.5f, 0.5f }).y;

			// 選択中のモードの詳細
			game_mode_detail[selecting_mode_num]->Render(
				{ (width - spr_wdt_detail) - MOVE_FACTOR - fabsf(sprite_move), (height - spr_hgt_detail) * 0.5f },
				{ spr_wdt_detail, spr_hgt_detail },
				{ 1.0f, 1.0f, 1.0f, (1.0f - move_easing_rate) }
			);


			float spr_wdt_small = game_mode_sprite[selecting_mode_num]->GetSpriteSizeWithScaling({ 0.2f, 0.2f }).x;
			float spr_hgt_small = game_mode_sprite[selecting_mode_num]->GetSpriteSizeWithScaling({ 0.2f, 0.2f }).y;

			for (int i = 1; selecting_mode_num - i >= 0; i++)
			{
				game_mode_sprite[selecting_mode_num - i]->Render(
					{ (width - spr_wdt_small) * 0.5f, (height - spr_hgt_small) * 0.5f - MENU_DISTANCE * i + sprite_move },
					{ spr_wdt_small, spr_hgt_small }
				);
			}

			for (int i = 1; selecting_mode_num + i <= game_mode_list.size() - 1; i++)
			{
				game_mode_sprite[selecting_mode_num + i]->Render(
					{ (width - spr_wdt_small) * 0.5f, (height - spr_hgt_small) * 0.5f + MENU_DISTANCE * i + sprite_move },
					{ spr_wdt_small, spr_hgt_small }
				);
			}
		}

		if (game_board[SCast(UINT, EnumPlayerID::PLAYER_1)]->IsPausing())
		{
			float width		= SCast(float, graphics->GetScreenWidth());
			float height	= SCast(float, graphics->GetScreenHeight());
			float spr_wdt	= pause_sprite->GetSpriteSizeWithScaling({ 0.5f, 0.5f }).x;
			float spr_hgt	= pause_sprite->GetSpriteSizeWithScaling({ 0.5f, 0.5f }).y;

			pause_sprite->Render(
				{ (width - spr_wdt) * 0.5f, 20.0f },
				{ spr_wdt, spr_hgt }
			);

			for (int i = 0; i < 3; i++)
			{
				float spr_wdt_menu = pause_menu[i]->GetSpriteSizeWithScaling({ 0.4f, 0.4f }).x;
				float spr_hgt_menu = pause_menu[i]->GetSpriteSizeWithScaling({ 0.4f, 0.4f }).y;

				float color_factor = (i == selecting_pause_menu_num) 
					? 0.5f + (1.0f - move_easing_rate) * 0.5f 
					: (i == before_pause_menu_num) 
						? 0.5f + move_easing_rate * 0.5f
						: 0.5f;

				pause_menu[i]->Render(
					{ (width - spr_wdt_menu) * 0.5f, (height - spr_hgt_menu) * 0.5f + MENU_DISTANCE * (i - 1)},
					{ spr_wdt_menu, spr_hgt_menu },
					{ color_factor, color_factor, color_factor, 1.0f}
				);
			}
		}
	}
	framebuffer_manager->Deactivate("main");

	framebuffer_manager->GetFullscreenQuad()->Render(
		framebuffer_manager->GetFramebuffer("main")->GetSceneSRV().GetAddressOf(),
		0, 1,
		nullptr, graphics->GetPixelShader(EnumPixelShader::SCREEN_EFFECT).Get()
	);
}