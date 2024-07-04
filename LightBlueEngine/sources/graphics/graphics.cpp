#include "graphics.h"
// <>�C���N���[�h
#include <imgui.h>

// ""�C���N���[�h
// LightBlueEngine
#include "shader.h"
#include "texture.h"

// define�萔
#define INTEL_VENDOR_ID		0x8086
#define NVIDIA_VENDOR_ID	0x10DE

//--------------------------------------
//	Graphics �����o�֐�
//-------------------------------------

// �R���X�g���N�^
void Graphics::Initialize(HWND set_hwnd, bool windowed)
{
	hwnd = set_hwnd;

	if (!windowed)
	{
		StylizeWindow(TRUE);
	}

	RECT rc;
	GetClientRect(hwnd, &rc);
	LONG screen_w = rc.right - rc.left;
	LONG screen_h = rc.bottom - rc.top;

	screen_width	= screen_w;
	screen_height	= screen_h;

	HRESULT hr = S_OK;

	// �A�_�v�^����
	IDXGIFactory* factory;
	CreateDXGIFactory(IID_PPV_ARGS(&factory));
	IDXGIAdapter* adapter;
	for (UINT adapter_index = 0; S_OK == factory->EnumAdapters(adapter_index, &adapter); ++adapter_index)
	{
		DXGI_ADAPTER_DESC adapter_desc;
		adapter->GetDesc(&adapter_desc);
		if (adapter_desc.VendorId == NVIDIA_VENDOR_ID)	break;
		adapter->Release();
	}
	factory->Release();

	UINT create_device_flags = 0;

#ifdef _DEBUG
	create_device_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL feature_level=
		D3D_FEATURE_LEVEL_11_0;
	// �f�o�C�X�ƃX���b�v�`�F�[���쐬
	DXGI_SWAP_CHAIN_DESC swap_chain_desc = {};
	swap_chain_desc.BufferCount							= 1;
	swap_chain_desc.BufferDesc.Width					= screen_w;
	swap_chain_desc.BufferDesc.Height					= screen_h;
	swap_chain_desc.BufferDesc.Format					= DXGI_FORMAT_R8G8B8A8_UNORM;
	swap_chain_desc.BufferDesc.RefreshRate.Numerator	= SCast(UINT, refresh_rate);
	swap_chain_desc.BufferDesc.RefreshRate.Denominator	= 1;
	swap_chain_desc.BufferUsage							= DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swap_chain_desc.OutputWindow						= hwnd;
	swap_chain_desc.SampleDesc.Count					= 1;
	swap_chain_desc.SampleDesc.Quality					= 0;
	swap_chain_desc.Windowed							= windowed;
	swap_chain_desc.SwapEffect							= DXGI_SWAP_EFFECT_DISCARD;
	hr = D3D11CreateDeviceAndSwapChain(adapter, D3D_DRIVER_TYPE_UNKNOWN, NULL, create_device_flags,
		&feature_level, 1, D3D11_SDK_VERSION, &swap_chain_desc,
		swap_chain.ReleaseAndGetAddressOf(), device.ReleaseAndGetAddressOf(), NULL, device_context.ReleaseAndGetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	// �����_�[�^�[�Q�b�g�̍쐬
	ComPtr<ID3D11Texture2D> back_buffer = {};
	hr = swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), RCast(LPVOID*, back_buffer.GetAddressOf()));
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	hr = device->CreateRenderTargetView(back_buffer.Get(), NULL, render_target_view.ReleaseAndGetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	D3D11_SAMPLER_DESC sampler_desc = {};
	sampler_desc.Filter				= D3D11_FILTER_MIN_MAG_MIP_POINT;
	sampler_desc.AddressU			= D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressV			= D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressW			= D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.MipLODBias			= 0;
	sampler_desc.MaxAnisotropy		= 16;
	sampler_desc.ComparisonFunc		= D3D11_COMPARISON_ALWAYS;
	sampler_desc.BorderColor[0]		= 0;
	sampler_desc.BorderColor[1]		= 0;
	sampler_desc.BorderColor[2]		= 0;
	sampler_desc.BorderColor[3]		= 0;
	sampler_desc.MinLOD				= 0;
	sampler_desc.MaxLOD				= D3D11_FLOAT32_MAX;
	hr = device->CreateSamplerState(
		&sampler_desc, sampler_states[SCast(size_t, EnumSamplerState::POINT)].ReleaseAndGetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	hr = device->CreateSamplerState(
		&sampler_desc, sampler_states[SCast(size_t, EnumSamplerState::LINEAR)].ReleaseAndGetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	sampler_desc.Filter = D3D11_FILTER_ANISOTROPIC;
	hr = device->CreateSamplerState(
		&sampler_desc, sampler_states[SCast(size_t, EnumSamplerState::ANISOTROPIC)].ReleaseAndGetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	hr = device->CreateSamplerState(
		&sampler_desc, sampler_states[SCast(size_t, EnumSamplerState::BORDER)].ReleaseAndGetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	// �r���[�|�[�g�̐ݒ�
	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX		= 0;
	viewport.TopLeftY		= 0;
	viewport.Width			= SCast(float, screen_width);
	viewport.Height			= SCast(float, screen_height);
	viewport.MinDepth		= 0.0f;
	viewport.MaxDepth		= 1.0f;
	device_context->RSSetViewports(1, &viewport);

	if(create_device_flags & D3D11_CREATE_DEVICE_BGRA_SUPPORT)
	{
		// Direct2D�ADirectWrite�̏�����
		hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, graphics_2d.d2d1_factory.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		hr = swap_chain->GetBuffer(0, __uuidof(IDXGISurface), 
			RCast(void**, graphics_2d.surface.GetAddressOf()));
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		// dpi�擾
		UINT dpi;
		dpi = GetDpiForWindow(hwnd);

		// ��ʂ̔䗦����X������Y������dpi���擾����
		FLOAT aspect_x = SCast(FLOAT, screen_width)		/ SCast(FLOAT, screen_width + screen_height);
		FLOAT aspect_y = SCast(FLOAT, screen_height)	/ SCast(FLOAT, screen_width + screen_height);
		FLOAT dpi_x = 0.0f, dpi_y = 0.0f;
		dpi_x = SCast(FLOAT, (dpi) * aspect_x);
		dpi_y = SCast(FLOAT, (dpi) * aspect_y);

		// �v���p�e�B�ݒ�
		D2D1_RENDER_TARGET_PROPERTIES rt_props
			= D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT,
				D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED), dpi_x, dpi_y);

		// surface����render_target���쐬
		hr = graphics_2d.d2d1_factory->CreateDxgiSurfaceRenderTarget(graphics_2d.surface.Get(), 
			&rt_props, graphics_2d.d2d1_render_target.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		// dwrite��factory���쐬
		hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
			__uuidof(IDWriteFactory), RCast(IUnknown**,graphics_2d.dwrite_factory.GetAddressOf()));
	}

	// �[�x�X�e���V���X�e�[�g�I�u�W�F�N�g�̍쐬
	//�[�x�e�X�g�F�I���@�[�x���C�g�F�I��
	D3D11_DEPTH_STENCIL_DESC depth_stencil_desc = {};
	depth_stencil_desc.DepthEnable		= TRUE;
	depth_stencil_desc.DepthWriteMask	= D3D11_DEPTH_WRITE_MASK_ALL;
	depth_stencil_desc.DepthFunc			= D3D11_COMPARISON_LESS_EQUAL;
	hr = device->CreateDepthStencilState(
		&depth_stencil_desc,
		depth_stencil_states[SCast(size_t, EnumDepthState::ZT_ON_ZW_ON)].ReleaseAndGetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	//�[�x�e�X�g�F�I���@�[�x���C�g�F�I�t
	depth_stencil_desc.DepthEnable		= TRUE;
	depth_stencil_desc.DepthWriteMask	= D3D11_DEPTH_WRITE_MASK_ZERO;
	depth_stencil_desc.DepthFunc			= D3D11_COMPARISON_LESS_EQUAL;
	hr = device->CreateDepthStencilState(
		&depth_stencil_desc,
		depth_stencil_states[SCast(size_t, EnumDepthState::ZT_ON_ZW_OFF)].ReleaseAndGetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	//�[�x�e�X�g�F�I�t�@�[�x���C�g�F�I��
	depth_stencil_desc.DepthEnable		= FALSE;
	depth_stencil_desc.DepthWriteMask	= D3D11_DEPTH_WRITE_MASK_ALL;
	depth_stencil_desc.DepthFunc			= D3D11_COMPARISON_LESS_EQUAL;
	hr = device->CreateDepthStencilState(
		&depth_stencil_desc,
		depth_stencil_states[SCast(size_t, EnumDepthState::ZT_OFF_ZW_ON)].ReleaseAndGetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	//�[�x�e�X�g�F�I�t�@�[�x���C�g�F�I�t
	depth_stencil_desc.DepthEnable		= FALSE;
	depth_stencil_desc.DepthWriteMask	= D3D11_DEPTH_WRITE_MASK_ZERO;
	depth_stencil_desc.DepthFunc			= D3D11_COMPARISON_LESS_EQUAL;
	hr = device->CreateDepthStencilState(
		&depth_stencil_desc,
		depth_stencil_states[SCast(size_t, EnumDepthState::ZT_OFF_ZW_OFF)].ReleaseAndGetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	D3D11_BLEND_DESC blend_desc = {};
	blend_desc.AlphaToCoverageEnable		= FALSE;
	blend_desc.IndependentBlendEnable	= FALSE;
	// ����
	blend_desc.RenderTarget[0].BlendEnable			= TRUE;
	blend_desc.RenderTarget[0].SrcBlend				= D3D11_BLEND_SRC_ALPHA;
	blend_desc.RenderTarget[0].DestBlend				= D3D11_BLEND_INV_SRC_ALPHA;
	blend_desc.RenderTarget[0].BlendOp				= D3D11_BLEND_OP_ADD;
	blend_desc.RenderTarget[0].SrcBlendAlpha			= D3D11_BLEND_ONE;
	blend_desc.RenderTarget[0].DestBlendAlpha		= D3D11_BLEND_ONE;
	blend_desc.RenderTarget[0].BlendOpAlpha			= D3D11_BLEND_OP_ADD;
	blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	hr = device->CreateBlendState(
		&blend_desc,
		blend_states[SCast(size_t, EnumBlendState::ALPHA)].ReleaseAndGetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	// ����
	blend_desc.RenderTarget[0].BlendEnable				= FALSE;
	blend_desc.RenderTarget[0].SrcBlend					= D3D11_BLEND_ONE;
	blend_desc.RenderTarget[0].DestBlend				= D3D11_BLEND_ZERO;
	blend_desc.RenderTarget[0].BlendOp					= D3D11_BLEND_OP_ADD;
	blend_desc.RenderTarget[0].SrcBlendAlpha				= D3D11_BLEND_ONE;
	blend_desc.RenderTarget[0].DestBlendAlpha			= D3D11_BLEND_ZERO;
	blend_desc.RenderTarget[0].BlendOpAlpha				= D3D11_BLEND_OP_ADD;
	blend_desc.RenderTarget[0].RenderTargetWriteMask		= D3D11_COLOR_WRITE_ENABLE_ALL;

	hr = device->CreateBlendState(
		&blend_desc,
		blend_states[SCast(size_t, EnumBlendState::NONE)].ReleaseAndGetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	// ���Z
	blend_desc.RenderTarget[0].BlendEnable				= TRUE;
	blend_desc.RenderTarget[0].SrcBlend					= D3D11_BLEND_SRC_COLOR;
	blend_desc.RenderTarget[0].DestBlend					= D3D11_BLEND_INV_SRC_COLOR;
	blend_desc.RenderTarget[0].BlendOp					= D3D11_BLEND_OP_SUBTRACT;
	blend_desc.RenderTarget[0].SrcBlendAlpha				= D3D11_BLEND_ONE;
	blend_desc.RenderTarget[0].DestBlendAlpha			= D3D11_BLEND_ZERO;
	blend_desc.RenderTarget[0].BlendOpAlpha				= D3D11_BLEND_OP_SUBTRACT;
	blend_desc.RenderTarget[0].RenderTargetWriteMask		= D3D11_COLOR_WRITE_ENABLE_ALL;

	hr = device->CreateBlendState(
		&blend_desc,
		blend_states[SCast(size_t, EnumBlendState::SUBTRACT)].ReleaseAndGetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	// ���Z
	blend_desc.RenderTarget[0].BlendEnable				= TRUE;
	blend_desc.RenderTarget[0].SrcBlend					= D3D11_BLEND_SRC_COLOR;
	blend_desc.RenderTarget[0].DestBlend					= D3D11_BLEND_INV_SRC_COLOR;
	blend_desc.RenderTarget[0].BlendOp					= D3D11_BLEND_OP_ADD;
	blend_desc.RenderTarget[0].SrcBlendAlpha				= D3D11_BLEND_ONE;
	blend_desc.RenderTarget[0].DestBlendAlpha			= D3D11_BLEND_ZERO;
	blend_desc.RenderTarget[0].BlendOpAlpha				= D3D11_BLEND_OP_ADD;
	blend_desc.RenderTarget[0].RenderTargetWriteMask		= D3D11_COLOR_WRITE_ENABLE_ALL;

	hr = device->CreateBlendState(
		&blend_desc,
		blend_states[SCast(size_t, EnumBlendState::ADD)].ReleaseAndGetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	// ���X�^���C�U�X�e�[�g�I�u�W�F�N�g�̐���
	D3D11_RASTERIZER_DESC rasterizer_desc = {};
	rasterizer_desc.FillMode				= D3D11_FILL_SOLID;
	rasterizer_desc.CullMode				= D3D11_CULL_BACK;
	rasterizer_desc.FrontCounterClockwise	= FALSE;
	rasterizer_desc.DepthBias				= 0;
	rasterizer_desc.DepthBiasClamp			= 0;
	rasterizer_desc.SlopeScaledDepthBias	= 0;
	rasterizer_desc.DepthClipEnable			= TRUE;
	rasterizer_desc.ScissorEnable			= FALSE;
	rasterizer_desc.MultisampleEnable		= FALSE;
	rasterizer_desc.AntialiasedLineEnable	= FALSE;
	hr = device->CreateRasterizerState(
		&rasterizer_desc,
		rasterizer_states[SCast(size_t, EnumRasterizerState::SOLID)].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	rasterizer_desc.FillMode = D3D11_FILL_WIREFRAME;
	rasterizer_desc.CullMode = D3D11_CULL_BACK;
	rasterizer_desc.AntialiasedLineEnable = TRUE;
	hr = device->CreateRasterizerState(
		&rasterizer_desc,
		rasterizer_states[SCast(size_t, EnumRasterizerState::WIREFRAME)].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	rasterizer_desc.FillMode = D3D11_FILL_SOLID;
	rasterizer_desc.CullMode = D3D11_CULL_NONE;
	rasterizer_desc.AntialiasedLineEnable = FALSE;
	hr = device->CreateRasterizerState(
		&rasterizer_desc,
		rasterizer_states[SCast(size_t, EnumRasterizerState::CULL_NONE)].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	rasterizer_desc.FillMode = D3D11_FILL_WIREFRAME;
	rasterizer_desc.CullMode = D3D11_CULL_NONE;
	rasterizer_desc.AntialiasedLineEnable = TRUE;
	hr = device->CreateRasterizerState(
		&rasterizer_desc,
		rasterizer_states[SCast(size_t, EnumRasterizerState::WIREFRAME_CULL_NONE)].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	rasterizer_desc.FillMode = D3D11_FILL_SOLID;
	rasterizer_desc.CullMode = D3D11_CULL_NONE;
	rasterizer_desc.FrontCounterClockwise = TRUE;
	rasterizer_desc.AntialiasedLineEnable = TRUE;
	hr = device->CreateRasterizerState(
		&rasterizer_desc,
		rasterizer_states[SCast(size_t, EnumRasterizerState::PARTICLE)].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	// �s�N�Z���V�F�[�_�[�ǂݍ���
	Shader::CreatePSFromCso("shader/luminance_extraction_ps.cso", 
		pixel_shaders[SCast(size_t, EnumPixelShader::LUMINANCE_EXTRACTION)].GetAddressOf());

	Shader::CreatePSFromCso("shader/color_extraction_ps.cso", 
		pixel_shaders[SCast(size_t, EnumPixelShader::COLOR_EXTRACTION)].GetAddressOf());
	
	Shader::CreatePSFromCso("shader/blur_ps.cso", 
		pixel_shaders[SCast(size_t, EnumPixelShader::BLUR)].GetAddressOf());
	
	Shader::CreatePSFromCso("shader/extract_emissive_model_ps.cso",
		pixel_shaders[SCast(size_t, EnumPixelShader::EXTRACT_EMISSIVE)].GetAddressOf());
	
	Shader::CreatePSFromCso("shader/screen_effect_ps.cso",
		pixel_shaders[SCast(size_t, EnumPixelShader::SCREEN_EFFECT)].GetAddressOf());
	
	// �v�Z�V�F�[�_�[�ǂݍ���
	Shader::CreateCSFromCso("shader/block_particle_cs.cso", 
		compute_shaders[SCast(size_t, EnumComputeShader::BLOCK)].GetAddressOf());
}

// �X�V����
void Graphics::Update()
{
	std::scoped_lock<std::mutex> lock(graphics_mtx);

	ID3D11RenderTargetView* null_render_target_views[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = {};
	device_context->OMSetRenderTargets(_countof(null_render_target_views), null_render_target_views, 0);

	ID3D11ShaderResourceView* null_shader_resource_views[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] = {};
	device_context->VSSetShaderResources(0, _countof(null_shader_resource_views), null_shader_resource_views);
	device_context->PSSetShaderResources(0, _countof(null_shader_resource_views), null_shader_resource_views);

	FLOAT color[] = {
		1.0f,
		1.0f,
		1.0f,
		1.0f
	};

	device_context->ClearRenderTargetView(render_target_view.Get(), color);
	device_context->OMSetRenderTargets(1, render_target_view.GetAddressOf(), depth_stencil_view.Get());

	// �T���v���[�X�e�[�g�I�u�W�F�N�g�̃o�C���h
	device_context->PSSetSamplers(
		SCast(size_t, EnumSamplerState::POINT), 1,
		sampler_states[SCast(size_t, EnumSamplerState::POINT)].GetAddressOf());

	device_context->PSSetSamplers(
		SCast(size_t, EnumSamplerState::LINEAR), 1,
		sampler_states[SCast(size_t, EnumSamplerState::LINEAR)].GetAddressOf());

	device_context->PSSetSamplers(
		SCast(size_t, EnumSamplerState::ANISOTROPIC), 1,
		sampler_states[SCast(size_t, EnumSamplerState::ANISOTROPIC)].GetAddressOf());

	device_context->PSSetSamplers(
		SCast(size_t, EnumSamplerState::BORDER), 1,
		sampler_states[SCast(size_t, EnumSamplerState::BORDER)].GetAddressOf());
}

void Graphics::StylizeWindow(BOOL new_fullscreen)
{
	windowed = new_fullscreen;

	if (new_fullscreen)
	{
		GetWindowRect(hwnd, &windowed_rect);
		SetWindowLongPtrA(hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW & ~(WS_CAPTION | WS_MAXIMIZEBOX | WS_SYSMENU | WS_THICKFRAME));

		RECT fullscreen_window_rect = {};

		HRESULT hr = E_FAIL;
		if (swap_chain)
		{
			ComPtr<IDXGIOutput> dxgi_output;
			hr = swap_chain->GetContainingOutput(&dxgi_output);
			if (hr == S_OK)
			{
				DXGI_OUTPUT_DESC output_desc;
				hr = dxgi_output->GetDesc(&output_desc);
				if (hr == S_OK)
				{
					fullscreen_window_rect = output_desc.DesktopCoordinates;
				}
			}
		}
		if (hr != S_OK)
		{
			DEVMODE devmode = {};
			devmode.dmSize = sizeof(DEVMODE);
			EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devmode);

			fullscreen_window_rect = {
				devmode.dmPosition.x,
				devmode.dmPosition.y,
				devmode.dmPosition.x + SCast(LONG, devmode.dmPelsWidth),
				devmode.dmPosition.y + SCast(LONG, devmode.dmPelsHeight)
			};
		}
		SetWindowPos(
			hwnd,
			NULL,
			fullscreen_window_rect.left,
			fullscreen_window_rect.top,
			fullscreen_window_rect.right,
			fullscreen_window_rect.bottom,
			SWP_FRAMECHANGED | SWP_NOACTIVATE);

		ShowWindow(hwnd, SW_MAXIMIZE);
	}
	else
	{
		SetWindowLongPtrA(hwnd, GWL_STYLE, windowed_style);
		SetWindowPos(
			hwnd,
			HWND_NOTOPMOST,
			windowed_rect.left,
			windowed_rect.top,
			windowed_rect.right - windowed_rect.left,
			windowed_rect.bottom - windowed_rect.top,
			SWP_FRAMECHANGED | SWP_NOACTIVATE);

		ShowWindow(hwnd, SW_NORMAL);
	}
}
