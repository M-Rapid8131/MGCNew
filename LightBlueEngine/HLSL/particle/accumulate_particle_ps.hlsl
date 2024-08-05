// インクルード
#include "../3D/3D_utility/material.hlsli"
#include "../3D/3D_utility/material_data.hlsli"
#include "../3D/3D_utility/physically_based_rendering.hlsli"
#include "../3D/3d_header.hlsli"
#include "particle_header.hlsli"

// 変数
AppendStructuredBuffer<Particle> append_particle_buffer : register(u1);

// 関数
void ShaderMain(VS_OUT_3D model_pin)
{
	const SbMaterial MATERIAL = materials[material];
	
	MaterialData material_data = GetMaterialData(MATERIAL, model_pin.texcoord);
	
	// 必要な変数
	const float3 projection_vec = model_pin.w_position.xyz;
	const float3 view_vec = normalize(model_pin.w_position.xyz - camera_position);
	const float3 light_vec = normalize(directional_light_direction);
	float3 norm_vec = model_pin.w_normal.xyz;
	float3 tangent = has_tangent ? normalize(model_pin.w_tangent.xyz) : float3(1, 0, 0);
	const float sigma = has_tangent ? model_pin.w_tangent.w : 1.0;
	
	tangent = normalize(tangent - norm_vec * dot(norm_vec, tangent));
	
	const float3 bitangent = normalize(cross(norm_vec, tangent) * sigma);
	
	// normal
	//if (MATERIAL.normal_texture.index > -1)
	//{
	//	float2 transformed_texcoord = TransformTexcoord(model_pin.texcoord, MATERIAL.normal_texture.khr_texture_transform);
	//	float3 normal = material_textures[TEX_NORMAL].Sample(sampler_states[SS_LINEAR], transformed_texcoord).xyz;
	//	normal = (normal * 2.0) - 1.0;
	//	normal = normalize(normal * float3(MATERIAL.normal_texture.scale, MATERIAL.normal_texture.scale, 1.0));
	//	norm_vec = normalize((normal.x * tangent) + (normal.y * bitangent) + (normal.z * norm_vec));
	//}
	
	float4 color = material_data.basecolor_factor;
	if (color.a < 0.01f)
		discard;

	uint num_structs, stride;
	append_particle_buffer.GetDimensions(num_structs, stride);
	
	Particle particle;
	particle.particle_index = 0;
	particle.position		= model_pin.w_position.xyz - float3(1.0f, 0.0f, 0.0f);
	particle.size			= emit_size;
	particle.color			= float4(object_color, color.a);
	particle.velocity		= norm_vec * emit_speed;
	particle.acceleration	= 0.0f;
	particle.normal			= norm_vec;
	particle.life			= life_time;
	particle.start_time		= 0.0f;
	
	append_particle_buffer.Append(particle);
}