#ifndef __SHADING_FUNCTION_HLSLI__
#define __SHADING_FUNCTION_HLSLI__

// �֐�

//	�����o�[�g�g�U���ˌv�Z�֐�
//	norm_vec:�@��(���K���ς�)
//	light_vec:���˃x�N�g��(���K���ς�)
//	C:���ˌ�(�F�E����)
//	K:���˗�
float3 CalcLambert(float3 norm_vec,float3 light_vec,float3 C,float3 K)
{
	float power = saturate(dot(norm_vec, -light_vec));
	return C * power * K;
}

//	�t�H���̋��ʔ��ˌv�Z�֐�
//	norm_vec:�@��(���K���ς�)
//	light_vec:���˃x�N�g��(���K���ς�)
//	E:�����x�N�g��(���K���ς�)
//	C:���ˌ�(�F�E����)
//	K:���˗�
float3 CalcPhongSpecular(float3 norm_vec,float3 light_vec,float3 E,float3 C,float3 K)
{
	float3 reflect_vec = reflect(light_vec, norm_vec);
	float power = max(dot(-E, reflect_vec), 0);
	power = pow(power, 128);
	return C * power * K;
}

//	�����o�[�g�g�U���ˌv�Z�֐�
//	norm_vec:�@��(���K���ς�)
//	light_vec:���˃x�N�g��(���K���ς�)
//	C:���ˌ�(�F�E����)
//	K:���˗�
float3 CalcHalfLambert(float3 norm_vec, float3 light_vec, float3 C, float3 K)
{
	float D = saturate(dot(norm_vec, -light_vec)*0.5f+0.5f);
	return C * D * K;
}

//	�������C�g
//	norm_vec:�@��(���K���ς�)
//	E:���������x�N�g��(���K���ς�)
//	light_vec:���˃x�N�g��(���K���ς�)
//	C:���C�g�F
//	RimPower:�������C�g�̋���
float3 CalcRimLight(float3 norm_vec,float3 E,float3 light_vec, float3 C,float RimPower = 1.0f)
{
	float rim = 1.0f - saturate(dot(norm_vec, -E));
	return C * pow(rim, RimPower) * saturate(dot(light_vec, -E));
}

//	�����v�V�F�[�f�B���O
//	tex:�����v�V�F�[�f�B���O�p�e�N�X�`��
//	samp:�����v�V�F�[�f�B���O�p�T���v���[�X�e�[�g
//	norm_vec:�@��(���K���ς�)
//	light_vec:���˃x�N�g��(���K���ς�)
//	C:���ˌ�(�F�E����)
//	K:���˗�
float3 CalcRampShading(Texture2D tex,SamplerState samp,float3 norm_vec,float3 light_vec,float3 C,float3 K)
{
	float D = saturate(dot(norm_vec, -light_vec) * 0.5f + 0.5f);
	float Ramp = tex.Sample(samp, float2(D, 0.5f)).r;
	return C * Ramp * K.rgb;
}

float3 CalcBlockColor(Texture2D tex,SamplerState samp,float rate)
{
	return tex.Sample(samp, float2(rate, 0.5f)).rgb;

}

float3 CalcSphereEnvironment(Texture2D tex,SamplerState samp,in float3 color,float3 norm_vec,float3 E, float value)
{
	float3 reflect_vec = reflect(E, norm_vec);
	float2 texcoord = reflect_vec.xy * 0.5f + 0.5f;
	return lerp(color.rgb, tex.Sample(samp, texcoord).rgb, value);
}

float3 CalcHemiSphereLight(float3 normal , float3 up,float3 sky_color,float3 ground_color,float4 hemisphere_weight)
{
	float factor = dot(normal, up) * 0.5f + 0.5f;
	return lerp(ground_color, sky_color, factor) * hemisphere_weight.x;
}

float4 CalcFog(in float4 color,float4 fog_color,float2 fog_range,float eye_length)
{
	float fogAlpha = saturate((eye_length - fog_range.x) / (fog_range.y - fog_range.x));
	return lerp(color, fog_color, fogAlpha);
}

#endif // __SHADING_FUNCTION_HLSLI__