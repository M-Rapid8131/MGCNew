// <>インクルード
#include <algorithm>
#include <implot.h>
#include <imgui.h>

// ""インクルード
// LightBlueEngine
#include "bloom_effect.h"
#include "graphics.h"
#include "shader.h"

//--------------------------------------
//	BloomEffect メンバ関数
//-------------------------------------
// コンストラクタ
BloomEffect::BloomEffect(const CbBloom init_setting, UINT width, UINT height) : bloom_constants(init_setting)
{
	Graphics* graphics = Graphics::GetInstance();
	ID3D11Device* device = graphics->GetDevice().Get();

	// ブルーム出力用
	main_screen = std::make_unique<FullscreenQuad>();

	// ブルーム適用後のフレームバッファ
	glow_extraction = std::make_unique<Framebuffer>(width, height, false);

	// ブルームを作成するフレームバッファ、解像度
	for (size_t downsampled_index = 0; downsampled_index < bloom_constants.downsampled_count; downsampled_index++)
	{
		UINT blur_width		= SCast(UINT, width * pow(2, bloom_constants.blur_buffer_mag - 1));
		UINT blur_height	= SCast(UINT, height * pow(2, bloom_constants.blur_buffer_mag - 1));

		gaussian_blur[downsampled_index][0] = std::make_unique<Framebuffer>(blur_width >> downsampled_index, blur_height >> downsampled_index, false);
		gaussian_blur[downsampled_index][1] = std::make_unique<Framebuffer>(blur_width >> downsampled_index, blur_height >> downsampled_index, false);
	}

	// シェーダー
	Shader::CreatePSFromCso("shader/glow_extraction_ps.cso", glow_extraction_ps.GetAddressOf());
	Shader::CreatePSFromCso("shader/downsampling_ps.cso", downsampling_ps.GetAddressOf());
	Shader::CreatePSFromCso("shader/horizontal_ps.cso", horizontal_ps.GetAddressOf());
	Shader::CreatePSFromCso("shader/vertical_ps.cso", vertical_ps.GetAddressOf());
	Shader::CreatePSFromCso("shader/upsampling_ps.cso", upsampling_ps.GetAddressOf());

	// 定数バッファ設定
	D3D11_BUFFER_DESC buffer_desc = {};
	buffer_desc.ByteWidth = sizeof(CbBloom);
	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	buffer_desc.CPUAccessFlags = 0;
	buffer_desc.MiscFlags = 0;
	buffer_desc.StructureByteStride = 0;
	device->CreateBuffer(&buffer_desc, nullptr, bloom_cbuffer.GetAddressOf());
}

// ImGui表示
void BloomEffect::DebugGUI(std::string name)
{
	if (ImGui::CollapsingHeader(name.c_str()))
	{
		ImGui::Image(RCast(void*, glow_extraction->GetSceneSRV().Get()), ImVec2(1280.0f / 4.0f, 720.0f / 4.0f));

		std::string param_name = "[" + name + "] extraction_threshold";
		ImGui::DragFloat(param_name.c_str(), &bloom_constants.bloom_extraction_threshold, 0.01f, 0.00f, 10.0f);

		param_name = "[" + name + "] intensity";
		ImGui::DragFloat(param_name.c_str(), &bloom_constants.bloom_intensity, 0.01f, 0.00f, 10.0f);

		param_name = "[" + name + "] smooth rate";
		ImGui::DragFloat(param_name.c_str(), &bloom_constants.smooth_rate, 0.01f, 0.00f, 10.0f);

		param_name = "[" + name + "] using_switch";
		ImGui::Checkbox(param_name.c_str(), &using_switch);

		if (ImPlot::BeginPlot(name.c_str()))
		{
			float x_data[1000];
			float y_data[1000];

			auto smooth_step = [](float a, float b, float x) {
				if (x < a)	return 0.0f;
				else if (x > b)	return 1.0f;

				float t = (std::clamp)((x - a) / (b - a), 0.0f, 1.0f);
				return t * t * (3 - 2 * t);
				};

			auto step = [](float a, float x)
				{
					if (a < x) return 1.0f;
					else return 0.0f;
				};

			for (int i = 0; i < 1000; i++)
			{
				x_data[i] = (float)i * 0.01f;
				y_data[i] = smooth_step(bloom_constants.bloom_extraction_threshold,
					bloom_constants.smooth_rate,
					x_data[i]);
			}

			param_name = "[" + name + "] SmoothStepLine";
			ImPlot::PlotLine(param_name.c_str(), x_data, y_data, 1000);
			ImPlot::EndPlot();
		}

		for (int i = 0; i < bloom_constants.downsampled_count; i++)
		{
			ImGui::Image(RCast(void*, gaussian_blur[i][0]->GetSceneSRV().Get()), ImVec2(1280.0f / 4.0f, 720.0f / 4.0f));
		}
	}
}

