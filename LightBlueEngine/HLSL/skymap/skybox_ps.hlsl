// インクルード
#include "skymap_header.hlsli"

// 変数
TextureCube skycube : register(t32);

// 関数
float4 ShaderMain(VS_OUT_SM pin) : SV_TARGET
{
	float4 ndc;
	ndc.x = (pin.texcoord.x * 2.0) - 1.0;
	ndc.y = 1.0 - (pin.texcoord.y * 2.0);
	ndc.z = 1;
	ndc.w = 1;
	
	float4 r = mul(ndc, inv_view_projection);
	r /= r.w;
		
	float3 back_color = HSV2RGB(sky_color_base);
	
	return float4(skycube.SampleLevel(sampler_states[SS_LINEAR], r.xyz, lod).rgb * back_color, 1.0f);
}