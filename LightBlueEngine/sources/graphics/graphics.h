#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__

// <>インクルード
#include <d3d11.h>
#include <d2d1.h>
#include <dwrite.h>
#include <dxgi1_6.h>
#include <windows.h>
#include <wrl.h>
#include <mutex>
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
	BOARD_START,
	BOARD_GAMEOVER,

	COMPUTE_SHADER_NUM
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

// 定数
//static const UINT

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
public:
	// public:定数バッファ構造体
	// struct >> Graphics >> [CbTransition]
	struct CbTransition
	{
		int		using_transition_texture		= false;
		int		using_transition_back_texture	= false;
		int		reverse							= false;
		
		int		cbtransition_ipad;

		float	transition_prog				= 0.0f;
		float	transition_smooth			= 0.1f;
		
		float	cbtransition_fpad[2];
	};

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
	void DebugGUI();
	void StylizeWindow(BOOL);
	void CreateSwapChain(IDXGIFactory6*);
	void AcquireHighPerformanceAdapter(IDXGIFactory6*, IDXGIAdapter3**);
	void OnSizeChanged(LONG, LONG);

	void SwapChainPresent(UINT interval, UINT flags)		
		{ swap_chain1->Present(interval, flags); }

	// public:ゲッター関数
	LONG		GetScreenWidth()	const				{ return screen_width; }
	LONG		GetScreenHeight()	const				{ return screen_height; }
	
	float		GetScreenWidthMag() const				
	{
		return SCast(float, screen_width) / SCast(float, BASE_SCREEN_WIDTH);
	}

	float		GetScreenHeightMag() const					
	{
		return SCast(float, screen_height) / SCast(float, BASE_SCREEN_HEIGHT);
	}

	std::mutex&					GetMutex()								{ return graphics_mtx; }
	HWND						GetHWND() const							{ return hwnd; }
	ComPtr<ID3D11Device>		GetDevice()								{ return device; }
	ComPtr<ID3D11DeviceContext> GetDeviceContext()						{ return device_context; }
	ComPtr<ID3D11PixelShader>	GetPixelShader(EnumPixelShader id)		{ return pixel_shaders[SCast(size_t, id)]; }
	ComPtr<ID3D11ComputeShader>	GetComputeShader(EnumComputeShader id)	{ return compute_shaders[SCast(size_t, id)]; }
	Graphics2D&					GetGraphics2D()							{ return graphics_2d; }
	CbTransition&				GetCbTransition()						{ return transition_constants; }

	BOOL						GetFullscreenMode()						{ return windowed; }
	void						GetFullscreenState(BOOL* get_fullscreen, IDXGIOutput* target)	{ swap_chain1->GetFullscreenState(get_fullscreen,&target); }
		// tips >> 戻り値がvoidだが、引数fullscreenに結果が格納されるのでゲッター関数扱いとしている

	// public:セッター関数
		// public:セッター関数
	void LoadTransitionTexture(const wchar_t* w_filename);
	void LoadTransitionBackTexture(const wchar_t* w_filename);

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

	void SetFullscreenState(BOOL new_fullscreen, IDXGIOutput* target)	{ swap_chain1->SetFullscreenState(new_fullscreen,target); }

private:
	// private:変数
	int									refresh_rate		= 60;
	LONG								screen_width		= BASE_SCREEN_WIDTH;
	LONG								screen_height		= BASE_SCREEN_HEIGHT;
	BOOL								windowed			= TRUE;
	BOOL								tearing_supported	= FALSE;
	RECT								windowed_rect;
	DWORD								windowed_style;
	HWND								hwnd;
	std::string							gpu_information;
	std::mutex							graphics_mtx;
	D3D11_TEXTURE2D_DESC				transition_texture_desc = {};
	D3D11_TEXTURE2D_DESC				transition_back_texture_desc = {};
	ComPtr<IDXGIAdapter3>				adapter3;
	ComPtr<ID3D11Device>				device;
	ComPtr<ID3D11DeviceContext>			device_context;
	ComPtr<IDXGISwapChain1>				swap_chain1;
	ComPtr<ID3D11RenderTargetView>		render_target_view;
	ComPtr<ID3D11DepthStencilView>		depth_stencil_view;
	ComPtr<ID3D11ShaderResourceView>	transition_texture;
	ComPtr<ID3D11ShaderResourceView>	transition_back_texture;
	ComPtr<ID3D11Buffer>				transition_cbuffer;
	Graphics2D							graphics_2d			= {};
	CbTransition						transition_constants;

	ComPtr<ID3D11SamplerState>			sampler_states[SCast(size_t, EnumSamplerState::SAMPLER_STATE_NUM)];
	ComPtr<ID3D11DepthStencilState>		depth_stencil_states[SCast(size_t, EnumDepthState::DEPTH_STATE_NUM)];
	ComPtr<ID3D11BlendState>			blend_states[SCast(size_t, EnumBlendState::BLEND_STATE_NUM)];
	ComPtr<ID3D11RasterizerState>		rasterizer_states[SCast(size_t, EnumRasterizerState::RASTERIZER_STATE_NUM)];
	ComPtr<ID3D11PixelShader>			pixel_shaders[SCast(size_t, EnumPixelShader::PIXEL_SHADER_NUM)];
	ComPtr<ID3D11ComputeShader>			compute_shaders[SCast(size_t, EnumComputeShader::COMPUTE_SHADER_NUM)];
};

#endif // __GRAPHICS_H__