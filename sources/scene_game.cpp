// <>�C���N���[�h
#include <imgui.h>
#include <implot.h>
#include <ImGuiFileDialog.h>

// ""�C���N���[�h
// LightBlueEngine
#include "game_system/gamesystem_director.h"
#include "game_system/gamesystem_input.h"
#include "scene/scene_loading.h"
#include "easing.h"

// �Q�[���\�[�X�t�@�C��
#include "scene_game.h"
#include "scene_result.h"

// �萔
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
// SceneGame �����o�֐�
//-------------------------------------------

// �f�X�g���N�^
SceneGame::~SceneGame()
{
}

// ����������
void SceneGame::Initialize()
{
	Graphics*	graphics	= Graphics::GetInstance();
	Camera*		camera		= GamesystemDirector::GetInstance()->GetCamera();

	// �J�����쐬�E�ݒ�
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

	// ���C�g�̕�����ݒ�
	auto& cb_light = GamesystemDirector::GetInstance()->GetLight()->GetLightConstants();
	cb_light.directional_light_direction.z = -1.0f;
	cb_light.directional_light_direction.x = 0.0f;
	cb_light.directional_light_direction.y = 0.0f;

	// �Ֆʂ��쐬
	auto player_board = std::make_unique<ObjectBoard>(SCast(UINT, EnumPlayerID::PLAYER_1));
	game_board.emplace_back(std::move(player_board));

	// �u���[�����쐬
	main_bloom		= std::make_unique<BloomEffect>(DEFAULT_MAIN_BLOOM, SCast(UINT, graphics->GetScreenWidth()), SCast(UINT, graphics->GetScreenHeight()));
	emissive_bloom	= std::make_unique<BloomEffect>(DEFAULT_EMISSIVE_BLOOM, SCast(UINT, graphics->GetScreenWidth()), SCast(UINT, graphics->GetScreenHeight()));

	// �p�[�e�B�N�����쐬
	background_particle = std::make_unique<ParticleSystem>("Game1");

	// ��������̉摜��ǂݍ���
	config_move_pad			= std::make_unique<Sprite>(L"./resources/sprite/config/config_move_pad.png");
	config_rotate_pad		= std::make_unique<Sprite>(L"./resources/sprite/config/config_rotate_pad.png");
	config_move_keyboard	= std::make_unique<Sprite>(L"./resources/sprite/config/config_move_keyboard.png");
	config_rotate_keyboard	= std::make_unique<Sprite>(L"./resources/sprite/config/config_rotate_keyboard.png");
}

// �X�V����
void SceneGame::Update(float elapsed_time)
{
	GamesystemDirector* director	= GamesystemDirector::GetInstance();
	FramebufferManager* fb_manager	= director->GetFramebufferManager();
	Graphics*			graphics	= Graphics::GetInstance();

	std::unique_ptr<SceneResult>	next_scene;
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
		next_scene		= std::make_unique<SceneResult>();
		scene_loading	= std::make_unique<SceneLoading>(next_scene.release());
		director->GetSceneManager()->ChangeScene(scene_loading.release());
		break;

	default:
		break;
	}

	//// �Ֆʂ̍X�V
	game_board[SCast(UINT, EnumPlayerID::PLAYER_1)]->Update(elapsed_time);
	game_board[SCast(UINT, EnumPlayerID::PLAYER_1)]->UIUpdate(elapsed_time);

	// �p�[�e�B�N���X�V
	background_particle->Update(elapsed_time);
	if (game_board[SCast(UINT, EnumPlayerID::PLAYER_1)]->GetBoardState().state == EnumBoardState::RESULT
		&& scene_state == EnumSceneState::SCENE_MAIN)
	{
		scene_state = EnumSceneState::TRANSITION_OUT_SETTING;
		director->SaveGameData(game_board[SCast(UINT, EnumPlayerID::PLAYER_1)]->GetGameData());
	}

	scene_time += elapsed_time;
}

// ImGui�\��
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

// �`�揈��
void SceneGame::Render()
{
	Graphics*			graphics			= Graphics::GetInstance();
	FramebufferManager* framebuffer_manager	= GamesystemDirector::GetInstance()->GetFramebufferManager();
	GamesystemInput*	input				= GamesystemInput::GetInstance();

	ParticleSystem::CbParticleEmitter& emitter = background_particle->GetCbParticleEmitter();

	// �t���[���o�b�t�@�ɕ`��(�K�p�͈͂����₷�����邽�ߒ����������g�p)
	framebuffer_manager->Activate("main");
	{
		// �Q�[���V�[��
		framebuffer_manager->Activate("game");
		{
			// �Ֆʕ`��
			graphics->SetDepthStencilState(EnumDepthState::ZT_ON_ZW_ON);
			graphics->SetRasterizerState(EnumRasterizerState::SOLID);
			graphics->SetBlendState(EnumBlendState::ALPHA, nullptr, 0xFFFFFFFF);
			game_board.at(SCast(size_t, EnumPlayerID::PLAYER_1))->Render();

			// �p�[�e�B�N���`��
			graphics->SetDepthStencilState(EnumDepthState::ZT_ON_ZW_OFF);
			graphics->SetRasterizerState(EnumRasterizerState::PARTICLE);
			background_particle->Render();
		}
		framebuffer_manager->Deactivate("game");

		main_bloom->MakeEffect(framebuffer_manager->GetFramebuffer("game")->GetSceneSRV().Get());

		// Emissive�̂ݕ`��V�[��
		framebuffer_manager->Activate("emissive_renderer");
		{
			//// �Ֆʕ`��
			graphics->SetDepthStencilState(EnumDepthState::ZT_ON_ZW_ON);
			graphics->SetRasterizerState(EnumRasterizerState::SOLID);
			graphics->SetBlendState(EnumBlendState::ALPHA, nullptr, 0xFFFFFFFF);
			game_board.at(SCast(size_t, EnumPlayerID::PLAYER_1))->EmissiveRender();

			//// �p�[�e�B�N���`��
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

		// ��������`��
		switch (input->GetInputDevice())
		{
			using enum EnumInputDevice;
		case XBOX:
			config_move_pad->Render(CONFIG_MOVE_POS, config_move_pad->GetSpriteSizeWithScaling({ 0.2f,0.2f }));
			config_rotate_pad->Render(CONFIG_ROTATE_POS, config_rotate_pad->GetSpriteSizeWithScaling({ 0.2f,0.2f }));
			break;

		case KEYBOARD:
			config_move_keyboard->Render(CONFIG_MOVE_POS, config_move_keyboard->GetSpriteSizeWithScaling({ 0.2f,0.2f }));
			config_rotate_keyboard->Render(CONFIG_ROTATE_POS, config_rotate_keyboard->GetSpriteSizeWithScaling({ 0.2f,0.2f }));
			break;

		default:
			break;
		}

	}
	framebuffer_manager->Deactivate("main");

	framebuffer_manager->GetFullscreenQuad()->Render(
		framebuffer_manager->GetFramebuffer("main")->GetSceneSRV().GetAddressOf(),
		0, 1,
		nullptr, graphics->GetPixelShader(EnumPixelShader::SCREEN_EFFECT).Get()
	);
}