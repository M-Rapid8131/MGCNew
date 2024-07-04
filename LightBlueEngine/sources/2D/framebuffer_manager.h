#ifndef __FRAMEBUFFER_MANAGER_H__
#define __FRAMEBUFFER_MANAGER_H__

// <>�C���N���[�h
#include <vector>
#include <map>
#include <string>
#include <memory>

// ""�C���N���[�h
// LightBlueEngine
#include "framebuffer.h"
#include "fullscreen_quad.h"
#include "singleton.h"

// class >> [FramebufferManager]
// �t���[���o�b�t�@���Ǘ�����N���X�B
class FramebufferManager
{
public:
	// public:�R���X�g���N�^�E�f�X�g���N�^
	FramebufferManager();
	~FramebufferManager();

	//public:�R�s�[�R���X�g���N�^�E���[�u�R���X�g���N�^
	FramebufferManager(const FramebufferManager&)					= delete;
	FramebufferManager& operator = (const FramebufferManager&)		= delete;
	FramebufferManager(FramebufferManager&&) noexcept				= delete;
	FramebufferManager& operator= (FramebufferManager&&) noexcept	= delete;

	// public:�ʏ�֐�
	void DebugGUI();
	void MakeFramebuffer(std::string, uint32_t, uint32_t, bool = true);
	void Clear(std::string, DirectX::XMFLOAT4, float = 1.0f);
	void Activate(std::string);
	void Deactivate(std::string);

	// public:�Q�b�^�[�֐�
	FullscreenQuad* GetFullscreenQuad()				{ return fullscreen_quad.get(); };
	Framebuffer*	GetFramebuffer(std::string);
	size_t			GetFramebufferNum()				{ return framebuffers.size(); }

	// public:�Z�b�^�[�֐�
	void MuteFramebuffer(std::string name, bool mute) { framebuffers.at(name)->MuteFramebuffer(mute); }
	
private:
	// private:�ϐ�
	std::unique_ptr<FullscreenQuad>							fullscreen_quad;

	std::vector<std::string>								name_list;
	std::vector<D3D11_VIEWPORT>								cached_viewports;
	std::map<std::string, std::unique_ptr<Framebuffer>>		framebuffers;
};

#endif // __FRAMEBUFFER_MANAGER_H__