// ブルームを作成
void BloomEffect::MakeEffect(ID3D11ShaderResourceView* base_color_srv)
{
	Graphics* graphics = Graphics::GetInstance();
	ID3D11DeviceContext* device_context = graphics->GetDeviceContext().Get();

	ID3D11ShaderResourceView* null_srv = {};
	ID3D11ShaderResourceView* cached_srvs[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT]{};
	device_context->PSGetShaderResources(0, bloom_constants.downsampled_count, cached_srvs);

	graphics->SetDepthStencilState(EnumDepthState::ZT_OFF_ZW_OFF);
	graphics->SetRasterizerState(EnumRasterizerState::SOLID);
	graphics->SetBlendState(EnumBlendState::ALPHA, nullptr, 0xFFFFFFFF);

	device_context->UpdateSubresource(bloom_cbuffer.Get(), 0, 0, &bloom_constants, 0, 0);
	device_context->PSSetConstantBuffers(SCast(UINT, EnumCommonConstantBuffer::BLOOM), 1, bloom_cbuffer.GetAddressOf());

	// 高輝度抽出
	glow_extraction->Clear({ 0.0f,0.0f,0.0f,1.0f });
	glow_extraction->Activate(cached_viewports.data());
	{
		main_screen->Render(&base_color_srv, 0, 1, nullptr, glow_extraction_ps.Get());
	}
	glow_extraction->Deactivate(cached_viewports.data());
	device_context->PSSetShaderResources(0, 1, &null_srv);

	// 元のサイズのフレームバッファ
	gaussian_blur[0][0]->Clear({ 0.0f,0.0f,0.0f,1.0f });
	gaussian_blur[0][0]->Activate(cached_viewports.data());
	{
		main_screen->Render(glow_extraction->GetSceneSRV().GetAddressOf(), 0, 1, nullptr, downsampling_ps.Get());
	}
	gaussian_blur[0][0]->Deactivate(cached_viewports.data());
	device_context->PSSetShaderResources(0, 1, &null_srv);

	// 2つのフレームバッファを使用してガウスブラーを適用
	gaussian_blur[0][1]->Clear({ 0.0f,0.0f,0.0f,1.0f });
	gaussian_blur[0][1]->Activate(cached_viewports.data());
	{
		main_screen->Render(gaussian_blur[0][0]->GetSceneSRV().GetAddressOf(), 0, 1, nullptr, horizontal_ps.Get());
	}
	gaussian_blur[0][1]->Deactivate(cached_viewports.data());
	device_context->PSSetShaderResources(0, 1, &null_srv);

	gaussian_blur[0][0]->Clear({ 0.0f,0.0f,0.0f,1.0f });
	gaussian_blur[0][0]->Activate(cached_viewports.data());
	{
		main_screen->Render(gaussian_blur[0][1]->GetSceneSRV().GetAddressOf(), 0, 1, nullptr, vertical_ps.Get());
	}
	gaussian_blur[0][0]->Deactivate(cached_viewports.data());
	device_context->PSSetShaderResources(0, 1, &null_srv);

	for (size_t downsampled_index = 1; downsampled_index < bloom_constants.downsampled_count; downsampled_index++)
	{
		// 一つ上のサイズのフレームバッファをこのフレームバッファ内で描画
		gaussian_blur[downsampled_index][0]->Clear({ 0.0f,0.0f,0.0f,1.0f });
		gaussian_blur[downsampled_index][0]->Activate(cached_viewports.data());
		{
			main_screen->Render(gaussian_blur[downsampled_index - 1][0]->GetSceneSRV().GetAddressOf(), 0, 1, nullptr, downsampling_ps.Get());
		}
		gaussian_blur[downsampled_index][0]->Deactivate(cached_viewports.data());
		device_context->PSSetShaderResources(0, 1, &null_srv);

		// 2つのフレームバッファを使用してガウスブラーを適用
		gaussian_blur[downsampled_index][1]->Clear({ 0.0f,0.0f,0.0f,1.0f });
		gaussian_blur[downsampled_index][1]->Activate(cached_viewports.data());
		{
			main_screen->Render(gaussian_blur[downsampled_index][0]->GetSceneSRV().GetAddressOf(), 0, 1, nullptr, horizontal_ps.Get());
		}
		gaussian_blur[downsampled_index][1]->Deactivate(cached_viewports.data());
		device_context->PSSetShaderResources(0, 1, &null_srv);

		gaussian_blur[downsampled_index][0]->Clear({ 0.0f,0.0f,0.0f,1.0f });
		gaussian_blur[downsampled_index][0]->Activate(cached_viewports.data());
		{
			main_screen->Render(gaussian_blur[downsampled_index][1]->GetSceneSRV().GetAddressOf(), 0, 1, nullptr, vertical_ps.Get());
		}
		gaussian_blur[downsampled_index][0]->Deactivate(cached_viewports.data());
		device_context->PSSetShaderResources(0, 1, &null_srv);
	}

	// ブラー用フレームバッファを全てミックスする
	glow_extraction->Clear({ 0.0f,0.0f,0.0f,1.0f });
	glow_extraction->Activate(cached_viewports.data());
	{
		std::vector<ID3D11ShaderResourceView*> srvs;

		for (size_t downsampled_index = 0; downsampled_index < bloom_constants.downsampled_count; downsampled_index++)
			srvs.push_back(gaussian_blur[downsampled_index][0]->GetSceneSRV().Get());

		main_screen->Render(srvs.data(), 0, bloom_constants.downsampled_count, nullptr, upsampling_ps.Get());
	}
	glow_extraction->Deactivate(cached_viewports.data());
	device_context->PSSetShaderResources(0, bloom_constants.downsampled_count, cached_srvs);

	for (ID3D11ShaderResourceView* cached_srv : cached_srvs)
	{
		if (cached_srv) cached_srv->Release();
	}
}
