// �C���N���[�h
#include "2d_header.hlsli" 

// �֐�
VS_OUT_2D ShaderMain(float4 position : POSITION, float4 color : COLOR, float2 texcoord : TEXCOORD)
{
	VS_OUT_2D vout;
	vout.position = position;
	vout.color = color;

	vout.texcoord = texcoord;

	return vout;
}