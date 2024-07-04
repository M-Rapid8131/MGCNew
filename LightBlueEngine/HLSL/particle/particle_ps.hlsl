// �C���N���[�h
#include "particle_header.hlsli"

// �֐�
float4 ShaderMain(GS_OUT_PT pin) : SV_TARGET
{
	float4 color = pin.color;
	
	// ���S����̋���
	float dist = length(pin.texcoord - 0.5f) * 2.0f;
	float alpha = 1.0f - saturate(dist);

	color.a *= alpha;
	
	return color;
}