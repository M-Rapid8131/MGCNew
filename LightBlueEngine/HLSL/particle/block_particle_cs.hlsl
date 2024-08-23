// �C���N���[�h
#include "particle_header.hlsli"

// �萔
static const float ERASING_SIZE = 0.2f;

// �ϐ�
RWStructuredBuffer<Particle> particle_buffer : register(u0);

// �֐�
float3 CalcFlockParticle(uint index)
{
	// ���p�[�e�B�N���̈ʒu���擾
	float3 position = particle_buffer[index].position;
	int count = 0;
	
	// �K�v�ȕϐ�
	float nearest_dist = FLT_MAX;
	float3 nearset_position = float3(0, 0, 0);
	float3 total_position = float3(0, 0, 0);
	float3 total_velocity = float3(0, 0, 0);
	float3 result;
	
	// ����J�n
	for (int other_index = 0; other_index < int(emit_amounts); other_index++)
	{
		float3 other_position = particle_buffer[other_index].position;
		
		// �������Z�o
		float dist = distance(other_position, position);
		
		if (dist > 20.0f)
			continue;
		
		if (dist != 0.0f)
		{
			// �ŋߓ_�X�V
			if (nearest_dist > dist)
			{
				nearest_dist = dist;
				nearset_position = particle_buffer[other_index].position;
			}
		}
		
		// ���v�l�ɉ��Z
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
	Particle particle = particle_buffer[index];
	
	float3 random_f3 = GetNoiseFactor(index + noise_gap);
	if (index < emit_amounts)
	{
		if(particle.life < 0.0f)
		{
			particle.life = 0.0f;
			particle.size = 0.0f;
			particle_buffer[index] = particle;
			return;
		}
		
		particle.life -= delta_time;
		particle.normal = normalize(camera_position - particle.position);
		
		if(particle.size > 0.0f)
			particle.size -= delta_time * 0.3f;
		else
			particle.size = 0.0f;
		
		//particle.acceleration = particle.normal + CONVERT_TO_SNORM(random_f3);
		
		// acceleration��velocity�ɉ��Z���Avelocity��position�ɉ��Z
		particle.velocity += (particle.normal + CONVERT_TO_SNORM(random_f3)) * delta_time;
		particle.position += particle.velocity * delta_time;
		
		if (length(particle.velocity) > MAX_SPEED)
		{
			particle.acceleration = 0.0f;
			particle.velocity = normalize(particle.velocity) * MAX_SPEED;
		}
		particle_buffer[index] = particle;
	}
	else
	{
		particle.color.rgba = 0.0f;
		particle.size = 0.0f;
		particle_buffer[index] = particle;
	}
}