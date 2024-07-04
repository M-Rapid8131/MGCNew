// ""�C���N���[�h
// LightBlueEngine
#include "graphics/graphics.h"
#include "game_system/gamesystem_director.h"
#include "scene_loading.h"

//-------------------------------------------
// SceneLoading �����o�֐�
//-------------------------------------------

// �f�X�g���N�^
SceneLoading::~SceneLoading()
{
	if (thread.get())
	{
		thread->join();
		thread.reset(nullptr);
	}
}

// ����������
void SceneLoading::Initialize()
{
	// thread�̍쐬
	thread = std::make_unique<std::jthread>(LoadingThread, this);

	loading_sprite = std::make_unique<Sprite>(L"./resources/sprite/loading.png");
}

// �X�V����
void SceneLoading::Update(float elapsed_time)
{
	SceneManager* scene_manager = GamesystemDirector::GetInstance()->GetSceneManager();
	if (scene && scene->IsReady())
	{
		scene_manager->ChangeScene(scene.release());
	}

	scene_time = elapsed_time;
}

// �`�揈��
void SceneLoading::Render()
{
	// �����Ƀ��[�f�B���O��ʂ̃X�v���C�g�Ƃ��Ȃ񂩂�����
	loading_sprite->Render({ 0.0f,0.0f },
		{ SCast(float, Graphics::GetInstance()->GetScreenWidth()), SCast(float, Graphics::GetInstance()->GetScreenHeight()) }
	);
}

// �}���`�X���b�h�ł��̎��̃V�[����ǂݍ���
void SceneLoading::LoadingThread(SceneLoading* scene)
{
	if (!scene->scene.get())	return;

	CoInitialize(nullptr);

	scene->scene->Initialize();

	CoUninitialize();

	scene->scene->SetReady();
}