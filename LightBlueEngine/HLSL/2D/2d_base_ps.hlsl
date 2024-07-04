// �C���N���[�h
#include "2d_header.hlsli"

// �ϐ�
Texture2D color_map_2d : register(t0);

// �֐�
float4 ShaderMain(VS_OUT_2D pin) : SV_TARGET
{
	float4 color = color_map_2d.Sample(sampler_states[SS_LINEAR], pin.texcoord);
	return color * pin.color;
}