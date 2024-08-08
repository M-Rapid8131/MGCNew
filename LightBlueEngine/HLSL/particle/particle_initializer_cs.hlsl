// �C���N���[�h
#include "particle_header.hlsli"

// �ϐ�
RWStructuredBuffer<Particle> particle_buffer : register(u0);

// ���_�o�b�t�@�ňʒu�Ȃǂ��w�肷�����ɁA����CS�ňʒu�Ȃǂ��w�肷��B
// (���_�o�b�t�@���g�p���Ȃ�����)

// �֐�
[numthreads(NUM_THREADS, 1, 1)]
void ShaderMain( uint3 dtid : SV_DispatchThreadID )
{
	uint index = dtid.x;
	
	Particle particle = particle_buffer[index];
	
	// �����_���l���擾
	float3 random_f3	= GetNoiseFactor(index);
	float3 random_f3_1	= GetNoiseFactor(index + noise_gap);
	
	float3 normal = normalize(CONVERT_TO_SNORM(random_f3));
	
	particle.particle_index = index;
	particle.position		= emit_position + emit_radius * CONVERT_TO_SNORM(random_f3);
	particle.size			= emit_size;
	particle.color			= random_color ? float4(random_f3_1, emit_color.a) : emit_color;
	particle.velocity		= CalcEmitDirection(index) * emit_speed;
	particle.acceleration	= CalcEmitDirection(index) * emit_accel;
	particle.normal			= float3(0.0f,1.0f,0.0f);
	particle.life			= life_time;
	particle.start_time		= start_diff * (index + 1);
	
	particle_buffer[index] = particle;
}