#include "graphics.h"
// <>インクルード
#include <imgui.h>

// ""インクルード
// LightBlueEngine
#include "shader.h"
#include "texture.h"

// define定数
#define INTEL_VENDOR_ID		0x8086
#define NVIDIA_VENDOR_ID	0x10DE
#define AMD_VENDOR_ID		0x1002

//--------------------------------------
//	Graphics メンバ関数
//-------------------------------------

// コンストラクタ
void Graphics::Initialize(HWND set_hwnd, bool window_mode)
{
	hwnd = set_hwnd;

	if (!window_mode)
	{
		StylizeWindow(TRUE);
	}

	HRESULT hr = S_OK;

	UINT create_device_flags = 0;
	UINT create_factory_flags = 0;

#ifdef _DEBUG
	create_device_flags |= D3D11_CREATE_DEVICE_DEBUG;
	create_factory_flags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

	ComPtr<IDXGIFactory6> dxgi_factory6;
	hr = CreateDXGIFactory2(create_factory_flags, IID_PPV_ARGS(dxgi_factory6.GetAddressOf()));
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	// アダプタ検索
	AcquireHighPerformanceAdapter(dxgi_factory6.Get(), adapter3.GetAddressOf());

	D3D_FEATURE_LEVEL feature_level = D3D_FEATURE_LEVEL_11_0;
	hr = D3D11CreateDevice(adapter3.Get(), D3D_DRIVER_TYPE_UNKNOWN, 0, create_device_flags,
		&feature_level, 1, D3D11_SDK_VERSION, device.GetAddressOf(), nullptr, device_context.GetAddressOf());

	CreateSwapChain(dxgi_factory6.Get());

	// レンダーターゲットの作成
	ComPtr<ID3D11Texture2D> back_buffer = {};
	hr = swap_chain1->GetBuffer(0, __uuidof(ID3D11Texture2D), RCast(LPVOID*, back_buffer.GetAddressOf()));
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

	if(create_device_flags & D3D11_CREATE_DEVICE_BGRA_SUPPORT)
	{
		// Direct2D、DirectWriteの初期化
		hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, graphics_2d.d2d1_factory.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		hr = swap_chain1->GetBuffer(0, __uuidof(IDXGISurface), 
			RCast(void**, graphics_2d.surface.GetAddressOf()));
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		// dpi取得
		UINT dpi;
		dpi = GetDpiForWindow(hwnd);

		// 画面の比率からX方向とY方向のdpiを取得する
		FLOAT aspect_x = SCast(FLOAT, screen_width)		/ SCast(FLOAT, screen_width + screen_height);
		FLOAT aspect_y = SCast(FLOAT, screen_height)	/ SCast(FLOAT, screen_width + screen_height);
		FLOAT dpi_x = 0.0f, dpi_y = 0.0f;
		dpi_x = SCast(FLOAT, (dpi) * aspect_x);
		dpi_y = SCast(FLOAT, (dpi) * aspect_y);

		// プロパティ設定
		D2D1_RENDER_TARGET_PROPERTIES rt_props
			= D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT,
				D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED), dpi_x, dpi_y);

		// surfaceからrender_targetを作成
		hr = graphics_2d.d2d1_factory->CreateDxgiSurfaceRenderTarget(graphics_2d.surface.Get(), 
			&rt_props, graphics_2d.d2d1_render_target.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		// dwriteのfactoryを作成
		hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
			__uuidof(IDWriteFactory), RCast(IUnknown**,graphics_2d.dwrite_factory.GetAddressOf()));
	}

	// 深度ステンシルステートオブジェクトの作成
	//深度テスト：オン　深度ライト：オン
	D3D11_DEPTH_STENCIL_DESC depth_stencil_desc = {};
	depth_stencil_desc.DepthEnable		= TRUE;
	depth_stencil_desc.DepthWriteMask	= D3D11_DEPTH_WRITE_MASK_ALL;
	depth_stencil_desc.DepthFunc		= D3D11_COMPARISON_LESS_EQUAL;
	hr = device->CreateDepthStencilState(
		&depth_stencil_desc,
		depth_stencil_states[SCast(size_t, EnumDepthState::ZT_ON_ZW_ON)].ReleaseAndGetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	//深度テスト：オン　深度ライト：オフ
	depth_stencil_desc.DepthEnable		= TRUE;
	depth_stencil_desc.DepthWriteMask	= D3D11_DEPTH_WRITE_MASK_ZERO;
	depth_stencil_desc.DepthFunc			= D3D11_COMPARISON_LESS_EQUAL;
	hr = device->CreateDepthStencilState(
		&depth_stencil_desc,
		depth_stencil_states[SCast(size_t, EnumDepthState::ZT_ON_ZW_OFF)].ReleaseAndGetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	//深度テスト：オフ　深度ライト：オン
	depth_stencil_desc.DepthEnable		= FALSE;
	depth_stencil_desc.DepthWriteMask	= D3D11_DEPTH_WRITE_MASK_ALL;
	depth_stencil_desc.DepthFunc			= D3D11_COMPARISON_LESS_EQUAL;
	hr = device->CreateDepthStencilState(
		&depth_stencil_desc,
		depth_stencil_states[SCast(size_t, EnumDepthState::ZT_OFF_ZW_ON)].ReleaseAndGetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	//深度テスト：オフ　深度ライト：オフ
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
	// 透過
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

	// 無効
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

	// 減算
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

	// 加算
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

	// ラスタライザステートオブジェクトの生成
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

	D3D11_BUFFER_DESC buffer_desc = {};
	buffer_desc.ByteWidth = sizeof(CbTransition);
	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	buffer_desc.CPUAccessFlags = 0;
	buffer_desc.MiscFlags = 0;
	buffer_desc.StructureByteStride = 0;

	device->CreateBuffer(&buffer_desc, nullptr, transition_cbuffer.GetAddressOf());

	// ピクセルシェーダー読み込み
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
	
	// 計算シェーダー読み込み
	Shader::CreateCSFromCso("shader/block_particle_cs.cso", 
		compute_shaders[SCast(size_t, EnumComputeShader::BLOCK)].GetAddressOf());

	Shader::CreateCSFromCso("shader/board_particle_start_cs.cso", 
		compute_shaders[SCast(size_t, EnumComputeShader::BOARD_START)].GetAddressOf());

	//Shader::CreateCSFromCso("shader/block_particle_cs.cso", 
	//	compute_shaders[SCast(size_t, EnumComputeShader::BLOCK)].GetAddressOf());
}

// 更新処理
void Graphics::Update()
{
	std::scoped_lock<std::mutex> lock(graphics_mtx);

	ID3D11RenderTargetView* null_render_target_views[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = {};
	device_context->OMSetRenderTargets(_countof(null_render_target_views), null_render_target_views, 0);

	ID3D11ShaderResourceView* null_shader_resource_views[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] = {};
	device_context->VSSetShaderResources(0, _countof(null_shader_resource_views), null_shader_resource_views);
	device_context->PSSetShaderResources(0, _countof(null_shader_resource_views), null_shader_resource_views);

	FLOAT color[] = {
		0.0f,
		0.0f,
		0.0f,
		0.0f
	};

	device_context->ClearRenderTargetView(render_target_view.Get(), color);
	device_context->OMSetRenderTargets(1, render_target_view.GetAddressOf(), depth_stencil_view.Get());

	// サンプラーステートオブジェクトのバインド
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
	
	device_context->UpdateSubresource(transition_cbuffer.Get(), 0, 0, &transition_constants, 0, 0);
	device_context->PSSetConstantBuffers(7, 1, transition_cbuffer.GetAddressOf());

	if (transition_texture)
	{
		device_context->PSSetShaderResources(40, 1, transition_texture.GetAddressOf());
	}
	if (transition_back_texture)
	{
		device_context->PSSetShaderResources(39, 1, transition_back_texture.GetAddressOf());
	}
}

void Graphics::DebugGUI()
{
	bool reverse = SCast(bool, transition_constants.reverse);

	if (ImGui::CollapsingHeader("Graphics"))
	{
		if(!gpu_information.empty())
			ImGui::Text(gpu_information.c_str());
		ImGui::Separator();
		ImGui::DragFloat("transition_prog", &transition_constants.transition_prog, 0.01f, 0.0f, 1.0f);
		ImGui::DragFloat("transition_smooth", &transition_constants.transition_smooth, 0.01f, 0.0f, 1.0f);

		ImGui::Checkbox("reverse", &reverse);
	}

	transition_constants.reverse = reverse ? 1 : 0;
}

void Graphics::StylizeWindow(BOOL new_screen_state)
{
	windowed = new_screen_state;

	if (new_screen_state)
	{
		GetWindowRect(hwnd, &windowed_rect);
		SetWindowLongPtrA(hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW & ~(WS_CAPTION | WS_MAXIMIZEBOX | WS_SYSMENU | WS_THICKFRAME));

		RECT fullscreen_window_rect = {};

		HRESULT hr = E_FAIL;
		if (swap_chain1)
		{
			ComPtr<IDXGIOutput> dxgi_output;
			hr = swap_chain1->GetContainingOutput(&dxgi_output);
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

void Graphics::CreateSwapChain(IDXGIFactory6* factory6_ptr)
{
	HRESULT hr = S_OK;

	if (swap_chain1)
	{
		ID3D11RenderTargetView* null_rtv = {};
		device_context->OMSetRenderTargets(1, &null_rtv, nullptr);
		render_target_view.Reset();

		DXGI_SWAP_CHAIN_DESC swap_chain_desc = {};
		swap_chain1->GetDesc(&swap_chain_desc);
		hr = swap_chain1->ResizeBuffers(swap_chain_desc.BufferCount, screen_width, screen_height, swap_chain_desc.BufferDesc.Format, swap_chain_desc.Flags);
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		ComPtr<ID3D11Texture2D> render_target_buffer;
		hr = swap_chain1->GetBuffer(0, IID_PPV_ARGS(render_target_buffer.GetAddressOf()));
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		D3D11_TEXTURE2D_DESC texture2d_desc;
		render_target_buffer->GetDesc(&texture2d_desc);

		hr = device->CreateRenderTargetView(render_target_buffer.Get(), nullptr, render_target_view.ReleaseAndGetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	}
	else
	{
		BOOL allow_tearing = FALSE;
		if (SUCCEEDED(hr))
		{
			hr = factory6_ptr->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allow_tearing, sizeof(allow_tearing));
		}
		
		tearing_supported = SUCCEEDED(hr) && allow_tearing;

		DXGI_SWAP_CHAIN_DESC1 swap_chain_desc1 = {};
		swap_chain_desc1.Width				= SCast(UINT, screen_width);
		swap_chain_desc1.Height				= SCast(UINT, screen_height);
		swap_chain_desc1.Format				= DXGI_FORMAT_B8G8R8A8_UNORM;
		swap_chain_desc1.Stereo				= FALSE;
		swap_chain_desc1.SampleDesc.Count	= 1;
		swap_chain_desc1.SampleDesc.Quality = 0;
		swap_chain_desc1.BufferUsage		= DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swap_chain_desc1.BufferCount		= 2;
		swap_chain_desc1.Scaling			= DXGI_SCALING_STRETCH;
		swap_chain_desc1.SwapEffect			= DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swap_chain_desc1.AlphaMode			= DXGI_ALPHA_MODE_UNSPECIFIED;
		swap_chain_desc1.Flags				= tearing_supported ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
		hr = factory6_ptr->CreateSwapChainForHwnd(device.Get(), hwnd, &swap_chain_desc1, nullptr, nullptr, swap_chain1.ReleaseAndGetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		ComPtr<ID3D11Texture2D> render_target_buffer;
		hr = swap_chain1->GetBuffer(0, IID_PPV_ARGS(render_target_buffer.GetAddressOf()));
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		hr = device->CreateRenderTargetView(render_target_buffer.Get(), nullptr, render_target_view.ReleaseAndGetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		// ビューポートの設定
		D3D11_VIEWPORT viewport = {};
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = SCast(float, screen_width);
		viewport.Height = SCast(float, screen_height);
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		device_context->RSSetViewports(1, &viewport);
	}
}

void Graphics::AcquireHighPerformanceAdapter(IDXGIFactory6* factory6_ptr, IDXGIAdapter3** adapter3_dptr)
{
	std::function<std::string(std::wstring)> wstos = [](const std::wstring wstr) -> std::string {
		char c[128];
		wcstombs_s(nullptr, c, wstr.c_str(), sizeof(c));
		std::string str;
		str.assign(c);
		return str;
		};

	HRESULT hr = S_OK;

	ComPtr<IDXGIAdapter3> enumerated_adapter;
	for (UINT adapter_index = 0;
		DXGI_ERROR_NOT_FOUND != factory6_ptr->EnumAdapterByGpuPreference(
			adapter_index, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(enumerated_adapter.ReleaseAndGetAddressOf()));
		adapter_index++)
	{
		DXGI_ADAPTER_DESC1 adapter_desc1;
		hr = enumerated_adapter->GetDesc1(&adapter_desc1);
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		if (adapter_desc1.VendorId == AMD_VENDOR_ID || adapter_desc1.VendorId == NVIDIA_VENDOR_ID)
		{
			std::string desc = wstos(adapter_desc1.Description);
			gpu_information.append("GPUDesc : " + desc + '\n');
			gpu_information.append("VenderID : " + std::to_string(adapter_desc1.VendorId) + '\n');
			gpu_information.append("DeviceID : " + std::to_string(adapter_desc1.DeviceId) + '\n');
			gpu_information.append("SubSysID : " + std::to_string(adapter_desc1.SubSysId) + '\n');
			gpu_information.append("Revision : " + std::to_string(adapter_desc1.Revision) + '\n');
			gpu_information.append("DedicatedVideoMemory : " + std::to_string(adapter_desc1.DedicatedVideoMemory) + '\n');
			gpu_information.append("DedicatedSystemMemory : " + std::to_string(adapter_desc1.DedicatedSystemMemory) + '\n');
			gpu_information.append("SharedSystemMemory : " + std::to_string(adapter_desc1.SharedSystemMemory) + '\n');
			gpu_information.append("AdapterLUID.HighPart : " + std::to_string(adapter_desc1.AdapterLuid.HighPart) + '\n');
			gpu_information.append("AdapterLUID.LowPart : " + std::to_string(adapter_desc1.AdapterLuid.LowPart) + '\n');
			gpu_information.append("Flags : " + std::to_string(adapter_desc1.Flags) + '\n');
			break;
		}
	}
	*adapter3_dptr = enumerated_adapter.Detach();
}

void Graphics::OnSizeChanged(LONG width, LONG height)
{
	HRESULT hr = S_OK;
	if (width != screen_width || height != screen_height)
	{
		screen_width = width;
		screen_height = height;

		ComPtr<IDXGIFactory6> dxgi_factory6;
		hr = swap_chain1->GetParent(IID_PPV_ARGS(dxgi_factory6.GetAddressOf()));
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		CreateSwapChain(dxgi_factory6.Get());
	}
}

void Graphics::LoadTransitionTexture(const wchar_t* w_filename)
{
	HRESULT hr = S_OK;
	hr = TextureLoader::LoadTextureFromFile(w_filename, &transition_texture, &transition_texture_desc);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	transition_constants.using_transition_texture = true;
}

void Graphics::LoadTransitionBackTexture(const wchar_t* w_filename)
{
	HRESULT hr = S_OK;
	hr = TextureLoader::LoadTextureFromFile(w_filename, &transition_back_texture, &transition_back_texture_desc);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	transition_constants.using_transition_back_texture = true;
}