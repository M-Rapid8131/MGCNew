#ifndef __BRDF_HLSLI__
#define __BRDF_HLSLI__

// �C���N���[�h
#include "../../common.hlsli"
#include "image_based_lighting.hlsli"

// BRDF�F���{��ɖ󂷂Ƒo�������˗����z�֐�

// �悭�o��ϐ��̐���
// f0�F���ʂ��琂���ɕ��̂𑨂����_�̔��˗�
// f90�F�\�ʂɐ����Ɏ������������Ƃ��ɕ\�ʂ��甽�˂�����̊���
// ior�FIndex of Refraction�̗��B

// �֐�
// �t���l����
// F = f0 + (1 - f0)(1 - |V�EH|)^5
float3 Fresnel(float3 f0, float3 f90, float VoH)
{
	return f0 + (f90 - f0) * pow(clamp(1.0 - VoH, 0, 1.0), 5.0);
}

float3 Mix(float3 base, float3 layer, float3 value)
{
	return (1.0 - value) * base + value * layer;
}

float3 FresnelMix(float ior, float3 base, float3 layer, float VoH)
{
	float f0 = pow(((1 - ior) / (1 + ior)), 2);
	float3 fr = Fresnel(f0, 1.0, VoH);
	return Mix(base, layer, fr);
}

// ggx = (N�EV * ��((N�EV)^2 *(1 - ��^2) + ��^2)) * (N�EL * ��((N�EL)^2 *(1 - ��^2) + ��^2))
float CalcGGXV(float NoL, float NoV, float HoL, float HoV, float alpha_roughness)
{
	float alpha_roughness_sq = alpha_roughness * alpha_roughness; // ��^2
	
	float ggxv = NoL * sqrt(NoV * NoV * (1.0 - alpha_roughness_sq) + alpha_roughness_sq); // (N�EV * ��((N�EV)^2 *(1 - ��^2) + ��^2))
	float ggxl = NoV * sqrt(NoL * NoL * (1.0 - alpha_roughness_sq) + alpha_roughness_sq); // (N�EL * ��((N�EL)^2 *(1 - ��^2) + ��^2))
	
	float ggx_denom = ggxv * ggxl;
	float ggx_numer = HoL * HoV;
	return (ggx_denom > 0.0) ? ggx_numer / ggx_denom : 0.0;
}

//             (��^2 * x + (N�EH))           
//D =  --------------------------------------  
//     (�� * ((N�EH)^2 * (��^2 - 1) + 1)^2)  
float CalcGGXD(float NoH, float alpha_roughness)
{
	float alpha_roughness_sq = alpha_roughness * alpha_roughness;
	
	//          (N�EH)^2              (��^2 - 1) + 1
	float f = (NoH * NoH) * (alpha_roughness_sq - 1.0) + 1.0;
	return (alpha_roughness_sq + NoH) / (PI * f * f);
}

float3 CalcBRDFLambertian(float3 f0, float3 f90, float3 diffuse_color, float VoH)
{
	return (1.0 - Fresnel(f0, f90, VoH)) * (diffuse_color / PI);
}

float3 CalcBRDFSpecularGGX(float3 f0, float3 f90, float alpha_roughness,
	float VoH, float NoL, float NoV, float NoH,float3 light_vec,float3 view_vec)
{
	float3	half_vec = normalize(light_vec + view_vec);
	float	HoL		= dot(half_vec, light_vec);
	float	HoV		= dot(half_vec, view_vec);
	float3	fresnel	= Fresnel(f0, f90, VoH);
	float	ggx_v	= CalcGGXV(NoL, NoV, HoL, HoV ,alpha_roughness);
	float	ggx_d	= CalcGGXD(NoH, alpha_roughness);
	
	return fresnel * ggx_v * ggx_d;
}

float3 CalcIBLRadianceLambertian(float3 norm_vec, float3 view_vec, float roughness, float3 diffuse_color, float3 f0)
{
	float NoV = clamp(dot(norm_vec, view_vec), 0.0, 1.0);
	
	float2	brdf_sample_point	= clamp(float2(NoV, roughness), 0.0, 1.0);
	float2	f_ab				= SampleLutGGX(brdf_sample_point).rg;
	
	float3 irradiance = SampleDiffuseIem(norm_vec).rgb;
	
	float3	fr = max(1.0 - roughness, f0) - f0;
	
	float3	k_s		= f0 + fr * pow(1.0 - NoV, 5.0);
	float3	fss_ess	= k_s * f_ab.x + f_ab.y;
	
	float	ems		= (1.0 - (f_ab.x + f_ab.y));
	float3	f_avg	= (f0 + (1.0 - f0) / 21.0);
	float3	fms_ems = ems * fss_ess * f_avg / (1.0 - f_avg * ems);
	float3	k_d		= diffuse_color * (1.0 - fss_ess + fms_ems);
	
	return (fms_ems + k_d) * irradiance;
}

float3 CalcIBLRadianceGGX(float3 norm_vec, float3 view_vec, float roughness, float3 f0)
{
	float NoV = clamp(dot(norm_vec, view_vec), 0.0, 1.0);
	
	float2 brdf_sample_point		= clamp(float2(NoV, roughness), 0.0, 1.0);
	float2 f_ab					= SampleLutGGX(brdf_sample_point).rg;
	
	float3 reflect_vec		= normalize(reflect(-view_vec, norm_vec));
	float3 specular_light	= SampleSpecularPmrem(reflect_vec, roughness).rgb;
	
	float3 fr		= max(1.0 - roughness, f0) - f0;
	float3 k_s		= f0 + fr * pow(1.0 - NoV, 5.0);
	float3 fss_ess	= k_s * f_ab.x + f_ab.y;
	
	return specular_light * fss_ess;
}

float3 VolumeTransmissionRay(float3 norm_vec, float3 view_vec, float thicness, float ior, column_major float4x4 model_matrix)
{
	// ���܂������̕���
	float3 refraction_vector = refract(-view_vec, normalize(norm_vec), 1.0 / ior);
	
	// ���f���s��̉�]�Ɉˑ����Ȃ��X�P�[�����O���v�Z
	float3 model_scale;
	model_scale.x = length(mul(float4(1, 0, 0, 0), model_matrix));
	model_scale.y = length(mul(float4(0, 1, 0, 0), model_matrix));
	model_scale.z = length(mul(float4(0, 0, 1, 0), model_matrix));
	
	// ���[�J����ԂŌ������w��
	return normalize(refraction_vector) * thicness * model_scale;
}

float3 IBLVolumeRefraction(float3 norm_vec, float3 view_vec, float perceptual_roughness, float3 basecolor, float3 f0, float3 f90,
	float3 projection_vec, column_major float4x4 wolrd_transform,column_major float4x4 view_projection_transform,float ior,
	float thicness,float3 attenuation_color,float attenuation_distance)
{
	float3 transmission_ray = VolumeTransmissionRay(norm_vec, view_vec, thicness, ior, wolrd_transform);
	float3 refracted_ray_exit	= projection_vec + transmission_ray;
	
	// ���K�����ꂽ�f�o�C�X���W�Ƀ}�b�s���O���A���܃x�N�g�����t���[���o�b�t�@�ɓW�J
	float4 ndc = mul(float4(refracted_ray_exit, 1.0), view_projection_transform);
	ndc = ndc / ndc.w;
	
	float2 refraction_coords;
	refraction_coords.x = 0.5 + 0.5 * ndc.x;
	refraction_coords.y = 0.5 - 0.5 * ndc.y;
	
	// 
	return float3(1, 1, 1);
}

#endif // __BRDF_HLSLI__