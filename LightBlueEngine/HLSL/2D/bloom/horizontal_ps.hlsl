// インクルード
#include "../../common.hlsli"
#include "../2d_header.hlsli"

Texture2D color_buffer_texture : register(t0);
float4 ShaderMain(VS_OUT_2D pin) : SV_TARGET
{
	uint mip_level = 0, width, height, number_of_levels;
	color_buffer_texture.GetDimensions(mip_level, width, height, number_of_levels);
	const float aspect_ratio = width / height;
	
	const float OFFSET[3] = { 0.0, 1.3846153846, 3.2307692308 };
	const float WEIGHT[3] = { 0.2270270270, 0.3162162162, 0.0702702703 };
	
	float4 color = color_buffer_texture.Sample(sampler_states[SS_BORDER], pin.texcoord) * WEIGHT[0];
	
	for (int i = 1; i < 3; i++)
	{
		color += color_buffer_texture.Sample(sampler_states[SS_BORDER], pin.texcoord + float2(OFFSET[i] / width, 0.0)) * WEIGHT[i];
		color += color_buffer_texture.Sample(sampler_states[SS_BORDER], pin.texcoord - float2(OFFSET[i] / width, 0.0)) * WEIGHT[i];
	}

	return color;
}