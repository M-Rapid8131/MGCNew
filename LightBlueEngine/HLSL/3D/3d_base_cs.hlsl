// �C���N���[�h
#include "3d_header.hlsli"

// �ϐ�
RWStructuredBuffer<InstanceModel> instance_models_buffer : register(u2);

[numthreads(512, 1, 1)]
void ShaderMain( uint3 dtid : SV_DispatchThreadID )
{
}