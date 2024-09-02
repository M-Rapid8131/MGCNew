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

// 定数
const float				HSV_MAX						= 360.0f;
const float				HSV_MAX_TIME				= 90.0f;
const float				DEFAULT_CAMERA_DISTANCE		= 80.0f;
const DirectX::XMFLOAT3 DEFAULT_CAMERA_TARGET		= { 0.0f,0.0f,50.0f };
const DirectX::XMFLOAT4 DEFAULT_CAMERA_DIRECTION	= { 0.0f,0.0f,-1.0f, 0.0f };

const ParticleSystem::CbParticleEmitter DEFAULT_EMITTER_SETTING = {
	100000,								// emit_amounts
	false,								// random_color
	false,								// disable
	0,									// dummy
	{ 0.0f, 0.0f, 50.0f },				// emit_position
	10.0f,								// emit_speed
	{ 0.0f, 3.0f, 0.0f },				// emit_force
	10.0f,								// emit_accel
	{ 0.0f, -1.0f, 0.0f },				// emit_direction
	1.0f,								// spread_rate
	{ 1.0f, 1.0f, 1.0f, NormC(200)},	// emit_color
	0.2f,								// emit_size
	10.0f,								// life_time
	0.005f,								// start_diff
	3.0f								// emit_radius
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

// HSVをRGBに変換。時間ごとに色を変化させるために使用
DirectX::XMFLOAT3 HSV2RGB(const DirectX::XMFLOAT3 hsv)
{
	DirectX::XMFLOAT3 rgb = {0.0f, 0.0f, 0.0f};
	if (SCast(int, hsv.y) == 0)
	{
		rgb.x = rgb.y = rgb.z = hsv.z;
	}
	else
	{
		float v_max = hsv.z;
		float v_min = v_max - (hsv.y * v_max);

		
		float	hsv_x	= fmodf(hsv.x, 360.0f);
		int		huei	= (int)(hsv_x / 60.0f);
		float	huef	= hsv_x / 60.0f - (float)(huei);
		float	p		= v_max * (1.0f - hsv.y);
		float	q		= v_max * (1.0f - hsv.y * huef);
		float	t		= v_max * (1.0f - hsv.y * (1 - huef));

		if (huei == 0)
		{
			rgb.x = v_max;
			rgb.y = t;
			rgb.z = p;
		}
		else if (huei == 1)
		{
			rgb.x = q;
			rgb.y = v_max;
			rgb.z = p;
		}
		else if (huei == 2)
		{
			rgb.x = p;
			rgb.y = v_max;
			rgb.z = t;
		}
		else if (huei == 3)
		{
			rgb.x = p;
			rgb.y = q;
			rgb.z = v_max;
		}
		else if (huei == 4)
		{
			rgb.x = t;
			rgb.y = p;
			rgb.z = v_max;
		}
		else if (huei == 5)
		{
			rgb.x = v_max;
			rgb.y = p;
			rgb.z = q;
		}
	}
	return rgb;
}

//-------------------------------------------
// SceneGame メンバ関数
//-------------------------------------------

// デストラクタ
SceneMainManu::~SceneMainManu()
{
}

// 初期化処理
void SceneMainManu::Initialize()
{
	Graphics*		graphics		= Graphics::GetInstance();
	AudioManager*	audio_manarger	= GamesystemDirector::GetInstance()->GetAudioManager();
	audio_manarger->PlayBGM(EnumBGMBank::STANDBY, 1.0f);

	// カメラ作成・設定
	Camera*				camera	= GamesystemDirector::GetInstance()->GetCamera();
	Camera::TPVData*	tpv		= camera->GetTPVCamera(SCast(size_t, EnumCameraChannel::TITLE));
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
	camera->SetTPVChannel(SCast(size_t, EnumCameraChannel::TITLE));

	model = std::make_unique<GameModel>("resources/model/block/block.gltf");

	main_bloom		= std::make_unique<BloomEffect>(DEFAULT_MAIN_BLOOM, SCast(UINT, graphics->GetScreenWidth()), SCast(UINT, graphics->GetScreenHeight()));
	emissive_bloom	= std::make_unique<BloomEffect>(DEFAULT_EMISSIVE_BLOOM, SCast(UINT, graphics->GetScreenWidth()), SCast(UINT, graphics->GetScreenHeight()));

	logo				= std::make_unique<Sprite>(L"resources/sprite/logo.png");

	menu_key.push_back(std::make_unique<Sprite>(L"resources/sprite/config/start_pad.png"));
	menu_key.push_back(std::make_unique<Sprite>(L"resources/sprite/config/start_pspad.png"));
	menu_key.push_back(std::make_unique<Sprite>(L"resources/sprite/config/start_keyboard.png"));

	rule_key.push_back(std::make_unique<Sprite>(L"resources/sprite/config/rule_pad.png"));
	rule_key.push_back(std::make_unique<Sprite>(L"resources/sprite/config/rule_pspad.png"));
	rule_key.push_back(std::make_unique<Sprite>(L"resources/sprite/config/rule_keyboard.png"));

	rule_close_key.push_back(std::make_unique<Sprite>(L"resources/sprite/config/rule_close_pad.png"));
	rule_close_key.push_back(std::make_unique<Sprite>(L"resources/sprite/config/rule_close_pspad.png"));
	rule_close_key.push_back(std::make_unique<Sprite>(L"resources/sprite/config/rule_close_keyboard.png"));

	background_particle = std::make_unique<ParticleSystem>("Title");

	auto& rule1 = rule.emplace_back();
	rule1 = std::make_unique<Sprite>(L"resources/sprite/rule_1.png");
	auto& rule2 = rule.emplace_back();
	rule2 = std::make_unique<Sprite>(L"resources/sprite/rule_2.png");
}

// 更新処理
void SceneMainManu::Update(float elapsed_time)
{
	GamesystemDirector*		director	= GamesystemDirector::GetInstance();
	Camera*					camera		= director->GetCamera();
	Light*					light		= director->GetLight();
	Camera::TPVData*		tpv			= camera->GetTPVCamera();
	GamePad*				game_pad	= GamesystemInput::GetInstance()->GetGamePad();
	Graphics*				graphics	= Graphics::GetInstance();

	std::unique_ptr<SceneGame>		next_scene;
	std::unique_ptr<SceneLoading>	scene_loading;

	Graphics::CbTransition&	transition_constants = graphics->GetCbTransition();
	switch (scene_state)
	{
		using enum EnumSceneState;
	case SCENE_SETTING:
		director->ClearGameData();
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
		next_scene		= std::make_unique<SceneGame>();
		scene_loading	= std::make_unique<SceneLoading>(next_scene.release());
		director->GetSceneManager()->ChangeScene(scene_loading.release());
		break;
	default:
		break;
	}

	// カメラの回転
	DirectX::XMFLOAT3	camera_rot = { 0.0f, ROT_SPEED, 0.0f };

	DirectX::XMVECTOR	v_camera_rot	= DirectX::XMVectorScale(DirectX::XMLoadFloat3(&camera_rot), elapsed_time);
	float				angle			= DirectX::XMVectorGetX(DirectX::XMVector3Length(v_camera_rot));
	DirectX::XMVECTOR	nv_camera_rot	= DirectX::XMVector3Normalize(v_camera_rot);

	DirectX::XMFLOAT3 n_camera_rot;
	DirectX::XMStoreFloat3(&n_camera_rot, nv_camera_rot);

	DirectX::XMVECTOR q_rotation = DirectX::XMVectorSet(
		n_camera_rot.x * sin(angle * 0.5f),
		n_camera_rot.y * sin(angle * 0.5f),
		n_camera_rot.z * sin(angle * 0.5f),
		cos(angle * 0.5f)
	);

	q_rotation = DirectX::XMQuaternionNormalize(q_rotation);

	// カメラの向き
	DirectX::XMVECTOR v_direction = DirectX::XMLoadFloat4(&tpv->tpv_direction);
	v_direction = DirectX::XMVector3Rotate(v_direction, q_rotation);
	DirectX::XMStoreFloat4(&tpv->tpv_direction, v_direction);

	// ライトの向き
	DirectX::XMFLOAT3& light_direction = light->GetLightConstants().directional_light_direction;
	DirectX::XMStoreFloat3(&light_direction, v_direction);

	float hsv_rate = fmodf(scene_time, HSV_MAX_TIME) / HSV_MAX_TIME;
	hsv_color.x = std::lerp(0.0f, HSV_MAX, hsv_rate);

	// ブロック・パーティクルの色の変化
	DirectX::XMFLOAT3 rgb_color = HSV2RGB(hsv_color);
	ParticleSystem::CbParticleEmitter& emitter = background_particle->GetCbParticleEmitter();
	emitter.emit_color.x = rgb_color.x;
	emitter.emit_color.y = rgb_color.y;
	emitter.emit_color.z = rgb_color.z;
	background_particle->Update(elapsed_time);

	if(!viewing_rule)
	{
		if (game_pad->GetButtonDown() & BTN_START)
		{
			switch (selecting_menu)
			{
			case 0:
				director->GetAudioManager()->PlaySE(EnumSEBank::ENTER_GAME);
				scene_state = EnumSceneState::TRANSITION_OUT_SETTING;
				break;
			case 1:
				viewing_rule = true;
				rule_page = 0;
				break;
			default:
				break;
			}
		}
		else if (game_pad->GetButtonDown() & BTN_A)
		{
			viewing_rule = true;
			director->GetAudioManager()->PlaySE(EnumSEBank::STAND);
		}
	}
	else
	{
		if (game_pad->GetButtonDown() & BTN_START)
		{
			viewing_rule = false;
			director->GetAudioManager()->PlaySE(EnumSEBank::STAND);
		}

		else if (game_pad->GetButtonDown() & BTN_LEFT)
		{
			if (rule_page == 1)
			{
				rule_page = 0;
				director->GetAudioManager()->PlaySE(EnumSEBank::LEVEL_UP);
			}
		}
		
		else if (game_pad->GetButtonDown() & BTN_RIGHT)
		{
			if (rule_page == 0)
			{
				rule_page = 1;
				director->GetAudioManager()->PlaySE(EnumSEBank::LEVEL_UP);
			}
		}
	}

	scene_time += elapsed_time;
}

// ImGui表示
void SceneMainManu::DebugGUI()
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
	}
	main_bloom->DebugGUI("Main Bloom");
	emissive_bloom->DebugGUI("Emissive Bloom");
