// インクルード
#include "3d_header.hlsli"

// 変数
RWStructuredBuffer<InstanceModel> instance_models : register(u2);

// 関数
VS_OUT_3D ShaderMain(VS_IN_3D vin)
{
	float sigma = vin.tangent.w;
	
	if (skin > -1)
	{
		row_major float4x4 skin_matrix =
		vin.weights.x * joint_matrices[vin.joints.x] +
		vin.weights.y * joint_matrices[vin.joints.y] +
		vin.weights.z * joint_matrices[vin.joints.z] +
		vin.weights.w * joint_matrices[vin.joints.w];
		
		vin.position = float4(vin.position.xyz, 1);
		vin.normal = normalize(float4(vin.normal.xyz, 0));
		vin.tangent = normalize(float4(vin.tangent.xyz, 0));
	}
	
	VS_OUT_3D vout;
	
	vout.position = mul(float4(vin.position, 1.0f), mul(world, mul(view, projection)));
	vout.w_position = mul(vin.position, world);
	
	vout.w_normal = normalize(mul(float4(vin.normal, 0.0f), world));
	
	vin.tangent.w = 0;
	vout.w_tangent = normalize(mul(vin.tangent, world));
	vout.w_tangent.w = sigma;
	
	vout.texcoord = vin.texcoord;
	
	// シャドウマップ用のtexcoordを計算
	float4 wvpPos = mul(vin.position, mul(world, mul(view, projection)));
	wvpPos /= wvpPos.w;
	wvpPos.y = -wvpPos.y;
	wvpPos.xy = 0.5f * wvpPos.xy + 0.5f;
	vout.shadow_texcoord = wvpPos.xyz;
	
	return vout;
}