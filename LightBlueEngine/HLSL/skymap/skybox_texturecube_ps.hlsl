// インクルード
#include "skymap_header.hlsli"

// 変数
TextureCube skycube : register(t32);

// 関数
float4 ShaderMain(GS_OUT_CUBOID pin) : SV_TARGET
{
	return skycube.SampleLevel(sampler_states[SS_LINEAR], pin.bearing , 0);
}