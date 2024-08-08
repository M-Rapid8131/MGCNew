// �C���N���[�h
#include "particle_header.hlsli"

// �ϐ�
RWStructuredBuffer<Particle> particle_buffer : register(u0);

// �֐�
[numthreads(NUM_THREADS, 1, 1)]
void ShaderMain( uint3 dtid : SV_DispatchThreadID )
{
	// ���݂̃p�[�e�B�N���ԍ�
	uint index = dtid.x;
	
	// ���p�[�e�B�N�����擾
	Particle particle = particle_buffer[index];

	// �����_���l�擾
	float3 random_f3 = GetNoiseFactor(index + noise_gap);
	
	// �ő吔�`�F�b�N
	if (index < emit_amounts)
	{				
		// �X�^�[�g�܂ł̎��Ԃ͉������Ȃ�
		if(particle.start_time > 0.0f)
		{
			particle.position = emit_position + CONVERT_TO_SNORM(random_f3) * emit_radius;
			
			if(!disable)
				particle.start_time -= delta_time;
			
			if(!random_color)
				particle.color = emit_color;
			particle_buffer[index]	= particle;
			return;
		}
		
		// �p�[�e�B�N���̌����𒲐�
		particle.normal = normalize(camera_position - particle.position);
		
		// �������߂Â��Ə��X�ɏ������Ȃ��Ă�������
		particle.size = lerp(0.0f, emit_size, GetLifeRate(particle.life));
		particle.life -= delta_time;
		
		// acceleration��velocity�ɉ��Z���Avelocity��position�ɉ��Z	
		particle.acceleration	= (CalcEmitDirection(index) * emit_accel) * delta_time;
		particle.velocity		+= (particle.acceleration + emit_force) * delta_time;
		particle.velocity		*=	accel_attenuation;
		particle.position		+=	particle.velocity * delta_time;

		// �p�[�e�B�N���̃X�s�[�h��������
		if(length(particle.velocity) > MAX_SPEED)
		{
			particle.acceleration = 0.0f;
			particle.velocity = normalize(particle.velocity) * MAX_SPEED;
		}
		
		// �p�[�e�B�N���Ĕz�u����
		if (particle.life < 0.0f)
		{
			if(!disable)
			{
				particle.acceleration	= CalcEmitDirection(index) * emit_accel;
				particle.velocity		= CalcEmitDirection(index) * emit_speed;
				particle.position		= emit_position + CONVERT_TO_SNORM(random_f3) * emit_radius;
				particle.life			= life_time;
				if (!random_color)
					particle.color = emit_color;
				else
					particle.color = float4(random_f3, emit_color.a);
			}		
		}
		
		particle_buffer[index] = particle;
	}
	else
	{
		particle.color.a = 0.0f;
		particle_buffer[index] = particle;
	}
}