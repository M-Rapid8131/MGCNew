// インクルード
#include "../../common.hlsli"
#include "../2d_header.hlsli"

Texture2D color_buffer_texture : register(t0);
float4 ShaderMain(VS_OUT_2D pin) : SV_TARGET
{
	return color_buffer_texture.Sample(sampler_states[SS_LINEAR], pin.texcoord, 0.0);
}