// �C���N���[�h
#include "2d_header.hlsli"

// �萔
static const float COLOR_GAP_FACTOR = 0.002f;

// �ϐ�
Texture2D color_map_2d : register(t0);

// �֐�
float4 ShaderMain(VS_OUT_2D pin) : SV_TARGET
{
	// ���̉�ʂ̐F�����̂܂ܕ`��
	return color_map_2d.Sample(sampler_states[SS_LINEAR], pin.texcoord);
}