// インクルード
#include "2d_header.hlsli"

// 関数
VS_OUT_2D ShaderMain(in uint vertexid : SV_VERTEXID)
{
	VS_OUT_2D vout = (VS_OUT_2D)0;
	
	const float2 position[4] =
	{
		{ -1, +1 },
		{ +1, +1 },
		{ -1, -1 },
		{ +1, -1 }
	};
	const float2 texcoords[4] =
	{
		{ 0, 0 },
		{ 1, 0 },
		{ 0, 1 },
		{ 1, 1 }
	};
	
	vout.position = float4(position[vertexid], 0, 1);
	vout.texcoord = texcoords[vertexid];
	return vout;
}