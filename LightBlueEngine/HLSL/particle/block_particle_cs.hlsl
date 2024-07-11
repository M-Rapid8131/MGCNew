// インクルード
#include "particle_header.hlsli"

// 定数
static const float ERASING_SIZE = 0.2f;

// 変数
RWStructuredBuffer<Particle> particle_buffer : register(u0);

// 関数
float3 CalcFlockParticle(uint index)
{
	// 自パーティクルの位置を取得
	float3 position = particle_buffer[index].position;
	int count = 0;
	
	// 必要な変数
	float nearest_dist = FLT_MAX;
	float3 nearset_position = float3(0, 0, 0);
	float3 total_position = float3(0, 0, 0);
	float3 total_velocity = float3(0, 0, 0);
	float3 result;
	
	// 判定開始
	for (int other_index = 0; other_index < int(emit_amounts); other_index++)
	{
		float3 other_position = particle_buffer[other_index].position;
		
		// 距離を算出
		float dist = distance(other_position, position);
		
		if (dist > 20.0f)
			continue;
		
		if (dist != 0.0f)
		{
			// 最近点更新
			if (nearest_dist > dist)
			{
				nearest_dist = dist;
				nearset_position = particle_buffer[other_index].position;
			}
		}
		
		// 合計値に加算
		total_position += particle_buffer[other_index].position;
		total_velocity += particle_buffer[other_index].velocity;

		count++;
	}
	if (count != 0)
	{
		float3 average_position = total_position / float(count);
		result = normalize(average_position - position) * position_weight;
		result += normalize(total_velocity) * velocity_weight;
		result += normalize(nearset_position - position) * flee_weight;
	}
	else
	{
		result = float3(0, 0, 0);
	}
	
	return result;
}

[numthreads(NUM_THREADS, 1, 1)]
void ShaderMain(uint3 dtid : SV_DispatchThreadID)
{
	uint index = dtid.x;
	
	float3 random_f3 = GetNoiseFactor(index);
	float3 hover = float3(0.0f, 0.5f, 0.0f);
	if (index <= emit_amounts)
	{
		Particle particle = particle_buffer[index];
		particle.life -= delta_time;
		particle.normal = normalize(camera_position - particle.position);
		
		if(particle.size > 0.0f)
			particle.size -= delta_time * 0.3f;
		
		particle.acceleration = normalize(particle.normal);
		
		// accelerationをvelocityに加算し、velocityをpositionに加算
		particle.velocity += (particle.acceleration + CONVERT_TO_SNORM(random_f3)) * delta_time * 3.0f;
		particle.position += particle.velocity * delta_time;
		
		if (length(particle.velocity) > MAX_SPEED)
		{
			particle.acceleration = 0.0f;
			particle.velocity = normalize(particle.velocity) * MAX_SPEED;
		}
		particle_buffer[index] = particle;
	}
}