#endif
}

// 描画処理
void SceneMainManu::Render()
{
	Graphics*			graphics			= Graphics::GetInstance();
	FramebufferManager* framebuffer_manager = GamesystemDirector::GetInstance()->GetFramebufferManager();

	GamePad* game_pad = GamesystemInput::GetInstance()->GetGamePad();
	const int INPUT_DEVICE_ID = SCast(int, game_pad->GetInputDevice());

	DirectX::XMFLOAT3 rgb_color = HSV2RGB(hsv_color);

	ParticleSystem::CbParticleEmitter& emitter = background_particle->GetCbParticleEmitter();

	DirectX::XMMATRIX m_scaling = DirectX::XMMatrixScaling(BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE);
	DirectX::XMMATRIX m_rotation
		= DirectX::XMMatrixRotationRollPitchYaw(DirectX::XMConvertToRadians(0.0f), DirectX::XMConvertToRadians(0.0f), 0);
	DirectX::XMMATRIX m_translation
		= DirectX::XMMatrixTranslation(emitter.emit_position.x, emitter.emit_position.y, emitter.emit_position.z);

	DirectX::XMMATRIX m_matrix = m_scaling * m_rotation * m_translation;

	DirectX::XMFLOAT4X4 matrix;
	DirectX::XMStoreFloat4x4(&matrix, m_matrix);

	// フレームバッファに描画(適用範囲を見やすくするため中かっこを使用)
	framebuffer_manager->Activate("main");
	{
		// ゲームシーン
		framebuffer_manager->Activate("game");
		{
			graphics->SetDepthStencilState(EnumDepthState::ZT_ON_ZW_ON);
			graphics->SetRasterizerState(EnumRasterizerState::SOLID);
			graphics->SetBlendState(EnumBlendState::ALPHA, nullptr, 0xFFFFFFFF);
			model->Render(false, matrix, rgb_color);

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
			graphics->SetDepthStencilState(EnumDepthState::ZT_ON_ZW_ON);
			graphics->SetRasterizerState(EnumRasterizerState::SOLID);
			graphics->SetBlendState(EnumBlendState::ALPHA, nullptr, 0xFFFFFFFF);
			model->Render(false, matrix, rgb_color, graphics->GetPixelShader(EnumPixelShader::EXTRACT_EMISSIVE).Get());

			// パーティクル描画
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
			, nullptr, graphics->GetPixelShader(EnumPixelShader::BLUR).Get()
		);

		float width		= SCast(float, Graphics::GetInstance()->GetScreenWidth());
		float height	= SCast(float, Graphics::GetInstance()->GetScreenHeight());

		// ロゴ
		if (viewing_rule)
		{
			rule[rule_page]->Render({ 0.0f, 0.0f }, rule[0]->GetSpriteSizeWithScaling({ 1.0f, 1.0f }));
			DirectX::XMFLOAT2 size = rule_close_key[INPUT_DEVICE_ID]->GetSpriteSizeWithScaling({0.5f, 0.5f});
			rule_close_key[INPUT_DEVICE_ID]->Render({ MARGIN_S, height - size.y}, size);
		}
		else
		{
			DirectX::XMFLOAT2 size = logo->GetSpriteSizeWithScaling({ 0.5f, 0.5f });
			logo->Render({ MARGIN_S, MARGIN_S }, size);

			// ゲームに進むボタンの表示
			EnumInputDevice input_device = GamesystemInput::GetInstance()->GetInputDevice();

			size = rule_key[INPUT_DEVICE_ID]->GetSpriteSizeWithScaling({ 0.5f, 0.5f });
			DirectX::XMFLOAT2 menu_size = menu_key[INPUT_DEVICE_ID]->GetSpriteSizeWithScaling({ 0.5f, 0.5f });

			rule_key[INPUT_DEVICE_ID]->Render({ MARGIN_S, height - size.y - MARGIN_S }, size);
			menu_key[INPUT_DEVICE_ID]->Render({ MARGIN_S, height - menu_size.y - size.y - MARGIN_S }, menu_size);
		}
	}
	framebuffer_manager->Deactivate("main");

	graphics->SetBlendState(EnumBlendState::ALPHA, nullptr, 0xFFFFFFFF);
	framebuffer_manager->GetFullscreenQuad()->Render(
		framebuffer_manager->GetFramebuffer("main")->GetSceneSRV().GetAddressOf(),
		0, 1,
		nullptr, graphics->GetPixelShader(EnumPixelShader::SCREEN_EFFECT).Get()
	);
}