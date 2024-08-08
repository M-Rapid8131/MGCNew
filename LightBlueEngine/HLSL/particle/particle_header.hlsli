#ifndef __PARTICLE_HEADER_HLSLI__
#define __PARTICLE_HEADER_HLSLI__

// インクルード
#include "../common.hlsli"

// 定数
static const int NUM_THREADS	= 512;
static const int SPREAD_FACTOR	= 30.0f; // 拡散の幅
static const int EMIT_RADIUS	= 3.0f;

static const float MAX_SPEED	= 10.0f;
static const float RESET_DIST	= 20.0f;
static const float DEFAULT_LIFE	= 10.0f;

// VS >> GS
struct VS_OUT_PT
{
	uint	instance_id		: SV_INSTANCEID;
};

// GS >> PS
struct GS_OUT_PT
{
	float4	position	: SV_POSITION;
	float4	color		: COLOR;
	float2	texcoord	: TEXCOORD0;
};

// 構造体
struct Particle
{
	uint	particle_index;
	float3	position;
	float	size;
	float4	color;
	float3	velocity;
	float3	acceleration;
	float3	normal;
	float	life;
	float	start_time;
};

// 定数バッファ
// 備考：modelのhlslとregister番号がかぶらないように、b3に設定した。
cbuffer PARTICLE_CONSTANT_BUFFER : register(b4)
{
	uint	group_count;
	uint	noise_gap;
	
	uint2	cbparticle_ipad;
	
	float	delta_time;
	float	accel_attenuation;
	
	float2	cbparticle_fpad;
};

cbuffer PARTICLE_EMITTER_CONSTANT_BUFFER : register(b5)
{
	uint emit_amounts;
	uint random_color;
	uint disable;
	
	uint cbparticle_emitter_ipad;

	float3	emit_position;
	float	emit_speed;
	float3	emit_force;
	float	emit_accel;
	float3	emit_direction;
	float	spread_rate;
	float4	emit_color;
	float	emit_size;
	float	life_time;
	float	start_diff;
	float	emit_radius;
}

cbuffer PARTICLE_FLOCK_CONSTANT_BUFFER : register(b6)
{
	float	position_weight;
	float	velocity_weight;
	float	flee_weight;
	float 	cbparticle_flock_fpad;
};

// 関数
float3 CalcEmitDirection(uint index)
{
	float3 rnd_f3 = GetNoiseFactor(index);
	
	float3 rnd_dir = normalize(CONVERT_TO_SNORM(rnd_f3));
	
	return lerp(emit_direction.xyz, rnd_dir, spread_rate);
}

float GetLifeRate(float particle_life)
{
	float life_rate = 0.0f;
	
	if (particle_life > 0.0f)
	{
		life_rate = clamp(particle_life, 0.0f, 1.0f);
		
		if (life_time < 1.0f && life_time > 0.0f)
			life_rate *= 1.0f / life_time;
	}
	
	return life_rate;
}

#endif // __PARTICLE_HEADER_HLSLI__