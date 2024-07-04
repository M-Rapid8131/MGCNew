// <>インクルード
#include <d3d11.h>
#include <random>
#include <numeric>
#include <imgui.h>

// ""インクルード
// LightBlueEngine
#include "graphics/graphics.h"
#include "random_noise.h"
#include "engine_utility.h"

// ゲームソースファイル
#include "misc.h"

// define
#define UNORM_TO_HEX(unorm) (UINT)(unorm * 0xff)

// 0xaabbggaa
// シフト演算子を使って各色の値を作成
#define CHANNEL_R(num)	(UINT)(num << 0)
#define CHANNEL_G(num)	(UINT)(num << 8)
#define CHANNEL_B(num)	(UINT)(num << 16)
#define CHANNEL_A(num)	(UINT)(num << 24)

//-------------------------------------------
// RandomNoise メンバ関数
//-------------------------------------------

// コンストラクタ
RandomNoise::RandomNoise(UINT width, UINT height)
{
	noise_width		= width;
	noise_height	= height;
	noise_size		= width * height;

	noise_data[0].resize(SCast(size_t, width) * SCast(size_t, height));
	noise_data[1].resize(SCast(size_t, width) * SCast(size_t, height));
	noise_data[2].resize(SCast(size_t, width) * SCast(size_t, height));
	noise_color = std::make_unique<DWORD[]>(noise_size);

	MakeNoise();
}

// ノイズテクスチャ作成
void RandomNoise::MakeNoise()
{
	Graphics*				graphics		= Graphics::GetInstance();
	ID3D11Device*			device			= graphics->GetDevice().Get();
	ID3D11DeviceContext*	device_context	= graphics->GetDeviceContext().Get();

	HRESULT hr = S_OK;

	// 順番に番号を付与したあと、シャッフルする
	for (size_t color = 0; color < 3; color++)
	{
		const int SEED = rand() % 10000 + 1;
		std::iota(noise_data[color].begin(), noise_data[color].end(), 0);
		std::shuffle(noise_data[color].begin(), noise_data[color].end(), std::mt19937(SEED));
	}

	// ピクセルごとの色を設定
	for (size_t i = 0; i < noise_size; i++)
	{
		UINT color_data[3] = {};
		for (size_t color = 0; color < 3; color++)
		{
			float data_num = SCast(float, noise_data[color].at(i));
			data_num /= noise_size;
			data_num = SCast(float, UNORM_TO_HEX(data_num));

			color_data[color] = SCast(UINT, data_num);
		}

		UINT r = CHANNEL_R(color_data[0]);		// 0x aa  bb  gg 'RR'
		UINT g = CHANNEL_G(color_data[1]);		// 0x aa  bb 'GG' rr
		UINT b = CHANNEL_B(color_data[2]);		// 0x aa 'BB' gg  rr
		UINT a = 0xff000000;					// 0x'AA' bb  gg  rr

		UINT merged_color_data = r + g + b + a;

		noise_color[i] = merged_color_data;
	}

	// テクスチャ作成
	D3D11_TEXTURE2D_DESC texture2d_desc = {};
	texture2d_desc.Width				= noise_width;
	texture2d_desc.Height				= noise_height;
	texture2d_desc.MipLevels			= 1;
	texture2d_desc.ArraySize			= 1;
	texture2d_desc.Format				= DXGI_FORMAT_R8G8B8A8_UNORM;
	texture2d_desc.SampleDesc.Count		= 1;
	texture2d_desc.SampleDesc.Quality	= 0;
	texture2d_desc.Usage				= D3D11_USAGE_DEFAULT;
	texture2d_desc.BindFlags			= D3D11_BIND_SHADER_RESOURCE;
	texture2d_desc.CPUAccessFlags		= 0;
	texture2d_desc.MiscFlags			= 0;

	D3D11_SUBRESOURCE_DATA	subresource_data = {};
	subresource_data.pSysMem			= noise_color.get();
	subresource_data.SysMemPitch		= sizeof(DWORD) * noise_width;

	hr = device->CreateTexture2D(&texture2d_desc, &subresource_data, noise_texture.ReleaseAndGetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	// シェーダーリソース作成
	D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
	srv_desc.Format					= DXGI_FORMAT_R8G8B8A8_UNORM;
	srv_desc.ViewDimension			= D3D11_SRV_DIMENSION_TEXTURE2D;
	srv_desc.Texture2D.MipLevels	= 1;

	hr = device->CreateShaderResourceView(noise_texture.Get(), &srv_desc, noise_srv.ReleaseAndGetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	device_context->VSSetShaderResources(41, 1, noise_srv.GetAddressOf());
	device_context->GSSetShaderResources(41, 1, noise_srv.GetAddressOf());
	device_context->CSSetShaderResources(41, 1, noise_srv.GetAddressOf());
	device_context->PSSetShaderResources(41, 1, noise_srv.GetAddressOf());
}

// ImGui表示
void RandomNoise::DebugGUI()
{
	if (ImGui::CollapsingHeader("RandomNoise"))
	{
		ImGui::Image(RCast(void*, noise_srv.Get()), ImVec2(512, 512));
	}
}