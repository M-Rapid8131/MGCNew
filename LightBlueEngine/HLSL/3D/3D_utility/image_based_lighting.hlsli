#ifndef __IMAGE_BASED_LIGHTING_HLSLI__
#define __IMAGE_BASED_LIGHTING_HLSLI__

// インクルード
#include "../../common.hlsli"

// 変数
TextureCube skybox			: register(t32);
TextureCube diffuse_iem		: register(t33);
TextureCube specular_pmrem	: register(t34);
Texture2D	lut_ggx			: register(t35);

// 関数
float4 SampleLutGGX(float2 brdf_sample_point)
{
	return lut_ggx.Sample(sampler_states[SS_LINEAR], brdf_sample_point);
}

float4 SampleDiffuseIem(float3 v)
{
	return diffuse_iem.Sample(sampler_states[SS_LINEAR], v);
}

float4 SampleSpecularPmrem(float3 v,float roughness)
{
	uint width, height, number_of_levels;
	specular_pmrem.GetDimensions(0, width, height, number_of_levels);
	
	float lod = roughness * float(number_of_levels - 1);
	
	return specular_pmrem.SampleLevel(sampler_states[SS_LINEAR], v, lod);
}

// まだ未実装
float3 IBLVolumeRefraction(float3 norm_vec,float3 view_vec,float perceptual_roughness,float3 basecolor)
{
	return float3(1, 1, 1);
}

#endif // __IMAGE_BASED_LIGHTING_HLSLI__