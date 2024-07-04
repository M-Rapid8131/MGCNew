#ifndef __SKYMAP_HLSLI__
#define __SKYMAP_HLSLI__

// インクルード
#include "../common.hlsli"
#include "../filter_functions.hlsli"

// 変数
Texture2D skybox : register(t32);

// 構造体
struct VS_OUT_SM
{
	float4 position : SV_POSITION;
	float2 texcoord : TEXCOORD;
};

struct VS_OUT_CUBOID
{
	float4	position	: SV_POSITION;
	float2	texcoord		: TEXCOORD;
	float3	bearing		: BEARING;
	uint	instance_id : SV_INSTANCEID;
};

struct GS_OUT_CUBOID
{
	float4	position						: SV_POSITION;
	float2	texcoord							: TEXCOORD;
	float3	bearing							: BEARING;
	uint	sv_render_target_array_index		: SV_RENDERTARGETARRAYINDEX;
};

// 定数バッファ
cbuffer SKYMAP_CONSTANT_BUFFER : register(b0)
{
	row_major float4x4	inv_view_projection;
	int					lod;
	
	int3				cbskymap_ipad;
};

// 関数
float4 SampleSkybox(float3 v, float roughness, float time)
{
	uint width, height, number_of_levels;
	skybox.GetDimensions(0, width, height, number_of_levels);
	
	float lod = roughness * float(number_of_levels - 1);
	v = normalize(v);
	
	float2 sample_point;
	sample_point.x = (atan2(v.z, v.x) + PI) / (PI * 2.0);
	sample_point.y = 1.0 - ((asin(v.y) + PI * 0.5) / PI);
	
	sample_point.x += time;
	
	float3 back_color = HSV2RGB(sky_color_base);
	
	float3 color = skybox.SampleLevel(sampler_states[SS_LINEAR], sample_point, lod).rgb * back_color;
	return float4(color, 1.0f);
}

#endif // __SKYMAP_HLSLI_