#ifndef __FRAMEBUFFER_H__
#define __FRAMEBUFFER_H__

// <>インクルード
#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl.h>
#include <cstdint>

// ""インクルード
// LightBlueEngine
#include "engine_utility.h"

// using
using Microsoft::WRL::ComPtr;

// enum class
enum class EnumFramebufferSRV
{
	SCENE,
	DEPTH
};

// class >> [Framebuffer]
// 画面を記録するクラス。ポストエフェクト用に加工可能。
class Framebuffer
{
public:
	// public:コンストラクタ・デストラクタ
	Framebuffer(uint32_t, uint32_t,bool = true);
	virtual ~Framebuffer()	= default;

	// public:通常関数
	void Clear(DirectX::XMFLOAT4, float = 1);
	void Activate(D3D11_VIEWPORT*);
	void Deactivate(D3D11_VIEWPORT*);

	// public:ゲッター関数
	ComPtr<ID3D11ShaderResourceView> 	GetSceneSRV()
		{ return mute_framebuffer ? nullptr : shader_resource_view[SCast(int, EnumFramebufferSRV::SCENE)]; }
	ComPtr<ID3D11ShaderResourceView> 	GetDepthSRV()	
		{ return mute_framebuffer ? nullptr : shader_resource_view[SCast(int, EnumFramebufferSRV::DEPTH)]; }

	// public:セッター関数
	void MuteFramebuffer(bool mute) { mute_framebuffer = mute; }

private:
	// private:変数
	bool								mute_framebuffer				= false;
	bool								use_dsv;
	UINT								viewport_count					= D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
	D3D11_VIEWPORT						viewport						= {};
	ComPtr<ID3D11RenderTargetView>		render_target_view;
	ComPtr<ID3D11RenderTargetView>		cached_render_target_view;
	ComPtr<ID3D11DepthStencilView>		depth_stencil_view;
	ComPtr<ID3D11DepthStencilView>		cached_depth_stencil_view;

	ComPtr<ID3D11ShaderResourceView>	shader_resource_view[2];		// 0:通常 1:深度
};

#endif // __FRAMEBUFFER_H__