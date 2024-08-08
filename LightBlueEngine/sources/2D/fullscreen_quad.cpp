// ""�C���N���[�h
// LightBlueEngine
#include "graphics/graphics.h"
#include "graphics/texture.h"
#include "shader.h"
#include "fullscreen_quad.h"

// �Q�[���\�[�X�t�@�C��

//-------------------------------------------
// FullscreenQuad �����o�֐�
//-------------------------------------------

// �R���X�g���N�^
FullscreenQuad::FullscreenQuad()
{
	// �V�F�[�_�[�I�u�W�F�N�g�쐬
	Shader::CreateVSFromCso("shader/fullscreen_quad_vs.cso", embedded_vertex_shader.ReleaseAndGetAddressOf(),
		nullptr, nullptr, 0);
	Shader::CreatePSFromCso("shader/fullscreen_quad_ps.cso", embedded_pixel_shader.ReleaseAndGetAddressOf());
}

// �`��
void FullscreenQuad::Render(ID3D11ShaderResourceView** shader_resource_view, 
	uint32_t start_slot, uint32_t num_views, ID3D11VertexShader* replaced_vertex_shader, ID3D11PixelShader* replaced_pixel_shader)
{
	Graphics*				graphics		= Graphics::GetInstance();
	ID3D11DeviceContext*	device_context	= graphics->GetDeviceContext().Get();

	// ���_�o�b�t�@�A�v���~�e�B�u�g�|���W�[�A�C���v�b�g���C�A�E�g����U���Z�b�g
	device_context->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
	device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	device_context->IASetInputLayout(nullptr);

	// �s�N�Z���V�F�[�_�[�A���_�V�F�[�_�[���Z�b�g
	replaced_vertex_shader ? device_context->VSSetShader(replaced_vertex_shader, 0, 0) :
		device_context->VSSetShader(embedded_vertex_shader.Get(), 0, 0);
	replaced_pixel_shader ? device_context->PSSetShader(replaced_pixel_shader, 0, 0) :
		device_context->PSSetShader(embedded_pixel_shader.Get(), 0, 0);

	// �V�F�[�_�[���\�[�X���Z�b�g
	device_context->PSSetShaderResources(start_slot, num_views, shader_resource_view);

	device_context->Draw(4, 0);
}