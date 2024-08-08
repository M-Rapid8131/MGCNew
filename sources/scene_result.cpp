// ""インクルード
// LightBlueEngine
#include "game_system/gamesystem_director.h"
#include "scene/scene_loading.h"
#include "easing.h"

// ゲームソースファイル
#include "scene_main_manu.h"
#include "scene_game.h"
#include "scene_result.h"

void SceneResult::Initialize()
{
	// 画像読み込み
	result_spr		= std::make_unique<Sprite>(L"resources/sprite/result.png");
	cleared_spr		= std::make_unique<Sprite>(L"resources/sprite/cleared.png");
	game_over_spr	= std::make_unique<Sprite>(L"resources/sprite/game_over.png");
	level_spr		= std::make_unique<Sprite>(L"resources/sprite/level.png");
	block_count_spr	= std::make_unique<Sprite>(L"resources/sprite/block_count.png");
	score_spr		= std::make_unique<Sprite>(L"resources/sprite/score.png");
	//menu_key_spr	= std::make_unique<Sprite>(L"resources/sprite/result_button.png");
}

void SceneResult::Update(float elapsed_time)
{
	FramebufferManager* fb_manager = GamesystemDirector::GetInstance()->GetFramebufferManager();
	Graphics* graphics = Graphics::GetInstance();
	AudioManager* audio_manarger = GamesystemDirector::GetInstance()->GetAudioManager();

	std::unique_ptr<SceneMainManu>	next_scene;
	std::unique_ptr<SceneLoading>	scene_loading;

	Graphics::CbTransition& transition_constants = graphics->GetCbTransition();
	switch (scene_state)
	{
		using enum EnumSceneState;
	case SCENE_SETTING:
		audio_manarger->PlayBGM(EnumBGMBank::RESULT, true);
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
		scene_time += elapsed_time;
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
		audio_manarger->StopBGM();
		next_scene		= std::make_unique<SceneMainManu>();
		scene_loading	= std::make_unique<SceneLoading>(next_scene.release());
		GamesystemDirector::GetInstance()->GetSceneManager()->ChangeScene(scene_loading.release());
		break;

	default:
		break;
	}

	if (scene_time > 5.0f && scene_state == EnumSceneState::SCENE_MAIN)
	{
		scene_state = EnumSceneState::TRANSITION_OUT_SETTING;
	}
}

// ImGui表示
void SceneResult::DebugGUI()
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
	}
#endif
}

// 描画処理
void SceneResult::Render()
{
	Graphics*			graphics			= Graphics::GetInstance();
	GamesystemDirector* director			= GamesystemDirector::GetInstance();
	FramebufferManager* framebuffer_manager = director->GetFramebufferManager();
	NumberRenderer*		number_renderer		= director->GetNumberRenderer();

	float width = SCast(float, graphics->GetScreenWidth());
	float height = SCast(float, graphics->GetScreenHeight());

	const DirectX::XMFLOAT2 SPR_SCALE = { 0.5f, 0.5f };

	GameData& result = director->GetGameData(SCast(size_t, EnumPlayerID::PLAYER_1));

	// フレームバッファに描画(適用範囲を見やすくするため中かっこを使用)
	framebuffer_manager->Activate("main");
	{
		graphics->SetDepthStencilState(EnumDepthState::ZT_OFF_ZW_ON);
		graphics->SetRasterizerState(EnumRasterizerState::SOLID);
		graphics->SetBlendState(EnumBlendState::ALPHA, nullptr, 0xFFFFFFFF);

		float alpha = 0.0f;

		// リザルト画面用の各スプライトを描画
		DirectX::XMFLOAT2	result_size = result_spr->GetSpriteSizeWithScaling(SPR_SCALE);
		alpha = Easing::Out(EnumEasingType::QUART, EnumEasingMode::RATE,
			appear_time.at("result"), appear_time.at("level"), scene_time);

		result_spr->Render(
			{ (width - result_size.x) * 0.5f, MARGIN_S },
			result_size,
			{ 1.0f, 1.0f, 1.0f, alpha }
		);


		DirectX::XMFLOAT2 level_size = level_spr->GetSpriteSizeWithScaling(SPR_SCALE);
		alpha = Easing::Out(EnumEasingType::QUART, EnumEasingMode::RATE,
			appear_time.at("level"), appear_time.at("count"), scene_time);

		DirectX::XMFLOAT4 level_number_color;
		if (result.cleared)
			level_number_color = { 1.0f, 1.0f, 0.0f, alpha };
		else
			level_number_color = { 1.0f, 1.0f, 1.0f, alpha };
		
			level_spr->Render(
			{ MARGIN_L, MARGIN_S * 5.0f },
			level_size,
			{ 1.0f, 1.0f, 1.0f, alpha}
		);

		number_renderer->RenderInt(result.speed_level, { width - MARGIN_L, MARGIN_S * 5.0f },
			SPR_SCALE, EnumNumberAlignment::RIGHT_ALIGNMENT, level_number_color);

		DirectX::XMFLOAT2 block_count_size = block_count_spr->GetSpriteSizeWithScaling(SPR_SCALE);
		alpha = Easing::Out(EnumEasingType::QUART, EnumEasingMode::RATE,
			appear_time.at("count"), appear_time.at("score"), scene_time);

		block_count_spr->Render(
			{ MARGIN_L, MARGIN_S * 8.0f },
			block_count_size,
			{ 1.0f, 1.0f, 1.0f, alpha }
		);

		number_renderer->RenderInt(result.deleted_block_count, { width - MARGIN_L, MARGIN_S * 8.0f },
			SPR_SCALE, EnumNumberAlignment::RIGHT_ALIGNMENT, { 1.0f, 1.0f, 1.0f, alpha });

		DirectX::XMFLOAT2 score_size = score_spr->GetSpriteSizeWithScaling(SPR_SCALE);
		alpha = Easing::Out(EnumEasingType::QUART, EnumEasingMode::RATE,
			appear_time.at("score"), appear_time.at("score") + 0.5f, scene_time);

		score_spr->Render(
			{ MARGIN_L, MARGIN_S * 11.0f },
			score_size,
			{ 1.0f, 1.0f, 1.0f, alpha }
		);

		number_renderer->RenderInt(result.score, { width - MARGIN_L, MARGIN_S * 11.0f },
			SPR_SCALE, EnumNumberAlignment::RIGHT_ALIGNMENT, { 1.0f, 1.0f, 1.0f, alpha });

	}
	framebuffer_manager->Deactivate("main");

	framebuffer_manager->GetFullscreenQuad()->Render(
		framebuffer_manager->GetFramebuffer("main")->GetSceneSRV().GetAddressOf(),
		0, 1,
		nullptr, graphics->GetPixelShader(EnumPixelShader::SCREEN_EFFECT).Get()
	);
}