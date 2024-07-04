// インクルード
#include "particle_header.hlsli"

// 変数
RWStructuredBuffer<Particle> particle_buffer : register(u0);

// 頂点バッファで位置などを指定する代わりに、このCSで位置などを指定する。
// (頂点バッファを使用しないため)

// 関数
[numthreads(NUM_THREADS, 1, 1)]
void ShaderMain( uint3 dtid : SV_DispatchThreadID )
{
	uint index = dtid.x;
	
	Particle particle = particle_buffer[index];
	
	// ランダム値を取得
	float3 random_f3 = GetNoiseFactor(index);
	float3 random_f3_1 = GetNoiseFactor(index + noise_gap);
	
	float3 normal = normalize(CONVERT_TO_SNORM(random_f3));
	
	particle.particle_index = index;
	particle.position		= emit_position.xyz + emit_amplitude.xyz * CONVERT_TO_SNORM(random_f3);
	particle.size			= emit_size;
	particle.color			= emit_color;
	particle.acceleration	= CalcEmitDirection(index) * emit_accel;
	particle.velocity		= CalcEmitDirection(index) * emit_speed;
	particle.normal			= float3(0.0f,1.0f,0.0f);
	particle.start_time		= start_diff * (index + 1);
	particle.life			= life_time;
	
	particle_buffer[index] = particle;
}