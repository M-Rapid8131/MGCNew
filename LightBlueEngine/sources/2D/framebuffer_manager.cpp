// <>�C���N���[�h
#include <memory>
#include <imgui.h>

// ""�C���N���[�h
// LightBlueEngine
#include "engine_utility.h"
#include "framebuffer_manager.h"

//-------------------------------------------
// FramebufferManager �����o�֐�
//-------------------------------------------

// �R���X�g���N�^
FramebufferManager::FramebufferManager()
{
	fullscreen_quad = std::make_unique<FullscreenQuad>();
	cached_viewports.resize(D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE);
}

// �f�X�g���N�^
FramebufferManager::~FramebufferManager()
{
	fullscreen_quad.reset();

	for (auto& fb_ptr : framebuffers)
	{
		fb_ptr.second.reset();
	}
	framebuffers.clear();
}

// ImGui�\��
void FramebufferManager::DebugGUI()
{
	if (ImGui::CollapsingHeader("FramebufferManager"))
	{
		for (std::string name : name_list)
		{
			ImGui::Text(name.c_str());
			ImGui::Image(RCast(void*, framebuffers.at(name)->GetSceneSRV().Get()), ImVec2(1280.0f / 4.0f, 720.0f / 4.0f));
			ImGui::Image(RCast(void*, framebuffers.at(name)->GetDepthSRV().Get()), ImVec2(1280.0f / 4.0f, 720.0f / 4.0f));
			ImGui::Separator();
		}
	}
}

// �t���[���o�b�t�@�쐬
void FramebufferManager::MakeFramebuffer(std::string name, uint32_t width, uint32_t height, bool use_dsv)
{
	auto obj = std::make_unique<Framebuffer>(width, height, use_dsv);
	framebuffers.emplace(name, std::move(obj));
	name_list.push_back(name);
}

// �t���[���o�b�t�@������
void FramebufferManager::Clear(std::string name, DirectX::XMFLOAT4 color, float depth)
{
	if (!framebuffers.at(name))  return;
	framebuffers.at(name)->Clear(color, depth);
}

// �t���[���o�b�t�@�N��
void FramebufferManager::Activate(std::string name) 
{
	if (!framebuffers.at(name))  return;
	framebuffers.at(name)->Activate(cached_viewports.data());
}

// �t���[���o�b�t�@�I��
void FramebufferManager::Deactivate(std::string name) 
{
	if (!framebuffers.at(name))  return;
	framebuffers.at(name)->Deactivate(cached_viewports.data());
}

// �t���[���o�b�t�@�擾
Framebuffer* FramebufferManager::GetFramebuffer(std::string name)
{
	if (!framebuffers.at(name))  return	nullptr;
	return framebuffers.at(name).get();
}