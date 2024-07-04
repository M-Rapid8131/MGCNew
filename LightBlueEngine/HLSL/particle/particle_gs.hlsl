// インクルード
#include "particle_header.hlsli"

// 変数
StructuredBuffer<Particle> particle_buffer : register(t1);

// 関数
[maxvertexcount(4)]
void ShaderMain(
	point VS_OUT_PT					gin[1],
	inout TriangleStream<GS_OUT_PT> output
)
{	
	Particle particle = particle_buffer[gin[0].instance_id];
	
	float3	particle_position	= particle.position;
	float	size				= particle.size * 0.5f;
	
	const float3 billboard[] =
	{
		float3(-size, +size, 0), // 左上の角
		float3(+size, +size, 0), // 右上の角
		float3(-size, -size, 0), // 左下の角
		float3(+size, -size, 0), // 右下の角
	};
	
	const float2 texcoord[] =
	{
		float2(0.0f, 1.0f),
		float2(0.0f, 0.0f),
		float2(1.0f, 1.0f),
		float2(1.0f, 0.0f),
	};
	
	float3 z_vec = normalize(particle.normal);
	float3 x_vec = normalize(cross(z_vec, float3(0, 1, 0)));
	float3 y_vec = normalize(cross(z_vec, x_vec));
	row_major float3x3 reflect_vec = { x_vec, y_vec, z_vec };
	
	// ビルボード
	[unroll]
	for (int vertex_index = 0; vertex_index < 4; vertex_index++)
	{
		GS_OUT_PT element;
		
		element.position = mul(float4(particle_position + mul(billboard[vertex_index], reflect_vec), 1.0f), mul(view, projection));
		element.color		= particle.color;
		element.texcoord	= texcoord[vertex_index];
		output.Append(element);
	}
	
	// ストリップ繋ぎ終了
	output.RestartStrip();
}