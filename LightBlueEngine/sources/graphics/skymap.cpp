// ""インクルード
// LightBlueEngine
#include "shader.h"
#include "camera.h"
#include "graphics.h"
#include "skymap.h"
#include "texture.h"

//-----------------------------------------
// SkyMap メンバ関数
//-----------------------------------------

// コンストラクタ
SkyMap::SkyMap(const wchar_t* w_filename)
{
	Graphics*		graphics	= Graphics::GetInstance();
	ID3D11Device*	device		= graphics->GetDevice().Get();

	HRESULT	hr = S_OK;

	D3D11_TEXTURE2D_DESC		texture2d_desc = {};
	ID3D11ShaderResourceView*	skymap_srv;

	std::vector<const wchar_t*> w_skymap_filenames = {
		w_filename,
		L"./resources/skybox/diffuse_iem.dds",			 // IBLのみで使用
		L"./resources/skybox/specular_pmrem.dds",		 // IBLのみで使用
		L"./resources/skybox/lut_ggx.dds",				 // IBLのみで使用
	};

	skymap_srvs.resize(w_skymap_filenames.size());

	int count = 0;
	for (const wchar_t* w_skymap_filename : w_skymap_filenames)
	{
		TextureLoader::LoadTextureFromFile(w_skymap_filename, &skymap_srv, &texture2d_desc, 0);
		skymap_srvs.at(count).Attach(std::move(skymap_srv));
		if (count == 0)
		{
			is_texturecube = texture2d_desc.MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE;
		}
		count++;
	}

	// 通常のスカイマップ用
	Shader::CreateVSFromCso("shader/skymap_vs.cso", skymap_vs.GetAddressOf(), NULL, NULL, 0);
	Shader::CreatePSFromCso("shader/skymap_ps.cso", skymap_ps.GetAddressOf());
	Shader::CreatePSFromCso("shader/skybox_ps.cso", skybox_ps.GetAddressOf());

	// キューブマップ用
	Shader::CreateVSFromCso("shader/skymap_texturecube_vs.cso", skymap_texturecube_vs.GetAddressOf(), NULL, NULL, 0);
	Shader::CreateGSFromCso("shader/skymap_texturecube_gs.cso", skymap_texturecube_gs.GetAddressOf());
	Shader::CreatePSFromCso("shader/skymap_texturecube_ps.cso", skymap_texturecube_ps.GetAddressOf());
	Shader::CreatePSFromCso("shader/skybox_texturecube_ps.cso", skybox_texturecube_ps.GetAddressOf());

	// buffer_desc取得
	D3D11_BUFFER_DESC	buffer_desc = {};
	buffer_desc.ByteWidth			= (sizeof(CbSkymap) + 0x0f) & ~0x0f;
	buffer_desc.Usage				= D3D11_USAGE_DEFAULT;
	buffer_desc.BindFlags			= D3D11_BIND_CONSTANT_BUFFER;
	buffer_desc.CPUAccessFlags		= 0;
	buffer_desc.MiscFlags			= 0;
	buffer_desc.StructureByteStride	= 0;

	hr = device->CreateBuffer(&buffer_desc, nullptr, skymap_cbuffer.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
}

// 描画処理
void SkyMap::Render(DirectX::XMFLOAT4X4 view, DirectX::XMFLOAT4X4 projection, int lod)
{
	Graphics*				graphics		= Graphics::GetInstance();
	ID3D11DeviceContext*	device_context	= graphics->GetDeviceContext().Get();

	device_context->IASetVertexBuffers(0, 0, NULL, NULL, NULL);
	device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	device_context->IASetInputLayout(NULL);

	device_context->VSSetShader(skymap_vs.Get(), 0, 0);
	device_context->PSSetShader(is_texturecube ? skybox_ps.Get() : skymap_ps.Get(), 0, 0);
	
	// 環境SRVのセット
	int count = SCast(size_t, EnumSkyboxTextureID::SKYBOX);
	for (auto skymap_srv : skymap_srvs)
	{
		device_context->PSSetShaderResources(count, 1, skymap_srv.GetAddressOf());
		count++;
	}
	
	// view行列×projection行列の逆行列
	DirectX::XMMATRIX m_view		= DirectX::XMLoadFloat4x4(&view);
	DirectX::XMMATRIX m_projection	= DirectX::XMLoadFloat4x4(&projection);

	DirectX::XMStoreFloat4x4(&skymap_constant.inv_view_projection,
		DirectX::XMMatrixInverse(NULL, m_view * m_projection));

	skymap_constant.lod = lod;

	device_context->UpdateSubresource(skymap_cbuffer.Get(), 0, 0, &skymap_constant, 0, 0);
	device_context->PSSetConstantBuffers(SCast(size_t, EnumBufferSkymap::SKYMAP), 1, skymap_cbuffer.GetAddressOf());

	device_context->Draw(4, 0);

	device_context->VSSetShader(NULL, 0, 0);
	device_context->PSSetShader(NULL, 0, 0);
}

// 描画処理(Cubemap用)
void SkyMap::Render2Cubemap()
{
	Graphics*				graphics		= Graphics::GetInstance();
	ID3D11DeviceContext*	device_context	= graphics->GetDeviceContext().Get();

	device_context->IASetVertexBuffers(0, 0, NULL, NULL, NULL);
	device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	device_context->IASetInputLayout(NULL);

	device_context->VSSetShader(skymap_texturecube_vs.Get(), 0, 0);
	device_context->GSSetShader(skymap_texturecube_gs.Get(), 0, 0);
	device_context->PSSetShader(is_texturecube ? skybox_texturecube_ps.Get() : skymap_texturecube_ps.Get(), 0, 0);

	// 環境SRVのセット
	int count = SCast(size_t, EnumSkyboxTextureID::SKYBOX);
	for (auto skymap_srv : skymap_srvs)
	{
		device_context->PSSetShaderResources(count, 1, skymap_srv.GetAddressOf());
		count++;
	}

	// 6面分のDrawコール
	device_context->DrawInstanced(4, 6, 0, 0);

	device_context->VSSetShader(NULL, 0, 0);
	device_context->GSSetShader(NULL, 0, 0);
	device_context->PSSetShader(NULL, 0, 0);
}