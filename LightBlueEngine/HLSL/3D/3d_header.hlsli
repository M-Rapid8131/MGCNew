#ifndef __3D_HEADER_HLSLI__
#define __3D_HEADER_HLSLI__

// インクルード
#include "../common.hlsli"

// 定数
static const uint PRIMITIVE_MAX_JOINTS = 512;

// 変数
Texture2D model_mask_texture : register(t30);

// 構造体
struct InstanceModel
{
	uint		model_index;
	float3		position;
	float4		color;
};

//    -> VS
struct VS_IN_3D
{
	float4	position	: POSITION;
	float4	normal		: NORMAL;
	float4	tangent		: TANGENT;
	float2	texcoord	: TEXCOORD;
	uint4	joints		: JOINTS;
	float4	weights		: WEIGHTS;
};

// VS -> PS
struct VS_OUT_3D
{
	float4 position			: SV_POSITION;
	float4 w_position		: POSITION;
	float4 w_normal			: NORMAL;
	float4 w_tangent		: TANGENT;
	float2 texcoord			: TEXCOORD;
	float3 shadow_texcoord	: TEXCOORD1;
};

// 定数バッファ
cbuffer PRIMITIVE_CONSTANT_BUFFER : register(b0)
{
	row_major float4x4	world;
	int					has_tangent;
	int					material;
	int					skin;
	
	int cbprimitive_ipad;
};

cbuffer PRIMITIVE_JOINT_CONSTANT_BUFFER : register(b1)
{
	row_major float4x4 joint_matrices[PRIMITIVE_MAX_JOINTS];
};

cbuffer OBJECT_COLOR_CONSTANT_BUFFER : register(b2)
{
	float3	object_color;
	float	blink_factor;
	float	object_disolve;
	float3	cbobject_color_fpad;
}

#endif // __3D_HEADER_HLSLI__