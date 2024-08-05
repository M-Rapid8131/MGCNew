#ifndef __2D_HEADER_HLSLI__
#define __2D_HEADER_HLSLI__

// �C���N���[�h
#include "../common.hlsli"

// �\����
// VS -> PS
struct VS_OUT_2D
{
	float4 position : SV_POSITION;
	float4 color		: COLOR;
	float2 texcoord	: TEXCOORD;
};

// �萔�o�b�t�@
cbuffer UI_CONSTANT_BUFFER : register(b0)
{
	float4 ui_color;
};

cbuffer SCROLL_CONSTANT_BUFFER : register(b1)
{
	float2 uv_scroll;
	
	float2 cbscroll_fpad;
};

// �֐�
// �J���[�̃O���[�X�P�[���v�Z�������l��Ԃ��֐��B�P�x�@���g�p
float GrayScale(float3 base)
{
	return base.r * 0.21 + 0.72 * base.g + 0.07 * base.b;
}

// �J���[�̃I�[�o�[���C�v�Z�������l��Ԃ��֐��B
float Overlay(float base,float mix_factor)
{
	float new_factor = 1.0f;
	if(base < 0.5f)
	{
		new_factor = base * mix_factor * 2.0f;
	}
	else
	{
		new_factor = 2.0f * (base + mix_factor - base * mix_factor) - 1.0f;
	}
	return new_factor;
}

// �F������K�p�����J���[��Ԃ��֐��B��ɃQ�[����ʑS�̂̃|�X�g�G�t�F�N�g�Ŏg�p
float3 ApplyChromaticAberrationRG(Texture2D base_texture, float2 texcoord, float deviation_width)
{
	float2 r_texcoord = (texcoord - 0.5f) * (1.0f + deviation_width) + 0.5f;
	float2 g_texcoord = (texcoord - 0.5f) * (1.0f - deviation_width) + 0.5f;
	
	float col_r	= base_texture.Sample(sampler_states[SS_LINEAR], r_texcoord).r;
	float col_g	= base_texture.Sample(sampler_states[SS_LINEAR], g_texcoord).g;
	float col_b	= base_texture.Sample(sampler_states[SS_LINEAR], texcoord).b;
	
	float3 color = float3(col_r, col_g, col_b);
	return color;
}

// ��c��(�u���[�`�o�C�p�X)�̂悤�Ȃ��̂�K�p�����J���[��Ԃ��֐��B��ɃQ�[����ʑS�̂̃|�X�g�G�t�F�N�g�Ŏg�p
float3 ApplyBleachBypass(float3 base_color)
{
	// �O���[�X�P�[���l�͋P�x�@�ŎZ�o
	float gray_scale_factor = GrayScale(base_color);
	
	float3 new_color;
	new_color.r = Overlay(base_color.r, gray_scale_factor);
	new_color.g = Overlay(base_color.g, gray_scale_factor);
	new_color.b = Overlay(base_color.b, gray_scale_factor);
	
	return new_color;
}

#endif // __2D_HEADER_HLSLI__