// �C���N���[�h
#include "skymap_header.hlsli"

// �ϐ�
TextureCube skycube : register(t32);

// �֐�
float4 ShaderMain(GS_OUT_CUBOID pin) : SV_TARGET
{
	return skycube.SampleLevel(sampler_states[SS_LINEAR], pin.bearing , 0);
}