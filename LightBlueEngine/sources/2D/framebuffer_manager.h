#ifndef __FRAMEBUFFER_MANAGER_H__
#define __FRAMEBUFFER_MANAGER_H__

// <>インクルード
#include <vector>
#include <map>
#include <string>
#include <memory>

// ""インクルード
// LightBlueEngine
#include "framebuffer.h"
#include "fullscreen_quad.h"
#include "singleton.h"

// class >> [FramebufferManager]
// フレームバッファを管理するクラス。
class FramebufferManager
{
public:
	// public:コンストラクタ・デストラクタ
	FramebufferManager();
	~FramebufferManager();

	//public:コピーコンストラクタ・ムーブコンストラクタ
	FramebufferManager(const FramebufferManager&)					= delete;
	FramebufferManager& operator = (const FramebufferManager&)		= delete;
	FramebufferManager(FramebufferManager&&) noexcept				= delete;
	FramebufferManager& operator= (FramebufferManager&&) noexcept	= delete;

	// public:通常関数
	void DebugGUI();
	void MakeFramebuffer(std::string, uint32_t, uint32_t, bool = true);
	void Clear(std::string, DirectX::XMFLOAT4, float = 1.0f);
	void Activate(std::string);
	void Deactivate(std::string);

	// public:ゲッター関数
	FullscreenQuad* GetFullscreenQuad()				{ return fullscreen_quad.get(); };
	Framebuffer*	GetFramebuffer(std::string);
	size_t			GetFramebufferNum()				{ return framebuffers.size(); }

	// public:セッター関数
	void MuteFramebuffer(std::string name, bool mute) { framebuffers.at(name)->MuteFramebuffer(mute); }
	
private:
	// private:変数
	std::unique_ptr<FullscreenQuad>							fullscreen_quad;

	std::vector<std::string>								name_list;
	std::vector<D3D11_VIEWPORT>								cached_viewports;
	std::map<std::string, std::unique_ptr<Framebuffer>>		framebuffers;
};

#endif // __FRAMEBUFFER_MANAGER_H__