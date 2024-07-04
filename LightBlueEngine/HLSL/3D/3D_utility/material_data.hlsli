#ifndef __MATERIAL_DATA_HLSLI__
#define __MATERIAL_DATA_HLSLI__

// インクルード
#include "material.hlsli"

// 構造体
struct MaterialData
{
	// other
	float3		f0;
	float3		f90;
	float		ior;
	float		alpha_roughness;
	float3		c_diff;
	
	// 0 TEX_BASECOLOR >> t16
	float4		basecolor_factor; // rgba
	
	// 1 TEX_METALLIC_ROUGHNESS >> t17
	float		roughness_factor; // g
	float		metallic_factor; // b
		
	// 3 TEX_EMISSIVE >> t20
	float3		emissive_factor; // rgb
	float		emissive_strength; // a
	
	// 4 TEX_OCCULUSION >> t21
	float		occulusion_factor; // r
	float		occulusion_strength;
	
	// 5 TEX_CLEARCOAT >> t22
	// 6 TEX_CLEARCOAT_ROUGHNESS >> t23
	float		clearcoat_factor; // r
	float		clearcoat_f0;
	float		clearcoat_f90;
	float		clearcoat_roughness; // g
	float3		clearcoat_normal;

	// 8 TEX_TRANSMISSION >> t24
	float		transmission_factor; // r
	
	// 9 TEX_SPECULAR >> t25
	float		specular_factor; // a
	
	// 10 TEX_SPECULAR >> t26
	float3		specular_color_factor; // rgb
};

