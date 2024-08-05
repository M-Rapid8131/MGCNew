#ifndef __2D_HEADER_HLSLI__
#define __2D_HEADER_HLSLI__

// インクルード
#include "../common.hlsli"

// 構造体
// VS -> PS
struct VS_OUT_2D
{
	float4 position : SV_POSITION;
	float4 color		: COLOR;
	float2 texcoord	: TEXCOORD;
};

// 定数バッファ
cbuffer UI_CONSTANT_BUFFER : register(b0)
{
	float4 ui_color;
};

cbuffer SCROLL_CONSTANT_BUFFER : register(b1)
{
	float2 uv_scroll;
	
	float2 cbscroll_fpad;
};

// 関数
// カラーのグレースケール計算をした値を返す関数。輝度法を使用
float GrayScale(float3 base)
{
	return base.r * 0.21 + 0.72 * base.g + 0.07 * base.b;
}

// カラーのオーバーレイ計算をした値を返す関数。
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

// 色収差を適用したカラーを返す関数。主にゲーム画面全体のポストエフェクトで使用
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

// 銀残し(ブリーチバイパス)のようなものを適用したカラーを返す関数。主にゲーム画面全体のポストエフェクトで使用
float3 ApplyBleachBypass(float3 base_color)
{
	// グレースケール値は輝度法で算出
	float gray_scale_factor = GrayScale(base_color);
	
	float3 new_color;
	new_color.r = Overlay(base_color.r, gray_scale_factor);
	new_color.g = Overlay(base_color.g, gray_scale_factor);
	new_color.b = Overlay(base_color.b, gray_scale_factor);
	
	return new_color;
}

#endif // __2D_HEADER_HLSLI__