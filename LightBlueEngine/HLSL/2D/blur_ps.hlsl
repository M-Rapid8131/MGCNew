// インクルード
#include "2d_header.hlsli"

// const変数
static const int SCN_MAIN		= 0;
static const int SCN_LUMINANCE	= 1;
static const int SCN_EMISSIVE	= 2;

// 変数
Texture2D color_maps_2d[3] : register(t0);

// 関数
float3 ReinhardToneMapping(float3 color)
{
	float luma = dot(color, float3(0.2126, 0.7152, 0.0722));
	float tone_mapped_luma = luma / (1.0f + luma);
	color *= tone_mapped_luma / luma;
	return color;
}

float4 ShaderMain(VS_OUT_2D pin) : SV_TARGET
{
	float4	main_color		= color_maps_2d[0].Sample(sampler_states[SS_POINT], pin.texcoord);
	float4	bloom_color		= color_maps_2d[1].Sample(sampler_states[SS_POINT], pin.texcoord);
	float4	emissive_color	= color_maps_2d[2].Sample(sampler_states[SS_POINT], pin.texcoord);
	
    float3 fragment_color	= main_color.rgb + bloom_color.rgb + emissive_color.rgb;
	float	alpha			= main_color.a;
	
	// トーンマップ
	fragment_color = ReinhardToneMapping(fragment_color);
	
	fragment_color = pow(abs(fragment_color), INV_GAMMA);
	
	return float4(fragment_color, 1.0f);
}