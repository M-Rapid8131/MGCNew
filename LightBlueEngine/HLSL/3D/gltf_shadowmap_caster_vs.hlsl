// �C���N���[�h
#include "3d_header.hlsli"

// �֐�
float4 ShaderMain(float4 position : POSITION, float4 normal : NORMAL, float2 texcoord : TEXCOORD) : SV_POSITION
{
	return mul(position, mul(world, mul(view, projection)));
}