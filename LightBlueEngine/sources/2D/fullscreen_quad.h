#ifndef __FULLSCREEN_QUAD_H__
#define __FULLSCREEN_QUAD_H__

// <>�C���N���[�h
#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl.h>
#include <cstdint>

// using
using Microsoft::WRL::ComPtr;

// class >> [FullscreenQuad]
// ��ʑS�̂��L�^����N���X�B
class FullscreenQuad
{
public:
	// public:�R���X�g���N�^�E�f�X�g���N�^
	FullscreenQuad();
	~FullscreenQuad() {};

	// public:�ʏ�֐�
	void Render(ID3D11ShaderResourceView**, uint32_t, uint32_t, ID3D11VertexShader* = nullptr, ID3D11PixelShader* = nullptr);

private:
	// �ϐ�
	ComPtr<ID3D11VertexShader>				embedded_vertex_shader;
	ComPtr<ID3D11PixelShader>				embedded_pixel_shader;
};

#endif // __FULLSCREEN_QUAD_H__