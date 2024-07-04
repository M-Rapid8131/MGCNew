// <>インクルード
#include <imgui.h>

// ""インクルード
// LightBlueEngine
#include "light.h"
#include "graphics.h"

// using
using namespace std;

//--------------------------------------
//	Light メンバ関数
//-------------------------------------

// コンストラクタ
Light::Light()
{
	Graphics*		graphics	= Graphics::GetInstance();
	ID3D11Device*	device		= graphics->GetDevice().Get();

	HRESULT hr = S_OK;

	// 定数バッファ作成
	D3D11_BUFFER_DESC buffer_desc= {};
	buffer_desc.ByteWidth			= sizeof(CbLight);
	buffer_desc.Usage				= D3D11_USAGE_DEFAULT;
	buffer_desc.BindFlags			= D3D11_BIND_CONSTANT_BUFFER;
	buffer_desc.CPUAccessFlags		= 0;
	buffer_desc.MiscFlags			= 0;
	buffer_desc.StructureByteStride = 0;
	hr = device->CreateBuffer(&buffer_desc, nullptr, light_cbuffer.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	buffer_desc.ByteWidth = sizeof(CbSkyColor);
	hr = device->CreateBuffer(&buffer_desc, nullptr, sky_color_cbuffer.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
}

// デストラクタ
Light::~Light()
{
	light_cbuffer.Reset();
	sky_color_cbuffer.Reset();
}

// ImGui表示
void Light::DebugGUI()
{
	if (ImGui::CollapsingHeader("Light"))
	{
		if (ImGui::SliderFloat3("light_dir", &light_constants.directional_light_direction.x, -1.0f, +1.0f, "%.2f")) 
		{

		}
		if (ImGui::SliderFloat("light_radiance", &light_constants.radiance, 0.1f, 100.0f, "%.2f")) 
		{

		}
		if (ImGui::DragFloat3("light_focus", &light_constants.directional_light_focus.x, 0.1f))
		{

		}
		if (ImGui::ColorEdit3("ambient_color", &light_constants.ambient_color.x))
		{

		}
		if (ImGui::ColorEdit3("directional_light_color", &light_constants.directional_light_color.x)) 
		{

		}
	}
}

// 更新処理
void Light::Update()
{
	Graphics*					graphics		= Graphics::GetInstance();
	std::lock_guard<std::mutex> lock(graphics->GetMutex());
	ID3D11DeviceContext*		device_context	= graphics->GetDeviceContext().Get();

	// ライトの方向の正規化
	float x = light_constants.directional_light_direction.x;
	float y = light_constants.directional_light_direction.y;
	float z = light_constants.directional_light_direction.z;
	float light_len = sqrtf(x * x + y * y + z * z);

	light_constants.directional_light_direction = {
		light_constants.directional_light_direction.x / light_len,
		light_constants.directional_light_direction.y / light_len,
		light_constants.directional_light_direction.z / light_len,
	};

	DirectX::XMVECTOR v_from, v_to;
	v_from	= DirectX::XMLoadFloat3(&sky_color_constants.sky_mix_color_base);
	v_to	= DirectX::XMLoadFloat3(&sky_color_constants.sky_mix_color_dest);
	v_from	= DirectX::XMVectorLerp(v_from, v_to, 0.01f);

	DirectX::XMStoreFloat3(&sky_color_constants.sky_mix_color_base, v_from);

	//cbuffer LIGHT_CONSTANT_BUFFER : register(b10)	
	device_context->UpdateSubresource(light_cbuffer.Get(), 0, 0, &light_constants, 0, 0);
	device_context->VSSetConstantBuffers(
		SCast(UINT, EnumCommonConstantBuffer::LIGHT), 1, light_cbuffer.GetAddressOf());
	device_context->PSSetConstantBuffers(
		SCast(UINT, EnumCommonConstantBuffer::LIGHT), 1, light_cbuffer.GetAddressOf());

	//cbuffer SKY_COLOR_CONSTANT_BUFFER : register(b11)	
	device_context->UpdateSubresource(sky_color_cbuffer.Get(), 0, 0, &sky_color_constants, 0, 0);
	device_context->VSSetConstantBuffers(
		SCast(UINT, EnumCommonConstantBuffer::SKY_COLOR), 1, sky_color_cbuffer.GetAddressOf());
	device_context->PSSetConstantBuffers(
		SCast(UINT, EnumCommonConstantBuffer::SKY_COLOR), 1, sky_color_cbuffer.GetAddressOf());
}