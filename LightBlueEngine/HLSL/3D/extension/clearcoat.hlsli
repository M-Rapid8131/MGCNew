#ifndef __CLEARCOAT_HLSLI__
#define __CLEARCOAT_HLSLI__

// ""インクルード
#include "../3D_utility/brdf.hlsli"

// 構造体
struct Clearcoat
{
	float3	clearcoat_f0;
	float3	clearcoat_f90;
	float	clearcoat_factor;
	float3	clearcoat_normal;
	float	clearcoat_roughness;
};

// 関数
float3 CalcClearcoat(Clearcoat clearcoat_data, float3 light_vec, float3 view_vec, float3 norm_vec, float3 projection_vec)
{
	float3 clearcoat = 0.0f;
	clearcoat += CalcIBLRadianceGGX(clearcoat_data.clearcoat_normal, view_vec, clearcoat_data.clearcoat_roughness, clearcoat_data.clearcoat_f0);
	
	float3 half_vec = normalize(view_vec - light_vec);
	
	float NoL = max(0, dot(clearcoat_data.clearcoat_normal, light_vec));
	float NoV = max(0, dot(clearcoat_data.clearcoat_normal, view_vec));
	float NoH = max(0, dot(clearcoat_data.clearcoat_normal, half_vec));
	float HoV = max(0, dot(half_vec, view_vec));
	float roughness_sq = clearcoat_data.clearcoat_roughness * clearcoat_data.clearcoat_roughness;
	clearcoat += NoL * CalcBRDFSpecularGGX(clearcoat_data.clearcoat_f0, clearcoat_data.clearcoat_f90, roughness_sq, HoV, NoL, NoV, NoH, light_vec, view_vec);
	
	float	clearcoat_factor = 0.0f;
	float3	clearcoat_fresnel = 0.0f;
	
	clearcoat_factor		= clearcoat_data.clearcoat_factor;
	clearcoat_fresnel	= Fresnel(clearcoat_data.clearcoat_f0, clearcoat_data.clearcoat_f90, clamp(dot(clearcoat_data.clearcoat_normal, view_vec), 0.0f, 1.0f));
	clearcoat			*= clearcoat_factor;
	
	float3 result = (1.0f - clearcoat_factor * clearcoat_fresnel) + clearcoat;
	
	return result;
}

#endif