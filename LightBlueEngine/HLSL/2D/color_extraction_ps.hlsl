// �C���N���[�h
#include "2d_header.hlsli"

// �ϐ�
Texture2D texture_maps[4] : register(t0);

// �֐�
float4 ShaderMain(VS_OUT_2D pin) : SV_TARGET
{
	// ���邢�J���[�𒊏o
	float4 color = texture_maps[0].Sample(sampler_states[SS_LINEAR], pin.texcoord);
	color.rgb = smoothstep(0.6, 1.0, color.rgb);
	
	return color;
}