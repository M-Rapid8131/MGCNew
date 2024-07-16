#ifndef __PHYSICALLY_BASED_RENDERING__
#define __PHYSICALLY_BASED_RENDERING__

// インクルード
#include "../../common.hlsli"
#include "../../filter_functions.hlsli"
#include "brdf.hlsli"
#include "material_data.hlsli"

// 関数
float4 CalcPBR(MaterialData material_data, float3 light_vec, float3 view_vec, float3 norm_vec, float3 projection_vec,float3 object_color)
{
	float3 diffuse		= 0;
	float3 specular		= 0;
	float3 emissive		= 0;
	float3 clearcoat	= 0;
	
	const float NoL = max(0.0, dot(norm_vec, light_vec));		// 法線と光源位置の内積
	const float NoV = max(0.0, dot(norm_vec, view_vec));
	
	const float3 reflect_vec = reflect(-light_vec, norm_vec);
	const float3 half_vec = normalize(view_vec - light_vec);
	
	if (NoL > 0.0 || NoV > 0.0)
	{
		const float NoH = max(0.0, dot(norm_vec, half_vec));
		const float HoV = max(0.0, dot(half_vec, view_vec));
		
		// 双方向反射率分布関数に基づく計算
		diffuse		+= NoL * CalcBRDFLambertian(material_data.f0, material_data.f90, material_data.c_diff, HoV);
		specular	+= NoL * CalcBRDFSpecularGGX(material_data.f0, material_data.f90, material_data.alpha_roughness, HoV, NoL, NoV, NoH, light_vec, view_vec);
		
		// クリアコートも計算
		float CNoL = max(0.0, dot(material_data.clearcoat_normal, light_vec));
		float CNoV = max(0.0, dot(material_data.clearcoat_normal, view_vec));
		float CNoH = max(0.0, dot(material_data.clearcoat_normal, half_vec));
		clearcoat += CNoL * CalcBRDFSpecularGGX(material_data.clearcoat_f0, material_data.clearcoat_f90,
												material_data.clearcoat_roughness * material_data.clearcoat_roughness,
												HoV, CNoL, CNoV, CNoH, light_vec, view_vec);
	}
	
	specular *= specular + (material_data.specular_color_factor * material_data.specular_factor);
	
	// 少しだけ色をわかりやすくしたい
	float3 emissive_factor = material_data.emissive_factor;
	
	// emissiveの値はemissive_strengthの値に依存
	emissive =  emissive_factor * material_data.emissive_strength;
	
	// オクルージョンの適用
	diffuse		+= lerp(diffuse,	diffuse		* material_data.occulusion_factor, material_data.occulusion_strength);
	specular	+= lerp(specular,	specular	* material_data.occulusion_factor, material_data.occulusion_strength);
	clearcoat	+= lerp(clearcoat,	clearcoat	* material_data.occulusion_factor, material_data.occulusion_strength);
	
	// クリアコート
	float	clearcoat_factor		= material_data.clearcoat_factor;
	float3	clearcoat_fresnel		= Fresnel(material_data.clearcoat_f0, material_data.clearcoat_f90, clamp(dot(material_data.clearcoat_normal, view_vec), 0.0, 1.0));

	clearcoat = clearcoat * clearcoat_factor;
	
	float3 color = (diffuse + specular + emissive) * object_color;
	color = color * (1.0 - clearcoat_fresnel * clearcoat_factor) + clearcoat;
	
	return float4(color, material_data.basecolor_factor.a);
}

#endif //__PHYSICALLY_BASED_RENDERING__