// <>インクルード
#include <DirectXMath.h>
#include <imgui.h>

// ""インクルード
// LightBlueEngine
#include "graphics/camera.h"
#include "graphics/graphics.h"
#include "graphics/light.h"
#include "graphics/texture.h"
#include "engine_utility.h"
#include "shader.h"
#include "shadow_map.h"

//-------------------------------------------
// ShadowMap メンバ関数
//-------------------------------------------

// コンストラクタ
ShadowMap::ShadowMap()
{
	Graphics*		graphics	= Graphics::GetInstance();
	ID3D11Device*	device		= graphics->GetDevice().Get();

	// Texture2D作成
	D3D11_TEXTURE2D_DESC	texture2d_desc	= {};
	ComPtr<ID3D11Texture2D> depth_buffer;
	texture2d_desc.Width				= SHADOW_MAP_WIDTH;
	texture2d_desc.Height				= SHADOW_MAP_HEIGHT;
	texture2d_desc.MipLevels			= 1;
	texture2d_desc.ArraySize			= 1;
	texture2d_desc.Format				= DXGI_FORMAT_R32_TYPELESS;
	texture2d_desc.SampleDesc.Count		= 1;
	texture2d_desc.SampleDesc.Quality	= 0;
	texture2d_desc.Usage				= D3D11_USAGE_DEFAULT;
	texture2d_desc.BindFlags			= D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	texture2d_desc.CPUAccessFlags		= 0;
	texture2d_desc.MiscFlags			= 0;
	device->CreateTexture2D(&texture2d_desc, NULL, depth_buffer.GetAddressOf());

	// dsv作成
	D3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc = {};
	depth_stencil_view_desc.Format				= DXGI_FORMAT_D32_FLOAT;
	depth_stencil_view_desc.ViewDimension		= D3D11_DSV_DIMENSION_TEXTURE2D;
	depth_stencil_view_desc.Texture2D.MipSlice	= 0;
	device->CreateDepthStencilView(depth_buffer.Get(), &depth_stencil_view_desc, depth_stencil_view.GetAddressOf());

	// srv作成
	D3D11_SHADER_RESOURCE_VIEW_DESC shader_resource_view_desc = {};
	shader_resource_view_desc.Format						= DXGI_FORMAT_R32_FLOAT;
	shader_resource_view_desc.ViewDimension				= D3D11_SRV_DIMENSION_TEXTURE2D;
	shader_resource_view_desc.Texture2D.MostDetailedMip	= 0;
	shader_resource_view_desc.Texture2D.MipLevels		= 1;
	device->CreateShaderResourceView(depth_buffer.Get(), &shader_resource_view_desc, shader_resource_view.GetAddressOf());

	// シャドウマップのビューポート設定
	viewport.TopLeftX	= 0.0f;
	viewport.TopLeftY	= 0.0f;
	viewport.Width		= SCast(FLOAT, SHADOW_MAP_WIDTH);
	viewport.Height		= SCast(FLOAT, SHADOW_MAP_HEIGHT);
	viewport.MinDepth	= 0.0f;
	viewport.MaxDepth	= 1.0f;

	// 定数バッファ作成
	D3D11_BUFFER_DESC buffer_desc = {};
	buffer_desc.ByteWidth			= sizeof(CbShadowMap);
	buffer_desc.Usage				= D3D11_USAGE_DEFAULT;
	buffer_desc.BindFlags			= D3D11_BIND_CONSTANT_BUFFER;
	buffer_desc.CPUAccessFlags		= 0;
	buffer_desc.MiscFlags			= 0;
	buffer_desc.StructureByteStride	= 0;
	device->CreateBuffer(&buffer_desc, nullptr, shadow_map_constant_buffer.GetAddressOf());

	// サンプラーステート作成
	D3D11_SAMPLER_DESC sampler_desc{};
	sampler_desc.Filter			= D3D11_FILTER_MIN_MAG_MIP_POINT;
	sampler_desc.AddressU		= D3D11_TEXTURE_ADDRESS_BORDER;
	sampler_desc.AddressV		= D3D11_TEXTURE_ADDRESS_BORDER;
	sampler_desc.AddressW		= D3D11_TEXTURE_ADDRESS_BORDER;
	sampler_desc.MipLODBias		= 0;
	sampler_desc.MaxAnisotropy	= 16;
	sampler_desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	sampler_desc.BorderColor[0] = FLT_MAX;
	sampler_desc.BorderColor[1] = FLT_MAX;
	sampler_desc.BorderColor[2] = FLT_MAX;
	sampler_desc.BorderColor[3] = FLT_MAX;
	sampler_desc.MinLOD			= 0;
	sampler_desc.MaxLOD			= D3D11_FLOAT32_MAX;
	device->CreateSamplerState(&sampler_desc, sampler_state.GetAddressOf());
}

