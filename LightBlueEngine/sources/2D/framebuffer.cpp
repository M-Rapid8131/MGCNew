// ""�C���N���[�h
// LightBlueEngine
#include "graphics/graphics.h"
#include "engine_utility.h"
#include "framebuffer.h"

// �Q�[���\�[�X�t�@�C��
#include "misc.h"

//-------------------------------------------
// Framebuffer �����o�֐�
//-------------------------------------------

// �R���X�g���N�^
Framebuffer::Framebuffer(uint32_t width, uint32_t height, bool use_dsv) : use_dsv(use_dsv)
{
	Graphics*		graphics	= Graphics::GetInstance();
	ID3D11Device*	device		= graphics->GetDevice().Get();

	HRESULT hr = S_OK;
	
	// Texture2D�쐬
	ComPtr<ID3D11Texture2D> render_target_buffer;
	D3D11_TEXTURE2D_DESC		texture2d_desc = {};
	texture2d_desc.Width				= width;
	texture2d_desc.Height				= height;
	texture2d_desc.MipLevels			= 1;
	texture2d_desc.ArraySize			= 1;
	texture2d_desc.Format				= DXGI_FORMAT_R8G8B8A8_SNORM;
	texture2d_desc.SampleDesc.Count		= 1;
	texture2d_desc.SampleDesc.Quality	= 0;
	texture2d_desc.Usage				= D3D11_USAGE_DEFAULT;
	texture2d_desc.BindFlags			= D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	texture2d_desc.CPUAccessFlags		= 0;
	texture2d_desc.MiscFlags			= 0;
	hr = device->CreateTexture2D(&texture2d_desc, 0, render_target_buffer.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	// RTV�쐬
	D3D11_RENDER_TARGET_VIEW_DESC render_target_view_desc = {};
	render_target_view_desc.Format			= texture2d_desc.Format;
	render_target_view_desc.ViewDimension	= D3D11_RTV_DIMENSION_TEXTURE2D;
	hr = device->CreateRenderTargetView(render_target_buffer.Get(), &render_target_view_desc,
		render_target_view.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	// srv�쐬
	D3D11_SHADER_RESOURCE_VIEW_DESC shader_resource_view_desc = {};
	shader_resource_view_desc.Format					= texture2d_desc.Format;
	shader_resource_view_desc.ViewDimension				= D3D11_SRV_DIMENSION_TEXTURE2D;
	shader_resource_view_desc.Texture2D.MipLevels		= texture2d_desc.MipLevels;
	shader_resource_view_desc.Texture2D.MostDetailedMip = 0;
	hr = device->CreateShaderResourceView(render_target_buffer.Get(), &shader_resource_view_desc,
		shader_resource_view[0].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	// dsv���g�p����ꍇ
	if (use_dsv)
	{
		texture2d_desc.MiscFlags = 0;

		// �[�x�X�e���V���o�b�t�@�pTexture2D�쐬
		ComPtr<ID3D11Texture2D> depth_stencil_buffer = {};
		texture2d_desc.Format		= DXGI_FORMAT_R24G8_TYPELESS;
		texture2d_desc.BindFlags	= D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		hr = device->CreateTexture2D(&texture2d_desc, 0, depth_stencil_buffer.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		// dsv�쐬
		D3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc = {};
		depth_stencil_view_desc.Format			= DXGI_FORMAT_D24_UNORM_S8_UINT;
		depth_stencil_view_desc.ViewDimension	= D3D11_DSV_DIMENSION_TEXTURE2D;
		depth_stencil_view_desc.Flags			= 0;
		hr = device->CreateDepthStencilView(depth_stencil_buffer.Get(), &depth_stencil_view_desc,
			depth_stencil_view.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		// �[�x�X�e���V���o�b�t�@���g�p����srv�쐬
		shader_resource_view_desc.Format		= DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		shader_resource_view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		hr = device->CreateShaderResourceView(depth_stencil_buffer.Get(), &shader_resource_view_desc,
			shader_resource_view[1].GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	}

	// viewport�̏��
	viewport.Width		= SCast(float, width);
	viewport.Height		= SCast(float, height);
	viewport.MinDepth	= 0.0f;
	viewport.MaxDepth	= 1.0f;
	viewport.TopLeftX	= 0.0f;
	viewport.TopLeftY	= 0.0f;
}

// �t���[���o�b�t�@������
void Framebuffer::Clear(DirectX::XMFLOAT4 color, float depth) 
{	
	Graphics*				graphics		= Graphics::GetInstance();
	ID3D11DeviceContext*	device_context	= graphics->GetDeviceContext().Get();

	std::scoped_lock<std::mutex>	lock(graphics->GetMutex());

	float f_color[4] = { color.x, color.y, color.z, color.w };
	// rtv������
	device_context->ClearRenderTargetView(render_target_view.Get(), f_color);
	
	// dsv���g���ꍇ�A�����������
	if (use_dsv)
		device_context->ClearDepthStencilView(depth_stencil_view.Get(), D3D11_CLEAR_DEPTH, depth, 0);
}

// �t���[���o�b�t�@�N��
void Framebuffer::Activate(D3D11_VIEWPORT* cached_viewports_data)
{
	Graphics* graphics = Graphics::GetInstance();
	ID3D11DeviceContext* device_context = graphics->GetDeviceContext().Get();

	// �r���[�|�[�g�A�����_�[�^�[�Q�b�g���ꎞ�ۑ�
	viewport_count = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
	device_context->RSGetViewports(&viewport_count, cached_viewports_data);
	device_context->OMGetRenderTargets(1, cached_render_target_view.ReleaseAndGetAddressOf(),
		cached_depth_stencil_view.ReleaseAndGetAddressOf());

	// �t���[���o�b�t�@�ɍ����ւ�
	device_context->RSSetViewports(1, &viewport);
	device_context->OMSetRenderTargets(1, render_target_view.GetAddressOf(),
		depth_stencil_view.Get());
}

// �t���[���o�b�t�@�I��
void Framebuffer::Deactivate(D3D11_VIEWPORT* cached_viewports_data)
{
	Graphics*				graphics		= Graphics::GetInstance();
	ID3D11DeviceContext*	device_context	= graphics->GetDeviceContext().Get();

	// �r���[�|�[�g�A�����_�[�^�[�Q�b�g�����ɖ߂�
	if(cached_viewports_data)
		device_context->RSSetViewports(viewport_count, cached_viewports_data);
	device_context->OMSetRenderTargets(1, cached_render_target_view.GetAddressOf(),
		cached_depth_stencil_view.Get());
}