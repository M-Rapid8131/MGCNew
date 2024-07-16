// ""�C���N���[�h
// LightBlueEngine
#include "shader.h"
#include "camera.h"
#include "graphics.h"
#include "primitive_renderer.h"

// �Q�[���\�[�X�t�@�C��
#include "misc.h"

//----------------------------------------------
// PrimitiveRenderer �����o�֐�					
//----------------------------------------------

// �R���X�g���N�^
PrimitiveRenderer::PrimitiveRenderer()
{
	Graphics*				graphics		= Graphics::GetInstance();
	ID3D11Device*			device			= graphics->GetDevice().Get();

	HRESULT			hr = S_OK;

	D3D11_INPUT_ELEMENT_DESC input_element_desc[] =
	{
		{"POSITION",	0,	DXGI_FORMAT_R32G32B32_FLOAT,	0,	D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA,		0},
		{"COLOR",		0,	DXGI_FORMAT_R32G32B32A32_FLOAT,	0,	D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA,		0},
		{"TEXCOORD",	0,	DXGI_FORMAT_R32G32_FLOAT,		0,	D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA,		0},
	};

	// �V�F�[�_�[
	Shader::CreateVSFromCso("shader/primitive_renderer_vs.cso", vertex_shader.GetAddressOf(), 
		input_layout.GetAddressOf(), input_element_desc, _countof(input_element_desc));
	
	Shader::CreatePSFromCso("shader/primitive_renderer_ps.cso", pixel_shader.GetAddressOf());

	// �萔�o�b�t�@�E���_�o�b�t�@
	D3D11_BUFFER_DESC buffer_desc = {};

	buffer_desc.ByteWidth = sizeof(CbPrimitive);
	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	buffer_desc.CPUAccessFlags = 0;
	buffer_desc.MiscFlags = 0;
	buffer_desc.StructureByteStride = 0;
	hr = device->CreateBuffer(&buffer_desc, nullptr, primitive_cbuffer.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	buffer_desc.ByteWidth = sizeof(Vertex) * VERTEX_CAPACITY;
	buffer_desc.StructureByteStride = 0;
	buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
	buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	buffer_desc.MiscFlags = 0;
	hr = device->CreateBuffer(&buffer_desc, NULL, vertex_buffer.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
}

// �f�X�g���N�^
PrimitiveRenderer::~PrimitiveRenderer()
{
	vertex_shader.Reset();
	pixel_shader.Reset();
	input_layout.Reset();
	vertex_buffer.Reset();
	primitive_cbuffer.Reset();
}

// ���_�ǉ�
void PrimitiveRenderer::AddVertex(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& color,float alpha)
{
	Vertex& v		= vertices.emplace_back();
	v.position		= position;
	v.color.x		= color.x;
	v.color.y		= color.y;
	v.color.z		= color.z;
	v.color.w		= alpha;

	DirectX::XMVECTOR	v_dist	= DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&root_position), DirectX::XMLoadFloat3(&position));
	float				dist	= DirectX::XMVectorGetX(DirectX::XMVector3Length(v_dist));

	v.texcoord = { 0,dist / FADE_LENGTH };
}

// �`�揈��
void PrimitiveRenderer::Render(GameCamera* camera, D3D11_PRIMITIVE_TOPOLOGY primitive_topology)
{
	Graphics*				graphics		= Graphics::GetInstance();
	ID3D11DeviceContext*	device_context	= graphics->GetDeviceContext().Get();

	HRESULT	hr = S_OK;

	// �V�F�[�_�[�ݒ�
	device_context->VSSetShader(vertex_shader.Get(), nullptr, 0);
	device_context->PSSetShader(pixel_shader.Get(), nullptr, 0);
	device_context->IASetInputLayout(input_layout.Get());

	// �萔�o�b�t�@�ݒ�
	device_context->VSSetConstantBuffers(SCast(UINT, EnumBufferPrimitiveRenderer::PRIMITIVE), 1, primitive_cbuffer.GetAddressOf());

	// �r���[�v���W�F�N�V�����s��쐬
	DirectX::XMMATRIX m_view		= DirectX::XMLoadFloat4x4(&camera->GetView());
	DirectX::XMMATRIX m_projection	= DirectX::XMLoadFloat4x4(&camera->GetProjection());
	
	// �萔�o�b�t�@�X�V
	DirectX::XMStoreFloat4x4(&primitive_constants.view_projection, m_view * m_projection);
	device_context->UpdateSubresource(primitive_cbuffer.Get(), 0, 0, &primitive_constants, 0, 0);

	// ���_�o�b�t�@�ݒ�
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	device_context->IASetPrimitiveTopology(primitive_topology);
	device_context->IASetVertexBuffers(0, 1, vertex_buffer.GetAddressOf(), &stride, &offset);

	// �`��
	UINT total_vertex_count = SCast(UINT, vertices.size());
	UINT start				= 0;
	UINT count				= (total_vertex_count < VERTEX_CAPACITY) ? total_vertex_count : VERTEX_CAPACITY;

	while (start < total_vertex_count)
	{
		D3D11_MAPPED_SUBRESOURCE	mapped_subresource;
		
		hr = device_context->Map(vertex_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource);
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		memcpy(mapped_subresource.pData, &vertices[start], sizeof(Vertex) * count);
		device_context->Unmap(vertex_buffer.Get(), 0);
		device_context->Draw(count, 0);

		start += count;
		if ((start + count) > total_vertex_count)
		{
			count = total_vertex_count - start;
		}
	}

	vertices.clear();
}