// ImGui表示
void ShadowMap::DebugGUI()
{
	if (ImGui::CollapsingHeader("ShadowMap"))
	{
		ImGui::Image(shader_resource_view.Get(), { 256,144 }, { 0,0 }, { 1,1 }, { 1,1,1,1 });
		ImGui::SliderFloat("shadow_map_drawrect", &shadow_map_drawrect, 0.1f, 100.0f, "%.4f");
		ImGui::ColorEdit3("shadow_color", &shadow_map_constants.shadow_color.x);
		ImGui::SliderFloat("shadow_bias", &shadow_map_constants.shadow_bias, 0.0f, +0.01f);
	}
}

// シャドウマップ初期化
void ShadowMap::Clear(GameCamera* camera, Light* light, float depth)
{
	Graphics*				graphics		= Graphics::GetInstance();
	ID3D11DeviceContext*	device_context	= graphics->GetDeviceContext().Get();

	// シャドウマップを生成するための計算
	DirectX::XMVECTOR		v_up				= DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	DirectX::XMVECTOR		v_light_direction	= DirectX::XMLoadFloat3(&light->GetLightConstants().directional_light_direction);
	DirectX::XMVECTOR		v_light_focus		= DirectX::XMLoadFloat3(&light->GetLightConstants().directional_light_focus);
	DirectX::XMMATRIX		m_view				= DirectX::XMMatrixLookAtLH(v_light_direction,v_light_focus, v_up);
	DirectX::XMMATRIX		m_projection		= DirectX::XMMatrixOrthographicLH(shadow_map_drawrect, shadow_map_drawrect, 0.1f, 200.0f);

	DirectX::XMStoreFloat4x4(&shadow_map_constants.light_view_projection, m_view * m_projection);

	camera->SetView(m_view);
	camera->SetProjection(m_projection);

	device_context->VSSetConstantBuffers(
		SCast(UINT, EnumCommonConstantBuffer::SCENE), 1, camera->GetSceneCBuffer().GetAddressOf());

	device_context->ClearDepthStencilView(depth_stencil_view.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depth, 0);

	using_shadow_map = true;
}

// シャドウマップ起動
void ShadowMap::Activate(D3D11_VIEWPORT* cached_viewports_data)
{
	Graphics*				graphics		= Graphics::GetInstance();
	ID3D11DeviceContext*	device_context	= graphics->GetDeviceContext().Get();

	// ビューポート付け替え
	viewport_count = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
	device_context->RSGetViewports(&viewport_count, cached_viewports_data);
	device_context->OMGetRenderTargets(1, cached_render_target_view.ReleaseAndGetAddressOf(), cached_depth_stencil_view.ReleaseAndGetAddressOf());
	device_context->RSSetViewports(1, &viewport);

	device_context->UpdateSubresource(shadow_map_constant_buffer.Get(), 0, 0, &shadow_map_constants, 0, 0);
	device_context->VSSetConstantBuffers(
		SCast(UINT, EnumCommonConstantBuffer::SHADOWMAP), 1, shadow_map_constant_buffer.GetAddressOf());
	device_context->PSSetConstantBuffers(
		SCast(UINT, EnumCommonConstantBuffer::SHADOWMAP), 1, shadow_map_constant_buffer.GetAddressOf());

	ID3D11RenderTargetView* null_render_target_view = NULL;
	device_context->OMSetRenderTargets(1, &null_render_target_view, depth_stencil_view.Get());
}

// シャドウマップ終了
void ShadowMap::Deactivate(D3D11_VIEWPORT* cached_viewports_data)
{
	Graphics* graphics = Graphics::GetInstance();
	ID3D11DeviceContext* device_context = graphics->GetDeviceContext().Get();

	device_context->RSSetViewports(viewport_count, cached_viewports_data);
	device_context->OMSetRenderTargets(1, cached_render_target_view.GetAddressOf(), cached_depth_stencil_view.Get());

	using_shadow_map = false;
}