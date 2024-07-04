// <>�C���N���[�h
#include <imgui.h>

// ""�C���N���[�h
// LightBlueEngine
#include "scene_manager.h"

//-------------------------------------------
// SceneManager �����o�֐�
//-------------------------------------------

// �X�V����
void SceneManager::Update(float elapsed_time)
{
	// �V�[���`�F���W����
	if (next_scene)
	{
		// �Â��V�[�����I��
		if (current_scene)
			Clear();

		// �V�����V�[����ݒ�
		current_scene = std::move(next_scene);

		// �V�[������������
		if (!current_scene->IsReady())
		{
			current_scene->Initialize();
		}
	}

	// ���݂̃V�[���̍X�V����
	if (current_scene)
	{
		current_scene->Update(elapsed_time);
	}
}

// ImGui�\��
void SceneManager::DebugGUI()
{
	if (current_scene)
	{
		if (ImGui::CollapsingHeader("Scene"))
		{
			current_scene->DebugGUI();
		}
	}
}

// �`�揈��
void SceneManager::Render()
{
	if (current_scene)
	{
		current_scene->Render();
	}
}

// �V�[���N���A
void SceneManager::Clear()
{
	current_scene.reset();
}

// �V�[���؂�ւ�
void SceneManager::ChangeScene(Scene* new_scene)
{
	// �V�����V�[����ݒ�
	if (!new_scene)		return;
	next_scene.reset(std::move(new_scene));
}