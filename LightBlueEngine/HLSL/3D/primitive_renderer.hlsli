#ifndef __PRIMITIVE_RENDERER_HLSLI__
#define __PRIMITIVE_RENDERER_HLSLI__

// �C���N���[�h
#include "../common.hlsli"

// �\����

//    -> VS
struct VS_IN_PR
{
	float4 position	: POSITION;
	float4 color		: COLOR;
	float2 texcoord	: TEXCOORD;
};

// VS -> PS
struct VS_OUT_PR
{
	float4 position : SV_POSITION;
	float4 color		: COLOR;
	float2 texcoord : TEXCOORD;
};

#endif // __PRIMITIVE_RENDERER_HLSLI__