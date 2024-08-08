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
	ComPtr<ID3D11VertexShader>				embedded_vertex_shader;
	ComPtr<ID3D11PixelShader>				embedded_pixel_shader;
};

#endif // __FULLSCREEN_QUAD_H__