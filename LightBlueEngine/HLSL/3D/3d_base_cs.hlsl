// インクルード
#include "3d_header.hlsli"

// 変数
RWStructuredBuffer<InstanceModel> instance_models_buffer : register(u2);

[numthreads(512, 1, 1)]
void ShaderMain( uint3 dtid : SV_DispatchThreadID )
{
}