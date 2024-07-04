#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__

// <>インクルード
#include <d3d11.h>
#include <d2d1.h>
#include <dwrite.h>
#include <DirectXMath.h>
#include <windows.h>
#include <wrl.h>
#include <tchar.h>
#include <sstream>
#include <mutex>
#include <memory>
#include <vector>

// ""インクルード
// LightBlueEngine
#include "engine_utility.h"
#include "singleton.h"

// ゲームソースファイル
#include "misc.h"

// using
using Microsoft::WRL::ComPtr;

// enum
enum class EnumSamplerState
{
	POINT,
	LINEAR,
	ANISOTROPIC,
	BORDER,

	SAMPLER_STATE_NUM
};

enum class EnumDepthState
{
	ZT_ON_ZW_ON,
	ZT_ON_ZW_OFF,
	ZT_OFF_ZW_ON,
	ZT_OFF_ZW_OFF,

	DEPTH_STATE_NUM
};

enum class EnumBlendState
{
	ALPHA,
	NONE,
	SUBTRACT,
	ADD,

	BLEND_STATE_NUM
};

enum class EnumRasterizerState
{
	SOLID,
	WIREFRAME,
	CULL_NONE,
	WIREFRAME_CULL_NONE,
	PARTICLE,

	RASTERIZER_STATE_NUM
};

enum class EnumPixelShader
{
	LUMINANCE_EXTRACTION,
	COLOR_EXTRACTION,
	BLUR,
	EXTRACT_EMISSIVE,
	SCREEN_EFFECT,

	PIXEL_SHADER_NUM
};

enum class EnumComputeShader
{
	BLOCK,

	COMPUTE_SHADER_NUM
};

enum class EnumTexture
{
	MAIN_TEXTURE,
	MATERIAL_TEXTURE_1,
	MATERIAL_TEXTURE_2,
	MATERIAL_TEXTURE_3,
	MATERIAL_TEXTURE_4,
	MATERIAL_TEXTURE_5,
	RAMP_TEXTURE,
	SHADOWMAP_TEXTURE,
	MASK_TEXTURE,
};

enum class EnumCommonConstantBuffer
{
	BLOOM = 8,
	SCENE,
	LIGHT,
	SKY_COLOR,
	COLOR_FILTER,
	SHADOWMAP,
};

// struct >> [Graphics2D]
struct Graphics2D
{
	// 変数
	ComPtr<ID2D1Factory>			d2d1_factory;
	ComPtr<IDWriteFactory>			dwrite_factory;
	ComPtr<ID2D1RenderTarget>		d2d1_render_target;
	ComPtr<IDXGISurface>			surface;
};

// class >> [Graphics]
// DX11での描画関係のクラスをまとめて管理しているクラス。
class Graphics : public Singleton<Graphics>
{
private:
	// private:定数
	HWND hwnd;

public:
	// public:定数
	static const		LONG	BASE_SCREEN_WIDTH	= 1920;
	static const		LONG	BASE_SCREEN_HEIGHT	= 1080;
	static constexpr	LPWSTR	W_APPLICATION_NAME	= CCast(WCHAR*, L"MAGIC COLOR STONE");

	// public:コンストラクタ・デストラクタ
	Graphics() {};
	~Graphics() = default;

	//public:コピーコンストラクタ・ムーブコンストラクタ
	Graphics(const Graphics&)					= delete;
	Graphics& operator = (const Graphics&)		= delete;
	Graphics(Graphics&&) noexcept				= delete;
	Graphics& operator= (Graphics&&) noexcept	= delete;

	// public:通常関数
	void Initialize(HWND, bool);
	void Update();
	void StylizeWindow(BOOL);
	void SwapChainPresent(UINT interval, UINT flags)		
		{ swap_chain->Present(interval, flags); }

	// public:ゲッター関数
	LONG		GetScreenWidth()	const				{ return screen_width; }
	LONG		GetScreenHeight()	const				{ return screen_height; }
	
	float						GetScreenWidthMag() const				
	{
		return SCast(float, screen_width) / SCast(float, BASE_SCREEN_WIDTH);
	}

	float						GetScreenHeightMag() const					
	{
		return SCast(float, screen_height) / SCast(float, BASE_SCREEN_HEIGHT);
	}

	std::mutex&					GetMutex()								{ return graphics_mtx; }
	ComPtr<ID3D11Device>		GetDevice()								{ return device; }
	ComPtr<ID3D11DeviceContext> GetDeviceContext()						{ return device_context; }
	ComPtr<ID3D11PixelShader>	GetPixelShader(EnumPixelShader id)		{ return pixel_shaders[SCast(size_t, id)]; }
	ComPtr<ID3D11ComputeShader>	GetComputeShader(EnumComputeShader id)	{ return compute_shaders[SCast(size_t, id)]; }
	Graphics2D&					GetGraphics2D()							{ return graphics_2d; }

	BOOL						GetFullscreenMode()						{ return windowed; }
	void						GetFullscreenState(BOOL* get_fullscreen, IDXGIOutput* target)	{ swap_chain->GetFullscreenState(get_fullscreen,&target); }
		// tips >> 戻り値がvoidだが、引数fullscreenに結果が格納されるのでゲッター関数扱いとしている

	// public:セッター関数
	void SetDepthStencilState(EnumDepthState id) 
	{
		device_context->OMSetDepthStencilState(depth_stencil_states[SCast(size_t, id)].Get(), 1);
	}

	void SetBlendState(EnumBlendState id, const FLOAT* blend_factor, UINT sample_mask)
	{
		device_context->OMSetBlendState(blend_states[SCast(size_t, id)].Get(), blend_factor, sample_mask);
	}

	void SetRasterizerState(EnumRasterizerState id)
	{
		device_context->RSSetState(rasterizer_states[SCast(size_t, id)].Get());
	}

	void SetFullscreenState(BOOL new_fullscreen, IDXGIOutput* target)	{ swap_chain->SetFullscreenState(new_fullscreen,target); }

private:
	// private:変数
	int												refresh_rate		= 60;
	LONG											screen_width		= BASE_SCREEN_WIDTH;
	LONG											screen_height		= BASE_SCREEN_HEIGHT;
	BOOL											windowed			= TRUE;
	RECT											windowed_rect;
	DWORD											windowed_style;
	std::mutex										graphics_mtx;
	ComPtr<ID3D11Device>							device;
	ComPtr<ID3D11DeviceContext>						device_context;
	ComPtr<IDXGISwapChain>							swap_chain;
	ComPtr<ID3D11RenderTargetView>					render_target_view;
	ComPtr<ID3D11DepthStencilView>					depth_stencil_view;
	Graphics2D										graphics_2d			= {};

	ComPtr<ID3D11SamplerState>						sampler_states[SCast(size_t, EnumSamplerState::SAMPLER_STATE_NUM)];
	ComPtr<ID3D11DepthStencilState>					depth_stencil_states[SCast(size_t, EnumDepthState::DEPTH_STATE_NUM)];
	ComPtr<ID3D11BlendState>						blend_states[SCast(size_t, EnumBlendState::BLEND_STATE_NUM)];
	ComPtr<ID3D11RasterizerState>					rasterizer_states[SCast(size_t, EnumRasterizerState::RASTERIZER_STATE_NUM)];
	ComPtr<ID3D11PixelShader>						pixel_shaders[SCast(size_t, EnumPixelShader::PIXEL_SHADER_NUM)];
	ComPtr<ID3D11ComputeShader>						compute_shaders[SCast(size_t, EnumComputeShader::COMPUTE_SHADER_NUM)];
};

#endif // __GRAPHICS_H__