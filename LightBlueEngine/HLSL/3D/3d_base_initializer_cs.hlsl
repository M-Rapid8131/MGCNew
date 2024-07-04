// インクルード
#include "3d_header.hlsli"

// 変数
RWStructuredBuffer<InstanceModel> instance_models_buffer : register(u0);

[numthreads(512, 1, 1)]
void ShaderMain( uint3 dtid : SV_DispatchThreadID )
{
	uint index = dtid.x;
	InstanceModel inst_model = instance_models_buffer[index];
	
	inst_model.model_index	= index;
	inst_model.position		= float3((index % 10) * 2.0f, (index / 10) * 2.0f, 0.0f);
	inst_model.color		= float4(1, 1, 1, 1);
	
	instance_models_buffer[index] = inst_model;
}