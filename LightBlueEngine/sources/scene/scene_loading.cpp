// ""インクルード
// LightBlueEngine
#include "graphics/graphics.h"
#include "game_system/gamesystem_director.h"
#include "game_system/gamesystem_input.h"
#include "easing.h"
#include "scene_loading.h"

//-------------------------------------------
// SceneLoading メンバ関数
//-------------------------------------------

// デストラクタ
SceneLoading::~SceneLoading()
{
	if (thread.get())
	{
		thread->join();
		thread.reset(nullptr);
	}
}

// 初期化処理
void SceneLoading::Initialize()
{
	// threadの作成
	thread = std::make_unique<std::jthread>(LoadingThread, this);

	loading_sprite	= std::make_unique<Sprite>(L"./resources/sprite/loading_screen.png");
	loading_block	= std::make_unique<Sprite>(L"./resources/sprite/loading_block.png");
}

// 更新処理
void SceneLoading::Update(float elapsed_time)
{
	SceneManager* scene_manager = GamesystemDirector::GetInstance()->GetSceneManager();
	
	if (scene && scene->IsReady())
	{
		scene_manager->ChangeScene(scene.release());
	}

	scene_time += elapsed_time;
	easing_loop += elapsed_time;

	if (easing_loop > EASING_LOOP_TIME)
		easing_loop = 0.0f;
}

// 描画処理
void SceneLoading::Render()
{
	Graphics* graphics = Graphics::GetInstance();
	FramebufferManager* framebuffer_manager = GamesystemDirector::GetInstance()->GetFramebufferManager();

	framebuffer_manager->Activate("main");
	{
		float width = SCast(float, graphics->GetScreenWidth());
		float height = SCast(float, graphics->GetScreenHeight());

		DirectX::XMFLOAT2	spr_size = loading_block->GetSpriteSizeWithScaling({ 1.0f, 1.0f });
		DirectX::XMFLOAT2	spr_half_size = loading_block->GetSpriteSizeWithScaling({ 0.5f, 0.5f });
		float				block_square = spr_size.x * 0.2f;
		float				block_half_square = spr_half_size.x * 0.2f;

		graphics->SetDepthStencilState(EnumDepthState::ZT_OFF_ZW_ON);
		graphics->SetRasterizerState(EnumRasterizerState::SOLID);
		graphics->SetBlendState(EnumBlendState::ALPHA, nullptr, 0xFFFFFFFF);

		loading_sprite->Render(
			{ 0.0f, 0.0f },
			loading_sprite->GetSpriteSizeWithScaling({ 1.0f, 1.0f })
		);

		for (int i = 0; i < 5; i++)
		{
			float spr_x = (width * 0.5f) - (spr_half_size.x * 0.5f) + block_half_square * i;
			float spr_y = (height * 0.5f) - (spr_half_size.y * 0.5f);

			float easing_scale = 0.0f;

			float easing_value = easing_loop + SCast(float, i) * 0.1f;
			if (easing_value > EASING_LOOP_TIME)	easing_value -= EASING_LOOP_TIME;

			if (easing_value < EASING_LOOP_TIME * 0.5f)
			{
				easing_scale = Easing::Out(EnumEasingType::QUART, easing_value);
				loading_block->Render(
					{ spr_x, spr_y + (easing_scale - 1.0f) * block_half_square },
					{ block_half_square, spr_half_size.y },
					{ 1.0f, 1.0f, 1.0f, easing_scale },
					{ 0.0f, 0.0f, 0.0f },
					{ block_square * i, 0.0f },
					{ block_square, spr_size.y }
				);
			}
			else
			{
				easing_scale = Easing::In(EnumEasingType::QUART, easing_value - 1.0f);
				loading_block->Render(
					{ spr_x, spr_y + easing_scale * block_half_square },
					{ block_half_square, spr_half_size.y },
					{ 1.0f, 1.0f, 1.0f, 1.0f - easing_scale },
					{ 0.0f, 0.0f, 0.0f },
					{ block_square * i, 0.0f },
					{ block_square, spr_size.y }
				);
			}
		}
		framebuffer_manager->Deactivate("game");
	}
	framebuffer_manager->Deactivate("main");

	framebuffer_manager->GetFullscreenQuad()->Render(
		framebuffer_manager->GetFramebuffer("main")->GetSceneSRV().GetAddressOf(),
		0, 1
	);

}

// マルチスレッドでこの次のシーンを読み込む
void SceneLoading::LoadingThread(SceneLoading* scene)
{
	if (!scene->scene.get())	return;

	CoInitialize(nullptr);

	scene->scene->Initialize();

	CoUninitialize();

	scene->scene->SetReady();
}