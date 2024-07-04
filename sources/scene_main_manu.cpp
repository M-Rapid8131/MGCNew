// <>�C���N���[�h
#include <imgui.h>
#include <implot.h>
#include <ImGuiFileDialog.h>

// ""�C���N���[�h
// LightBlueEngine
#include "game_system/gamesystem_director.h"
#include "game_system/gamesystem_input.h"

// �Q�[���\�[�X�t�@�C��
#include "scene_main_manu.h"

// �萔
const float				HSV_MAX = 360.0f;
const float				DEFAULT_CAMERA_DISTANCE = 80.0f;
const DirectX::XMFLOAT3 DEFAULT_CAMERA_TARGET = { 0.0f,0.0f,50.0f };
const DirectX::XMFLOAT4 DEFAULT_CAMERA_DIRECTION = { 0.0f,0.0f,-1.0f, 0.0f };

ParticleSystem::CbParticleEmitter DEFAULT_EMITTER_SETTING = {
	100000,							// emit_amounts
	{ 0, 0, 0 },					// dummy
	{ 0.0f, 0.0f, 50.0f, 0.0f },	// emit_position
	{ 0.0f, 0.0f, 0.0f, 0.0f },		// emit_amplitude
	{ 0.0f, 0.0f, -1.0f, 0.0f },	// emit_direction
	{ 1.0f, 1.0f, 1.0f, 0.8f },		// emit_color
	1.0f,							// spread_rate
	0.15f,							// emit_size
	10.0f,							// emit_speed
	0.0f,							// emit_accel
	10.0f,							// life_time
	0.002f,							// start_diff
	{ 0.0f, 0.0f }					// dummy
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

// HSV��RGB�ɕϊ��B���Ԃ��ƂɐF��ω������邽�߂Ɏg�p
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

		
		float hsv_x = fmodf(hsv.x, 360.0f);
		float huei = (int)(hsv_x / 60);
		float huef = hsv_x / 60 - huei;
		float p = v_max * (1.0f - hsv.y);
		float q = v_max * (1.0f - hsv.y * huef);
		float t = v_max * (1.0f - hsv.y * (1 - huef));

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
// SceneGame �����o�֐�
//-------------------------------------------

// �f�X�g���N�^
SceneMainManu::~SceneMainManu()
{
}

// ����������
void SceneMainManu::Initialize()
{
	Graphics*		graphics		= Graphics::GetInstance();
	AudioManager*	audio_manarger	= GamesystemDirector::GetInstance()->GetAudioManager();
	audio_manarger->PlayBGM(EnumBGMBank::STANDBY, 0.3f);

	// �J�����쐬
	Camera::TPVData tpv_init = {};
	tpv_init.tpv_target		= DEFAULT_CAMERA_TARGET;
	tpv_init.tpv_direction	= DEFAULT_CAMERA_DIRECTION;
	tpv_init.tpv_distance	= DEFAULT_CAMERA_DISTANCE;
	GamesystemDirector::GetInstance()->GetCamera()->AddTPVCamera(&tpv_init);

	model = std::make_unique<GameModel>("resources/model/block/block.gltf");

	main_bloom		= std::make_unique<BloomEffect>(DEFAULT_MAIN_BLOOM, SCast(UINT, graphics->GetScreenWidth()), SCast(UINT, graphics->GetScreenHeight()));
	emissive_bloom	= std::make_unique<BloomEffect>(DEFAULT_EMISSIVE_BLOOM, SCast(UINT, graphics->GetScreenWidth()), SCast(UINT, graphics->GetScreenHeight()));

	logo = std::make_unique<Sprite>(L"resources/sprite/logo.png");

	background_particle = std::make_unique<ParticleSystem>(DEFAULT_EMITTER_SETTING);
}

// �X�V����
void SceneMainManu::Update(float elapsed_time)
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

	Camera*				camera	= GamesystemDirector::GetInstance()->GetCamera();
	Light*				light	= GamesystemDirector::GetInstance()->GetLight();
	Camera::TPVData*	tpv		= camera->GetTPVCamera();

	DirectX::XMFLOAT3	camera_rot = { 0.0f, 0.2f, 0.0f };

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
	DirectX::XMVECTOR v_direction = DirectX::XMLoadFloat4(&tpv->tpv_direction);
	v_direction = DirectX::XMVector3Rotate(v_direction, q_rotation);
	DirectX::XMStoreFloat4(&tpv->tpv_direction, v_direction);
	DirectX::XMFLOAT3& light_direction = light->GetLightConstants().directional_light_direction;
	DirectX::XMStoreFloat3(&light_direction, v_direction);

	hsv_color.x = std::lerp(0.0f, HSV_MAX, fmodf(scene_time, HSV_MAX * 0.3f) / (HSV_MAX * 0.3f));

	DirectX::XMFLOAT3 rgb_color = HSV2RGB(hsv_color);
	ParticleSystem::CbParticleEmitter& emitter = background_particle->GetCbParticleEmitter();
	emitter.emit_color.x = rgb_color.x;
	emitter.emit_color.y = rgb_color.y;
	emitter.emit_color.z = rgb_color.z;
	background_particle->Update(elapsed_time);

	scene_time += elapsed_time;
}

// ImGui�\��
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

// �`�揈��
void SceneMainManu::Render()
{
	Graphics*			graphics			= Graphics::GetInstance();
	FramebufferManager* framebuffer_manager = GamesystemDirector::GetInstance()->GetFramebufferManager();

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

	// �t���[���o�b�t�@�ɕ`��(�K�p�͈͂����₷�����邽�ߒ����������g�p)
	framebuffer_manager->Activate("main");
	{
		// �Q�[���V�[��
		framebuffer_manager->Activate("game");
		{
			graphics->SetDepthStencilState(EnumDepthState::ZT_ON_ZW_ON);
			graphics->SetRasterizerState(EnumRasterizerState::SOLID);
			graphics->SetBlendState(EnumBlendState::ALPHA, nullptr, 0xFFFFFFFF);
			model->Render(false, matrix, rgb_color);

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
			graphics->SetDepthStencilState(EnumDepthState::ZT_ON_ZW_ON);
			graphics->SetRasterizerState(EnumRasterizerState::CULL_NONE);
			graphics->SetBlendState(EnumBlendState::ALPHA, nullptr, 0xFFFFFFFF);
			model->Render(false, matrix, rgb_color, graphics->GetPixelShader(EnumPixelShader::EXTRACT_EMISSIVE).Get());

			// �p�[�e�B�N���`��
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
			, nullptr, graphics->GetPixelShader(EnumPixelShader::BLUR).Get()
		);

		float width		= SCast(float, Graphics::GetInstance()->GetScreenWidth());
		float height	= SCast(float, Graphics::GetInstance()->GetScreenHeight());

		DirectX::XMFLOAT2 size = logo->GetSpriteSizeWithScaling({ 0.5f, 0.5f });
		logo->Render({ 0.0f, 0.0f }, size);
	}
	framebuffer_manager->Deactivate("main");

	framebuffer_manager->GetFullscreenQuad()->Render(
		framebuffer_manager->GetFramebuffer("main")->GetSceneSRV().GetAddressOf(),
		SCast(uint32_t, EnumTexture::MAIN_TEXTURE), 1,
		nullptr, graphics->GetPixelShader(EnumPixelShader::SCREEN_EFFECT).Get()
	);
}