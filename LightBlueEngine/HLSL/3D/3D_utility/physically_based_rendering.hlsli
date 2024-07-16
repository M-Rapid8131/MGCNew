#ifndef __PHYSICALLY_BASED_RENDERING__
#define __PHYSICALLY_BASED_RENDERING__

// �C���N���[�h
#include "../../common.hlsli"
#include "../../filter_functions.hlsli"
#include "brdf.hlsli"
#include "material_data.hlsli"

// �֐�
float4 CalcPBR(MaterialData material_data, float3 light_vec, float3 view_vec, float3 norm_vec, float3 projection_vec,float3 object_color)
{
	float3 diffuse		= 0;
	float3 specular		= 0;
	float3 emissive		= 0;
	float3 clearcoat	= 0;
	
	const float NoL = max(0.0, dot(norm_vec, light_vec));		// �@���ƌ����ʒu�̓���
	const float NoV = max(0.0, dot(norm_vec, view_vec));
	
	const float3 reflect_vec = reflect(-light_vec, norm_vec);
	const float3 half_vec = normalize(view_vec - light_vec);
	
	if (NoL > 0.0 || NoV > 0.0)
	{
		const float NoH = max(0.0, dot(norm_vec, half_vec));
		const float HoV = max(0.0, dot(half_vec, view_vec));
		
		// �o�������˗����z�֐��Ɋ�Â��v�Z
		diffuse		+= NoL * CalcBRDFLambertian(material_data.f0, material_data.f90, material_data.c_diff, HoV);
		specular	+= NoL * CalcBRDFSpecularGGX(material_data.f0, material_data.f90, material_data.alpha_roughness, HoV, NoL, NoV, NoH, light_vec, view_vec);
		
		// �N���A�R�[�g���v�Z
		float CNoL = max(0.0, dot(material_data.clearcoat_normal, light_vec));
		float CNoV = max(0.0, dot(material_data.clearcoat_normal, view_vec));
		float CNoH = max(0.0, dot(material_data.clearcoat_normal, half_vec));
		clearcoat += CNoL * CalcBRDFSpecularGGX(material_data.clearcoat_f0, material_data.clearcoat_f90,
												material_data.clearcoat_roughness * material_data.clearcoat_roughness,
												HoV, CNoL, CNoV, CNoH, light_vec, view_vec);
	}
	
	specular *= specular + (material_data.specular_color_factor * material_data.specular_factor);
	
	// ���������F���킩��₷��������
	float3 emissive_factor = material_data.emissive_factor;
	
	// emissive�̒l��emissive_strength�̒l�Ɉˑ�
	emissive =  emissive_factor * material_data.emissive_strength;
	
	// �I�N���[�W�����̓K�p
	diffuse		+= lerp(diffuse,	diffuse		* material_data.occulusion_factor, material_data.occulusion_strength);
	specular	+= lerp(specular,	specular	* material_data.occulusion_factor, material_data.occulusion_strength);
	clearcoat	+= lerp(clearcoat,	clearcoat	* material_data.occulusion_factor, material_data.occulusion_strength);
	
	// �N���A�R�[�g
	float	clearcoat_factor		= material_data.clearcoat_factor;
	float3	clearcoat_fresnel		= Fresnel(material_data.clearcoat_f0, material_data.clearcoat_f90, clamp(dot(material_data.clearcoat_normal, view_vec), 0.0, 1.0));

	clearcoat = clearcoat * clearcoat_factor;
	
	float3 color = (diffuse + specular + emissive) * object_color;
	color = color * (1.0 - clearcoat_fresnel * clearcoat_factor) + clearcoat;
	
	return float4(color, material_data.basecolor_factor.a);
}

#endif //__PHYSICALLY_BASED_RENDERING__