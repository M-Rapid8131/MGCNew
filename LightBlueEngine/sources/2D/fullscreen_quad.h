#ifndef __FULLSCREEN_QUAD_H__
#define __FULLSCREEN_QUAD_H__

// <>インクルード
#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl.h>
#include <cstdint>

// using
using Microsoft::WRL::ComPtr;

// class >> [FullscreenQuad]
// 画面全体を記録するクラス。
class FullscreenQuad
{
public:
	// public:コンストラクタ・デストラクタ
	FullscreenQuad();
	~FullscreenQuad() {};

	// public:通常関数
	void Render(ID3D11ShaderResourceView**, uint32_t, uint32_t, ID3D11VertexShader* = nullptr, ID3D11PixelShader* = nullptr);

private:
	// 変数
	D3D11_TEXTURE2D_DESC					mask_texture_desc			= {};
	D3D11_TEXTURE2D_DESC					transition_texture_desc		= {};
	ComPtr<ID3D11Buffer>					dissolve_cbuffer;
	ComPtr<ID3D11VertexShader>				embedded_vertex_shader;
	ComPtr<ID3D11PixelShader>				embedded_pixel_shader;
	ComPtr<ID3D11ShaderResourceView>		mask_texture;
	ComPtr<ID3D11ShaderResourceView>		transition_texture;
};

#endif // __FULLSCREEN_QUAD_H__