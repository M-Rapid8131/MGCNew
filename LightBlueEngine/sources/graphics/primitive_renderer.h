#ifndef __PRIMITIVE_RENDERER_H__
#define __PRIMITIVE_RENDERER_H__

// <>�C���N���[�h
#include <vector>
#include <wrl.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include "graphics/camera.h"

// enum
enum class EnumBufferPrimitiveRenderer
{
	PRIMITIVE,
};

// using
using Microsoft::WRL::ComPtr;

// class >> [PrimitiveRenderer]
class PrimitiveRenderer
{
private:
	// private:�萔
	static constexpr		float	FADE_LENGTH		= 25.0f;
	static const			UINT	VERTEX_CAPACITY = 1024;

	// private:�\����
	// struct >> PrimitiveRenderer >> [Vertex]
	struct Vertex
	{
		DirectX::XMFLOAT3	position;
		DirectX::XMFLOAT4	color;
		DirectX::XMFLOAT2	texcoord;
	};

	// private:�萔�o�b�t�@�\����
	// struct >> PrimitiveRenderer >> [CbPrimitive]
	struct CbPrimitive
	{
		DirectX::XMFLOAT4X4	view_projection;
		DirectX::XMFLOAT4	color;
	};

public:
	// public:�R���X�g���N�^�E�f�X�g���N�^
	PrimitiveRenderer();
	~PrimitiveRenderer();

	// public:�ʏ�֐�
	void AddVertex(const DirectX::XMFLOAT3&, const DirectX::XMFLOAT3&,float);
	void Render(Camera*, D3D11_PRIMITIVE_TOPOLOGY);

	// public:�Z�b�^�[�֐�
	void SetRootPosition(DirectX::XMFLOAT3 p_root_position) { root_position = p_root_position; }
private:
	// private:�ϐ�
	DirectX::XMFLOAT3			root_position		= {0.0f,0.0f,0.0f};
	ComPtr<ID3D11VertexShader>	vertex_shader;
	ComPtr<ID3D11PixelShader>	pixel_shader;
	ComPtr<ID3D11InputLayout>	input_layout;
	ComPtr<ID3D11Buffer>		vertex_buffer;
	ComPtr<ID3D11Buffer>		primitive_cbuffer;
	CbPrimitive					primitive_constants = {};

	std::vector<Vertex>			vertices;
};

#endif // __PRIMITIVE_RENDERER_H__