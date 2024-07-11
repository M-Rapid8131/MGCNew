// インクルード
#include "../../common.hlsli"
#include "../2d_header.hlsli"
#include "bloom_header.hlsli"

Texture2D color_buffer_texture : register(t0);
float4 ShaderMain(VS_OUT_2D pin) : SV_TARGET
{
	float4 sampled_color = color_buffer_texture.Sample(sampler_states[SS_POINT], pin.texcoord);
#if 1
	return float4(smoothstep(bloom_extraction_threshold, smooth_rate, max(sampled_color.r, max(sampled_color.g, sampled_color.b))) * sampled_color.rgb, sampled_color.a);
#else
	return float4(step(bloom_extraction_threshold, max(sampled_color.r, max(sampled_color.g, sampled_color.b))) * sampled_color.rgb, sampled_color.a);
#endif
}