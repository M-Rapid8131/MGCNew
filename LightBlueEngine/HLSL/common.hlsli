#ifndef __COMMON_HLSL__
#define __COMMON_HLSL__

// インクルード
#include "filter_functions.hlsli"

// マクロ
#define CONVERT_TO_SNORM(unorm_float) (unorm_float * 2.0f - 1.0f)		// 0.0 ~ 1.0を-1.0 ~ +1.0に変換
#define CONVERT_TO_UNORM(snorm_float) ((unorm_float + 1.0f) * 0.5f)		// -1.0 ~ +1.0を0.0 ~ 1.0に変換

// 定数
static const int 	SS_POINT		= 0;
static const int 	SS_LINEAR		= 1;
static const int 	SS_ANISOTROPIC	= 2;
static const int 	SS_BORDER	= 3;
static const int	SCREEN_WIDTH	= 1280;
static const int	SCREEN_HEIGHT	= 720;

static const float	GAMMA			= 2.2f;
static const float	INV_GAMMA		= 1.0f / 2.2f;
static const float	PI				= 3.141592f;
static const float	EPSILON			= 1.0e-5f;
static const float	FLT_MAX			= 3.402823466e+38F;

// 変数
SamplerState	 sampler_states[4] : register(s0);

Texture2D	mask_texture : register(t40);
Texture2D	noise_texture : register(t41);

// 定数バッファ
cbuffer SCENE_CONSTANT_BUFFER : register(b9)
{
	int						blur_lod;
	int3					cbscene_ipad;
	
	row_major float4x4		view;
	row_major float4x4		projection;
	float3					camera_position;
	float					blur_strength;
	float					blur_size;
	float3					cbscene_fpad;
}

cbuffer LIGHT_CONSTANT_BUFFER : register(b10)
{
	float4	ambient_color				= { 0.2f, 0.2f, 0.2f, 0.2f };
	float4	directional_light_color		= { 1.0f, 1.0f, 1.0f, 1.0f };
	float3	directional_light_direction	= { 0.0f, 0.0f, 1.0f };
	float	radiance						= 1.0f;
	float3	directional_light_focus		= { 0.0f, 0.0f, 0.0f };
	
	float	cblight_fpad;
}

cbuffer SKY_COLOR_CONSTANT_BUFFER : register(b11)
{
	float3 sky_color_base;
	float3 sky_color_dest;
	
	float2 cbsky_color_fpad;
};

cbuffer COLOR_FILTER_CONSTANT_BUFFER : register(b12)
{
	float hue_shift;
	float saturation;
	float brightness;
	float filter_alpha;
}

cbuffer SHADOWMAP_CONSTANT_BUFFER : register(b13)
{
	row_major float4x4	light_view_projection;
	float3				shadow_color		= {	0.3f,0.3f,0.3f };
	float				shadow_bias		= 0.008f;
}

// 関数
// hlsl上でランダムな数を取得できる関数。
float3 GetNoiseFactor(uint index)
{
	uint mip_level = 1; // in
	uint width, height, number_of_levels; // out
	noise_texture.GetDimensions(mip_level, width, height, number_of_levels);
	
	int3 texture_location;
	
	texture_location.x = index % width;
	texture_location.y = (index / width) % height;
	texture_location.z = 0;
	
	return noise_texture.Load(texture_location).rgb;
}

// カラーフィルターを適用した後のカラーを返す関数。
float4 ApplyColorFilter(float4 base_color)
{
	float4 new_color = base_color;
	new_color.rgb = RGB2HSV(new_color.rgb);
	
	new_color.r += hue_shift;
	new_color.g *= saturation;
	new_color.b *= brightness;
	
	new_color.rgb = HSV2RGB(new_color.rgb);
	new_color.a *= filter_alpha;
	return new_color;
}

#endif		// _COMMON_HLSL_