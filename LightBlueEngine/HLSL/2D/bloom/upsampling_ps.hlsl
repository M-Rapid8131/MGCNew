// インクルード
#include "../../common.hlsli"
#include "../2d_header.hlsli"
#include "bloom_header.hlsli"

static const uint MAX_DOWNSAMPLED_COUNT = 12;
Texture2D downsampled_textures[MAX_DOWNSAMPLED_COUNT] : register(t0);
float4 ShaderMain(VS_OUT_2D pin) : SV_TARGET
{
	float3 color = 0;
	
	[unroll]
	for (uint downsampled_index = 0; downsampled_index < downsampled_count; downsampled_index++)
	{
		color += downsampled_textures[downsampled_index].Sample(sampler_states[SS_LINEAR], pin.texcoord).rgb;
	}
	return float4(color * bloom_intensity, 1.0);
}