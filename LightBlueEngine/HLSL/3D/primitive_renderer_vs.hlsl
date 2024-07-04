// �C���N���[�h
#include "primitive_renderer.hlsli"

// �֐�
VS_OUT_PR ShaderMain(VS_IN_PR vin)
{
	VS_OUT_PR vout;
	vout.position	= mul(vin.position, mul(view, projection));
	vout.color		= vin.color;
	vout.texcoord	= vin.texcoord;
	return vout;
}