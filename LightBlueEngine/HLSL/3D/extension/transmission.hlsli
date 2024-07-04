#ifndef __TRANSMISSION_HLSLI__
#define __TRANSMISSION_HLSLI__

// インクルード
#include "../3D_utility/brdf.hlsli"

// 構造体
struct Transmission
{
	float transmission_factor;
};

// 関数
float3 CalcBTDFSpecularGGX(float3 f0, float3 f90, float alpha_roughness,
	float3 view_vec, float3 norm_vec, float3 light_vec)
{
	float3 half_vec = normalize(light_vec + view_vec);
	float HoL		= dot(half_vec, light_vec);
	float HoV		= dot(half_vec, view_vec);
	float NoL		= dot(norm_vec, light_vec);
	float NoV		= dot(norm_vec, view_vec);
	
	float3 half_transmission_vec
					= normalize(view_vec - 2 * (NoL) * norm_vec + light_vec);
	
	float NoHT = dot(norm_vec, half_transmission_vec);
	
	float HToL = dot(half_transmission_vec, light_vec);
	float HToV = dot(half_transmission_vec, view_vec);
	
	float3 fresnel = Fresnel(f0, f90, HoV);
	float v_transmission = CalcGGXV(HToL / NoL, HToV / NoV, HoL, HoV, alpha_roughness);
	float d_transmission = CalcGGXD(NoHT, alpha_roughness);
	
	return fresnel * v_transmission * d_transmission;
}

#endif // __TRANSMISSION_HLSLI__