// インクルード
#include "3D_utility/brdf.hlsli"
#include "3D_utility/material_data.hlsli"
#include "3D_utility/shading_functions.hlsli"
#include "3D_utility/physically_based_rendering.hlsli"
#include "extension/clearcoat.hlsli"
#include "3d_header.hlsli"

// 関数
float4 ShaderMain(VS_OUT_3D pin) : SV_TARGET
{
	const SbMaterial MATERIAL = materials[material];
	
	MaterialData material_data = GetMaterialData(MATERIAL, pin.texcoord);
	
	// 必要な変数
	const float3 projection_vec = pin.w_position.xyz;
	const float3 view_vec = normalize(camera_position - pin.w_position.xyz);
	const float3 light_vec = normalize(directional_light_direction);
	float3 norm_vec = normalize(pin.w_normal.xyz);
	float3 tangent = has_tangent ? normalize(pin.w_tangent.xyz) : float3(1, 0, 0);
	const float sigma = has_tangent ? pin.w_tangent.w : 1.0;
	
	tangent = normalize(tangent - norm_vec * dot(norm_vec, tangent));
	
	const float3 bitangent = normalize(cross(norm_vec, tangent) * sigma);
	
	// normal
	if (MATERIAL.normal_texture.index > -1)
	{
		float2 transformed_texcoord = TransformTexcoord(pin.texcoord, MATERIAL.normal_texture.khr_texture_transform);
		float3 normal = material_textures[TEX_NORMAL].Sample(sampler_states[SS_LINEAR], transformed_texcoord).xyz;
		normal = (normal * 2.0) - 1.0;
		normal = normalize(normal * float3(MATERIAL.normal_texture.scale, MATERIAL.normal_texture.scale, 1.0));
		norm_vec = normalize((normal.x * tangent) + (normal.y * bitangent) + (normal.z * norm_vec));
	}
	
	// clearcoat_normal
	if (MATERIAL.clearcoat.clearcoat_normal_texture.index > -1)
	{
		float2 transformed_texcoord = TransformTexcoord(pin.texcoord, MATERIAL.clearcoat.clearcoat_normal_texture.khr_texture_transform);
		float3 normal = material_textures[TEX_CLEARCOAT_NORMAL].Sample(sampler_states[SS_LINEAR], transformed_texcoord).xyz;
		normal = (normal * 2.0) - 1.0;
		normal = normalize(normal * float3(MATERIAL.clearcoat.clearcoat_normal_texture.scale, MATERIAL.clearcoat.clearcoat_normal_texture.scale, 1.0));
		material_data.clearcoat_normal = normalize((normal.x * tangent) + (normal.y * bitangent) + (normal.z * norm_vec));
	}
	
	float4 color = CalcPBR(material_data, light_vec, view_vec, norm_vec, projection_vec, object_color);
	if (color.a < 0.01f)
		discard;
	
	// 回転軸のブロックであることを示すために色を加算
	color += float4(object_color, color.a) * blink_factor;
	
	float disolve_mask = model_mask_texture.Sample(sampler_states[SS_POINT], pin.texcoord).r;
	color.a *= step(disolve_mask, object_disolve) * MATERIAL.pbr_metallic_roughness.basecolor_factor.a;
	
	return color;
}