// �C���N���[�h
#include "3d_header.hlsli"

// �֐�
float4 ShaderMain(VS_OUT_3D pin) : SV_TARGET
{
	float	alpha	= 0.1f;	
	float4	color	= float4(object_color, alpha);
	
	return	color;
}