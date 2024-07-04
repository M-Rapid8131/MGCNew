#ifndef __BLOOM_EFFECT__
#define __BLOOM_EFFECT__

// <>インクルード
#include <memory>
#include <d3d11.h>
#include <wrl.h>

// ""インクルード
#include "2D/framebuffer.h"
#include "2D/fullscreen_quad.h"

#include "singleton.h"

// using
using Microsoft::WRL::ComPtr;

// class >> [BloomEffect]
class BloomEffect
{
private:
	static const size_t MAX_DOWNSAMPLED_COUNT = 12;

public:
	// public:定数バッファ構造体
	struct CbBloom
	{
		int		downsampled_count			= 6;
		int		blur_buffer_mag				= 1;
		int		cbbloom_ipad[2];
		float	bloom_extraction_threshold	= 0.2f;
		float	bloom_intensity				= 1.0f;
		float	smooth_rate					= 2.5f;
		float	cbbloom_fpad;
	};

	// public:コンストラクタ
	BloomEffect(const CbBloom, UINT, UINT);
	~BloomEffect() {};

	// public:通常関数
	void DebugGUI(std::string);
	void MakeEffect(ID3D11ShaderResourceView*);

	// public:ゲッター関数
	ID3D11ShaderResourceView* GetSRV() const { return using_switch ? glow_extraction->GetSceneSRV().Get() : nullptr; };

private:
	bool							using_switch = true;
	std::unique_ptr<FullscreenQuad> main_screen;
	std::unique_ptr<Framebuffer>	glow_extraction;
	ComPtr<ID3D11PixelShader>		glow_extraction_ps;
	ComPtr<ID3D11PixelShader>		downsampling_ps;
	ComPtr<ID3D11PixelShader>		horizontal_ps;
	ComPtr<ID3D11PixelShader>		vertical_ps;
	ComPtr<ID3D11PixelShader>		upsampling_ps;
	ComPtr<ID3D11Buffer>			bloom_cbuffer;
	CbBloom							bloom_constants;
	
	std::unique_ptr<Framebuffer>	gaussian_blur[MAX_DOWNSAMPLED_COUNT][2];
	std::vector<D3D11_VIEWPORT>		cached_viewports;
};

#endif // __BLOOM_EFFECT__