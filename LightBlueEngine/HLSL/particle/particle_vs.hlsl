// �C���N���[�h
#include "particle_header.hlsli"

// �֐�
VS_OUT_PT ShaderMain(uint instance_id : SV_INSTANCEID)
{
	VS_OUT_PT vout;
	vout.instance_id = instance_id;
	return vout;
}