// 関数
MaterialData GetMaterialData(SbMaterial material, float2 texcoord)
{
	MaterialData material_data;
	
	// 初期化
	material_data.f0						= 0.04;
	material_data.f90					= 1.0;
	material_data.ior					= 1.5;
	material_data.alpha_roughness		= 1.0;
	material_data.c_diff					= 1.0;
	
	material_data.basecolor_factor		= 1.0;
	
	material_data.roughness_factor		= 1.0;
	material_data.metallic_factor		= 1.0;
	
	material_data.emissive_factor		= 0.0;
	
	material_data.clearcoat_factor		= 0.0;
	material_data.clearcoat_f0			= 0.04;
	material_data.clearcoat_f90			= 1.0;
	material_data.clearcoat_roughness	= 1.0;

	material_data.transmission_factor	= 0.0;
	
	material_data.specular_factor		= 1.0;
	
	material_data.specular_color_factor = 1.0;
	
	// テクスチャから取得
	
	// basecolor >> rgba
	float4 basecolor = material.pbr_metallic_roughness.basecolor_factor;
	
	if (material.pbr_metallic_roughness.basecolor_texture.index > -1)
	{
		float2 transformed_texcoord	= TransformTexcoord(texcoord, material.pbr_metallic_roughness.basecolor_texture.khr_texture_transform);
		
		float4 sampled = material_textures[TEX_BASECOLOR].Sample(sampler_states[SS_ANISOTROPIC], transformed_texcoord);
		sampled.rgb = pow(abs(sampled.rgb), GAMMA); // rgbのみ
		basecolor *= sampled;
	}
	
	material_data.basecolor_factor = basecolor;
	
	// roughness >> g
	// metallic >> b
	float roughness = material.pbr_metallic_roughness.roughness_factor;
	float metallic	= material.pbr_metallic_roughness.metallic_factor;
	
	if (material.pbr_metallic_roughness.metallic_roughness_texture.index > -1)
	{
		float2 transformed_texcoord	= TransformTexcoord(texcoord, material.pbr_metallic_roughness.basecolor_texture.khr_texture_transform);
		
		float4 sampled = material_textures[TEX_METALLIC_ROUGHNESS].Sample(sampler_states[SS_ANISOTROPIC], transformed_texcoord);
		roughness *= sampled.g;
		metallic *= sampled.b;
	}
	
	material_data.roughness_factor	= roughness;
	material_data.metallic_factor	= metallic;
	
	material_data.f0					= lerp(material_data.f0, material_data.basecolor_factor.rgb, material_data.metallic_factor);
	material_data.f90				= 1.0;
	material_data.alpha_roughness	= material_data.roughness_factor * material_data.roughness_factor;
	material_data.c_diff				= lerp(material_data.basecolor_factor.rgb, 0.0, material_data.metallic_factor);
	
	// emissive >> rgb
	float3	emissive				= material.emissive_factor;
	float	emissive_strength	= material.emissive_strength.emissive_strength;
	
	if (material.emissive_texture.index > -1)
	{
		float2 transformed_texcoord = TransformTexcoord(texcoord, material.emissive_texture.khr_texture_transform);
		
		float4 sampled = material_textures[TEX_EMISSIVE].Sample(sampler_states[SS_ANISOTROPIC], transformed_texcoord);
		sampled.rgb = pow(abs(sampled.rgb), GAMMA); // rgbのみ
		emissive				=	sampled.rgb;
		emissive_strength	=	sampled.a;
	}
	
	material_data.emissive_factor	=	emissive;
	material_data.emissive_strength =	emissive_strength;
		
	// occulusion >> r
	float occulusion = 1.0;
	
	if (material.occulusion_texture.index > -1)
	{
		float2 transformed_texcoord	= TransformTexcoord(texcoord, material.pbr_metallic_roughness.basecolor_texture.khr_texture_transform);
		
		float4 sampled				= material_textures[TEX_OCCULUSION].Sample(sampler_states[SS_ANISOTROPIC], transformed_texcoord);
		occulusion *= sampled.r;
	}
	
	material_data.occulusion_factor		= occulusion;
	material_data.occulusion_strength	= material.occulusion_texture.strength;
	
	// clearcoat >> r
	float clearcoat = material.clearcoat.clearcoat_factor;
	
	if (material.clearcoat.clearcoat_texture.index > -1)
	{
		float2 transformed_texcoord	= TransformTexcoord(texcoord, material.pbr_metallic_roughness.basecolor_texture.khr_texture_transform);
		
		float4 sampled = material_textures[TEX_CLEARCOAT].Sample(sampler_states[SS_LINEAR], transformed_texcoord);
		clearcoat *= sampled.r;
	}
	
	material_data.clearcoat_factor = clearcoat;
	
	// clearcoat_roughness >> g
	float clearcoat_roughness = material.clearcoat.clearcoat_roughness_factor;
	
	if (material.clearcoat.clearcoat_roughness_texture.index > -1)
	{
		float2 transformed_texcoord = TransformTexcoord(texcoord, material.pbr_metallic_roughness.basecolor_texture.khr_texture_transform);
		
		float4 sampled = material_textures[TEX_CLEARCOAT_ROUGHNESS].Sample(sampler_states[SS_LINEAR], transformed_texcoord);
		clearcoat_roughness *= sampled.g;
	}
	
	material_data.clearcoat_roughness	= clearcoat_roughness;
	
	material_data.clearcoat_f0			= pow((material_data.ior - 1.0) / (material_data.ior + 1.0), 2.0);
	material_data.clearcoat_f90			= 1.0;

	// transmission >> r
	float transmission = material.transmission.transmission_factor;
	
	if (material.transmission.transmission_texture.index > -1)
	{
		float2 transformed_texcoord = TransformTexcoord(texcoord, material.pbr_metallic_roughness.basecolor_texture.khr_texture_transform);
		
		float4 sampled = material_textures[TEX_TRANSMISSION].Sample(sampler_states[SS_LINEAR], transformed_texcoord);
		transmission *= sampled.r;
	}
	
	material_data.transmission_factor = transmission;
	
	// specular_color >> rgb
	// specular >> a
	float3 specular_color = material.specular.specular_color_factor;
	float specular = material.specular.specular_factor;
	
	if (material.specular.specular_texture.index > -1)
	{
		float2 transformed_texcoord = TransformTexcoord(texcoord, material.specular.specular_color_texture.khr_texture_transform);
		
		float4 sampled = material_textures[TEX_SPECULAR].Sample(sampler_states[SS_LINEAR], transformed_texcoord);
		specular *= sampled.a;
	}
	
	if (material.specular.specular_color_texture.index > -1)
	{
		float2 transformed_texcoord	= TransformTexcoord(texcoord, material.specular.specular_texture.khr_texture_transform);
		
		float4 sampled = material_textures[TEX_SPECULAR].Sample(sampler_states[SS_LINEAR], transformed_texcoord);
		specular_color *= sampled.rgb;
	}

	material_data.specular_color_factor = specular_color;
	
	float3 dielectric_specular_f0	= min(material_data.f0 * specular_color, 1.0);
	material_data.f0					= lerp(dielectric_specular_f0, material_data.basecolor_factor.rgb, material_data.metallic_factor);
	material_data.specular_factor	= specular;
	material_data.c_diff				= lerp(material_data.basecolor_factor.rgb, 0, material_data.metallic_factor);
	
	return material_data;
}

#endif // __MATERIAL_DATA_HLSLI__