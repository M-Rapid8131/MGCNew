// インクルード
#include "2d_header.hlsli"

// 定数
static const float COLOR_GAP_FACTOR = 0.003f;
static const float2 VIEWPORT_OFFSET 
	= float2(0.5f, 0.5f) / float2(float(SCREEN_WIDTH), float(SCREEN_HEIGHT));

// 変数
Texture2D color_map_2d : register(t0);

// 関数
float4 ShaderMain(VS_OUT_2D pin) : SV_TARGET
{
	// 元の画面の色
	float4 base_color = color_map_2d.Sample(sampler_states[SS_LINEAR], pin.texcoord);
	
	// ラジアルブラーの計算
	float2	center		= float2(0.5f, 0.5f);
	float2	direction	= center - pin.texcoord;
	float	len			= length(direction);
	float2	offset		= normalize(direction) * VIEWPORT_OFFSET;
	
	offset *= (blur_strength * len);
	
	float4 blur_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
	
	// weightは等差数列の和の応用で計算：
	// start	= (200 / lod) * 0.9f
	// end		= (200 / lod) * 0.1f
	
	float start = (2.0f / float(blur_lod)) * 0.9f;
	float end	= (2.0f / float(blur_lod)) * 0.1f;
	
	// オフセットの計算方法：
	for (int i = 0; i < blur_lod; i++)
	{
		float rate = float(i) / float(blur_lod);
		float2 blur_texcoord = pin.texcoord + offset * lerp(0.0f, blur_size, rate);
		float weight = lerp(start, end, rate);
		
		blur_color += color_map_2d.Sample(sampler_states[SS_LINEAR], blur_texcoord) * weight;
	}
	
	// 色収差とブリーチバイパスを計算
	float3 chromatic_color = ApplyChromaticAberrationRG(color_map_2d, pin.texcoord, COLOR_GAP_FACTOR);
	float3 bleach_bypass_color = ApplyBleachBypass(base_color.rgb);
	
	// 混ぜ合わせた値をカラーとする
	float4 color = float4(lerp(bleach_bypass_color, chromatic_color, 0.75f), base_color.a);
	
	color = lerp(blur_color, color * post_effect_blend, 0.5f);
	
	return ApplyTransition(color, pin.texcoord);
}