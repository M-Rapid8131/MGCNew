// インクルード
#include "particle_header.hlsli"

// 変数
RWStructuredBuffer<Particle> particle_buffer : register(u0);

// 関数
[numthreads(NUM_THREADS, 1, 1)]
void ShaderMain( uint3 dtid : SV_DispatchThreadID )
{
	// 現在のパーティクル番号
	uint index = dtid.x;
	
	// 自パーティクルを取得
	Particle particle = particle_buffer[index];

	// ランダム値取得
	float3 random_f3 = GetNoiseFactor(index + noise_gap);
	// 最大数チェック
	if (index < emit_amounts)
	{				
		// スタートまでの時間は何もしない
		if(particle.start_time > 0.0f)
		{
			particle.position = emit_position.xyz + CONVERT_TO_SNORM(random_f3) * emit_amplitude.xyz;
			particle.start_time -= delta_time;
			particle.color = emit_color;
			particle_buffer[index]	= particle;
			return;
		}
		
		// パーティクルの向きを調整
		particle.normal = normalize(camera_position - particle.position);
		
		// 寿命が近づくと徐々に小さくなっていく処理
		particle.size = lerp(0.0f, emit_size, GetLifeRate(particle.life));
		particle.life -= delta_time;
		
		// accelerationをvelocityに加算し、velocityをpositionに加算
		particle.acceleration	=	emit_direction.xyz * emit_accel * delta_time;
		particle.acceleration	*=	accel_attenuation;
		particle.velocity		+=	particle.acceleration * delta_time;
		particle.position		+=	particle.velocity * delta_time;
		
		// パーティクルのスピード制限処理
		if(length(particle.velocity) > MAX_SPEED)
		{
			particle.acceleration = 0.0f;
			particle.velocity = normalize(particle.velocity) * MAX_SPEED;
		}
		
		// パーティクル再配置処理
		if (particle.life < 0.0f)
		{
			particle.acceleration	= CalcEmitDirection(index) * emit_accel;
			particle.velocity		= CalcEmitDirection(index) * emit_speed;
			particle.position		= emit_position.xyz + CONVERT_TO_SNORM(random_f3) * emit_amplitude.xyz;
			particle.life			= life_time;
			particle.color			= emit_color;
		}
		// 色変更に対応
		//particle.color = float4(random_f3, emit_color.a);
		
		particle_buffer[index] = particle;
	}
	else
	{
		particle.color.a = 0.0f;
		particle_buffer[index] = particle;
	}
}