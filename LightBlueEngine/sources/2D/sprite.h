#ifndef __SPRITE_H__
#define __SPRITE_H__

// <>�C���N���[�h
#include <d3d11.h>
#include <DirectXMath.h>
#include <string>
#include <wrl.h>

// using
using Microsoft::WRL::ComPtr;

// class >> [Sprite]
// 2D�̉摜�Ȃǂ�`�悷��N���X�B
class Sprite
{
private:
	// private:�萔
	static constexpr float COLOR_FACTOR = 255.0f;

	// private:�\����
	struct Vertex
	{
		DirectX::XMFLOAT3 position	= { 0.0f,0.0f,0.0f };
		DirectX::XMFLOAT4 color		= { 1.0f,1.0f,1.0f,1.0f };
		DirectX::XMFLOAT2 texcoord	= {0.0f,0.0f};
	};

public:
	// public:�R���X�g���N�^�E�f�X�g���N�^
	Sprite() {};
	Sprite(const wchar_t*, const char* = nullptr);
	Sprite(ID3D11ShaderResourceView*, const char* = nullptr);
	~Sprite() {};

	// public:�ʏ�֐�
	void Render(DirectX::XMFLOAT2, DirectX::XMFLOAT2,
		DirectX::XMFLOAT4 = { 1.0f,1.0f,1.0f,1.0f },
		DirectX::XMFLOAT3 = { 0.0f,0.0f,0.0f },
		DirectX::XMFLOAT2 = { 0.0f,0.0f }, DirectX::XMFLOAT2 = { 0.0f,0.0f });

	void RenderText(std::string, DirectX::XMFLOAT2, DirectX::XMFLOAT2, DirectX::XMFLOAT4 = { 1.0f,1.0f,1.0f,1.0f });

	// public:�Q�b�^�[�֐�
	const DirectX::XMFLOAT2 GetSpriteSizeWithScaling(DirectX::XMFLOAT2 scale)
	{
		return DirectX::XMFLOAT2(texture2d_desc.Width * scale.x, texture2d_desc.Height * scale.y);
	}

	ID3D11VertexShader*			GetVertexShader() { return vertex_shader.Get(); }
	ID3D11ShaderResourceView*	GetShaderResourceView() { return shader_resource_view.Get(); }

private:
	// private:�ϐ�
	bool								is_load_file					= true;
	D3D11_TEXTURE2D_DESC				texture2d_desc				= {};
	ComPtr<ID3D11Buffer>				vertex_buffer;
	ComPtr<ID3D11Buffer>				object_cbuffer;
	ComPtr<ID3D11InputLayout>			input_layout;
	ComPtr<ID3D11VertexShader>			vertex_shader;
	ComPtr<ID3D11PixelShader>			pixel_shader;
	ComPtr<ID3D11PixelShader>			replaced_pixel_shader;
	ComPtr<ID3D11ShaderResourceView>	shader_resource_view;
	ComPtr<ID3D11ShaderResourceView>	mask_shader_resource_view;
};

#endif // __SPRITE_H__