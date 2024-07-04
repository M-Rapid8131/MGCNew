// �C���N���[�h
#include "skymap_header.hlsli"

// �֐�
float4 ShaderMain(GS_OUT_CUBOID pin) : SV_TARGET
{	
	float3 v = normalize(pin.bearing);
	const float ROUGHNESS = 0;
	return SampleSkybox(v, ROUGHNESS, 0.01f);
}