// �C���N���[�h
#include "2d_header.hlsli"

// �ϐ�
Texture2D texture_maps[4] : register(t0);

// �֐�
float4 ShaderMain(VS_OUT_2D pin) : SV_TARGET
{
	// ���̉�ʂ̐F
	float4 color = texture_maps[0].Sample(sampler_states[SS_ANISOTROPIC], pin.texcoord);
	
	// �����ł͉��d���ϖ@���g�p�B
	color.rgb = smoothstep(0.5, 0.8, dot(color.rgb, float3(0.299, 0.587, 0.114))) * color.rgb;
	return color;
}